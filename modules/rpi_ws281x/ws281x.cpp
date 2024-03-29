#include <csignal>
#include <cstdint>
#include <chrono>
#include <iomanip>
#include <iterator>
#include <ostream>
#include <ratio>
#include <utility>

#include <signal.h>

#include <asio.hpp>
#include <boost/lexical_cast.hpp>

#include "core/logging.hpp"
#include "core/manager.hpp"
#include "modules/rpi_ws281x/ws281x.hpp"
#include "types.hpp"
#include "utility_gamma.hpp"

#include <ws2811.h>

using namespace std;
using namespace asio;
using namespace ip;

namespace sc {

	static Logger logger( "ws281x" );

	static constexpr char const ws281xSeparator[] = "\015\012";

    /**
     * class Ws281xWrapper
     */

	class Ws281xWrapper
	{
	public:
		Ws281xWrapper( uint16_t gpioPin, size_t ledCount, int stripType = WS2811_STRIP_GRB,
                       uint32_t frequency = WS2811_TARGET_FREQ, int dmaNumber = 5 )
                : wrapped_()
                , channel_( &wrapped_.channel[ 0 ] )
                , pending_()
        {
            wrapped_.freq = frequency;
            wrapped_.dmanum = dmaNumber;

            channel_->count = ledCount;
            channel_->gpionum = gpioPin;
            channel_->strip_type = stripType;
            channel_->brightness = 255;

            logger.debug( "initializing rpi_ws281x api" );

            if ( auto state = ws2811_init( &wrapped_ )) {
                throw runtime_error(
                        str( "couldn't initialize ws2811 api on pin ", gpioPin, ": ", ws2811_get_return_t_str( state )));
            }

            fill_n( channel_->leds, channel_->count, 0 );
            update();
        }

		size_t ledCount() const { return (size_t) channel_->count; }

		ws2811_led_t const* pixels() const { return channel_->leds; }
		ws2811_led_t* pixels() { wait(); return channel_->leds; }

		void update()
        {
            wait();
            ws2811_render( &wrapped_ );
            pending_ = true;
        }

	private:
		void wait()
        {
            if ( pending_ ) {
                ws2811_wait( &wrapped_ );
                pending_ = false;
            }
        }

        ws2811_t wrapped_;
		ws2811_channel_t* channel_;
		bool pending_;
	};

    /**
     * class Ws281xServer
     */

	class Ws281xServer
	{
	public:
		Ws281xServer( uint16_t gpioPin, size_t ledCount )
                : wrapper_( gpioPin, ledCount )
                , signals_( service_, SIGINT, SIGTERM )
                , acceptor_( service_, tcp::endpoint( tcp::v4(), 9999 ) )
                , socket_( service_ )
        {
            signals_.async_wait( [this]( error_code ec, int signo ) {
                logger.info( "received signal, shutting down" );
                service_.stop();
            } );
        }

		void run()
        {
            start();
            service_.run();
        }

	private:
		void start()
        {
            logger.info(
                    "waiting for connections on ", acceptor_.local_endpoint().address(),
                    ":", acceptor_.local_endpoint().port() );

            acceptor_.async_accept( socket_, [this]( error_code ec ) {
                if ( handleError( ec ) ) {
                    return;
                }

                auto&& endpoint = socket_.remote_endpoint();
                logger.info( "connection from ", endpoint.address(), ":", endpoint.port() );
                sendInitMessage();
            } );
        }

        void restart()
        {
            socket_.close();
            incoming_.consume( incoming_.size() );
            outgoing_.consume( outgoing_.size() );
            start();
        }

        void sendInitMessage()
        {
            ostream os( &outgoing_ );
            os << wrapper_.ledCount() << ws281xSeparator << flush;
            async_write( socket_, outgoing_, [this] ( error_code ec, size_t ) {
                if ( handleError( ec ) ) {
                    return;
                }
                receive();
            } );
        }

        void receive()
        {
            async_read_until( socket_, incoming_, ws281xSeparator, [this] ( error_code ec, size_t ) {
                if ( handleError( ec ) ) {
                    return;
                }

                auto srcBegin = buffers_begin( incoming_.data() );
                auto srcEnd = buffers_end( incoming_.data() );
                auto src = srcBegin;
                auto dstBegin = wrapper_.pixels();
                auto dstEnd = next( dstBegin, wrapper_.ledCount() );
                auto dst = dstBegin;
                while ( dst != dstEnd ) {
                    if ( distance( src, srcEnd ) >= 6 ) {
                        auto next = std::next( src, 6 );

                        size_t pos;
                        unsigned long long value;
                        try {
                            value = stoull( string( src, next ), &pos, 16 );
                        }
                        catch ( runtime_error const& ) {
                            pos = 0;
                        }
                        if ( pos == 6 ) {
                            *dst++ = value;
                            src = next;
                            continue;
                        }
                    }

                    logger.error( "protocol violation: message not made up of ", wrapper_.ledCount(), " six digit hex colors" );
                    restart();
                    return;
                }

                if ( distance( src, srcEnd ) < 2 || !equal( src, next( src, 2 ), ws281xSeparator ) ) {
                    logger.error( "protocol violation: message not terminated with separator" );
                    restart();
                    return;
                }

                incoming_.consume( (size_t) distance( srcBegin, src ) + 2 );
                wrapper_.update();
                receive();
            } );
        }

        bool handleError( error_code const& ec )
        {
            if ( ec ) {
                logger.error( "socket error in ws281x server: ", ec.message() );
                restart();
                return true;
            }
            return false;
        }

		Ws281xWrapper wrapper_;
		io_service service_;
		signal_set signals_;
		tcp::acceptor acceptor_;
		tcp::socket socket_;
		asio::streambuf incoming_;
		asio::streambuf outgoing_;
	};


	static PropertyKey const gpioPinProperty( "gpioPin" );
	static PropertyKey const ledCountProperty( "ledCount" );

	/**
	 * class Ws281xDirect
	 */

	class Ws281xDirect final
			: public Ws281x
	{
	public:
		Ws281xDirect( string&& id, Manager& manager, PropertyNode const& properties )
				: Ws281x( move( id ))
				, wrapper_( properties[ gpioPinProperty ].as< uint16_t >(),
							properties[ ledCountProperty ].as< size_t >()) {}

		size_t ledCount() const override { return wrapper_.ledCount(); }

		void send( ChannelBuffer const& values ) override
		{
			assert( values.size() == wrapper_->ledCount() * 3 );

			auto dstIt = wrapper_.pixels();
			for ( auto it = values.cbegin() ; it != values.cend() ; ) {
				uint8_t r = GammaTable< ratio< 10, 25 > >::get( RangedType< std::uint8_t >( *it++ ).get() );
				uint8_t g = GammaTable< ratio< 10, 25 > >::get( RangedType< std::uint8_t >( *it++ ).get() );
				uint8_t b = GammaTable< ratio< 10, 25 > >::get( RangedType< std::uint8_t >( *it++ ).get() );
				*dstIt++ = ( r << 16 ) | ( g << 8 ) | b;
			}
			wrapper_.update();
		}

	protected:
		void doStatistics( std::ostream& os ) const override {}

	private:
		Ws281xWrapper wrapper_;
	};


	/**
	 * class Ws281xClient
	 */

	class Ws281xClient final
			: public Ws281x
	{
	public:
		Ws281xClient( string&& id, Manager& manager, PropertyNode const& properties )
				: Ws281x( move( id ))
				, manager_( manager )
				, gpioPin_( properties[ gpioPinProperty ].as< uint16_t >())
				, ledCount_( properties[ ledCountProperty ].as< size_t >())
				, socket_( manager_.service())
		{
			manager_.readyEvent().subscribe( [this] { connect(); }, true );
		}

		size_t ledCount() const override { return ledCount_; }

		function< bool () > forkedProcess() const override
		{
			auto gpioPin = gpioPin_;
			auto ledCount = ledCount_;
			return [gpioPin, ledCount] { Ws281xServer( gpioPin, ledCount ).run(); return true; };
		}

		void send( ChannelBuffer const& values ) override
		{
			assert( values.size() == ledCount_ * 3 );

			if ( !socket_.is_open() ) {
				return;
			}

			ostream os( &outgoing_ );
			for ( auto const& it : values ) {
				uint8_t value = GammaTable< ratio< 10, 25 > >::get( RangedType< std::uint8_t >( it ).get() );
				os << setw( 2 ) << setfill( '0' ) << hex << (unsigned) value;
			}
			os << ws281xSeparator << flush;

			async_write( socket_, outgoing_, [this] ( error_code ec, size_t ) { handleError( ec ); } );
		}

	protected:
		void doStatistics( std::ostream& os ) const override
		{
			os << ", connected: " << socket_.is_open();
		}

	private:
		void connect()
		{
			logger.info( "connecting to ws281xsrv at localhost:9999" );

			tcp::resolver resolver( manager_.service() );
			async_connect(
					socket_, resolver.resolve( tcp::resolver::query( "localhost", "9999" ) ),
					[this] ( error_code ec, tcp::endpoint const& ) {
						if ( handleError( ec ) ) {
							return;
						}
						receiveInitMessage();
					} );
		}

		void reconnect()
		{
			socket_.close();
			incoming_.consume( incoming_.size() );
			outgoing_.consume( outgoing_.size() );
			retryConnect();
		}

		void retryConnect()
		{
			int retryMs = 1000;

			auto timer = make_shared< steady_timer >( manager_.service() );
			timer->expires_from_now( std::chrono::milliseconds( retryMs ) );
			timer->async_wait( [this, timer] ( error_code ec ) {
				if ( ec.value() == (int) errc::operation_canceled ) {
					return;
				}
				connect();
			} );
		}

		void receiveInitMessage()
		{
			async_read_until(
					socket_, incoming_, ws281xSeparator,
					[this] ( error_code ec, size_t ) {
						if ( handleError( ec ) ) {
							return;
						}

						auto begin = buffers_begin( incoming_.data() );
						auto end = buffers_end( incoming_.data() );
						auto it = search( begin, end, ws281xSeparator, ws281xSeparator + 2 );

						size_t count;
						try {
							count = boost::lexical_cast< size_t >( string( begin, it ) );
						}
						catch ( boost::bad_lexical_cast const& ) {
							logger.error( "invalid handshake from ws281xsrv: message not numeric" );
							reconnect();
							return;
						}

						if ( count != ledCount_ ) {
							logger.error( "ws281xsrv reports ", count, " leds, but ", ledCount_, " expected" );
							reconnect();
							return;
						}

						incoming_.consume( distance( begin, next( it, 2 ) ) );

						logger.info( "connection to ws281xsrv established" );
					} );
		}

		bool handleError( error_code const& ec )
		{
			if ( ec ) {
				logger.error( "error in ws281xsrv communication: ", ec.message() );
				reconnect();
				return true;
			}
			return false;
		}

		Manager& manager_;
		std::uint16_t gpioPin_;
		std::size_t ledCount_;
		ip::tcp::socket socket_;
		asio::streambuf incoming_;
		asio::streambuf outgoing_;
	};


    /**
     * registry
     */

    static ComponentRegistry< Ws281xDirect > registryDirect( "ws281x" );
    static ComponentRegistry< Ws281xClient > registryClient( "ws281xClient" );

} // namespace sc
