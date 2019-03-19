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
#include "modules/rpi_ws281x/ws281x_wrapper.hpp"
#include "types.hpp"
#include "utility_gamma.hpp"

#include <ws2811.h>

using namespace std;
using namespace asio;
using namespace ip;

namespace sc {

	static Logger logger( "ws281x" );

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
