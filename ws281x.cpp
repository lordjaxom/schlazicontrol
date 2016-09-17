#include <csignal>
#include <chrono>
#include <iomanip>
#include <iterator>
#include <ostream>
#include <thread>
#include <utility>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include <asio.hpp>
#include <boost/lexical_cast.hpp>

#include "logging.hpp"
#include "manager.hpp"
#include "types.hpp"
#include "ws281x.hpp"

#include <ws2811.h>

using namespace std;
using namespace asio;
using namespace ip;

namespace sc {

	static Logger logger( "ws281x" );

	static constexpr char const ws281xSeparator[] = "\015\012";

    static void killGracefully( pid_t pid, size_t timeoutMs = 2000 )
    {
        logger.debug( "terminating process ", pid );

        kill( pid, SIGTERM );

        while ( timeoutMs > 0 ) {
            int status;
            if ( waitpid( pid, &status, WNOHANG ) == pid ) {
                return;
            }

            this_thread::sleep_for( chrono::milliseconds( 100 ) );
            timeoutMs = timeoutMs > 100 ? timeoutMs - 100 : 0;
        }

        logger.debug( "termination failed, killing process ", pid );

        kill( pid, SIGKILL );
    }

	class Ws2811Wrapper
	{
	public:
		Ws2811Wrapper(
				uint16_t gpioPin, size_t ledCount, int stripType = WS2811_STRIP_GRB,
				uint32_t frequency = WS2811_TARGET_FREQ, int dmaNumber = 5 );

		size_t ledCount() const { return channel_->count; }

		ws2811_led_t const* pixels() const { return channel_->leds; }
		ws2811_led_t* pixels() { wait(); return channel_->leds; }

		void update();

	private:
		void wait();

		ws2811_t wrapped_;
		ws2811_channel_t* channel_;
		bool pending_;
	};

	Ws2811Wrapper::Ws2811Wrapper(
			uint16_t gpioPin, size_t ledCount, int stripType, uint32_t frequency, int dmaNumber )
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

		logger.debug( "initializing ws281x api on pin ", gpioPin );

		if ( ws2811_init( &wrapped_ ) ) {
			throw runtime_error( str( "couldn't initialize ws2811 api on pin ", gpioPin ) );
		}

		fill_n( channel_->leds, channel_->count, 0 );
		update();
	}

	void Ws2811Wrapper::update()
	{
		wait();
		ws2811_render( &wrapped_ );
		pending_ = true;
	}

	void Ws2811Wrapper::wait()
	{
		if ( pending_ ) {
			ws2811_wait( &wrapped_ );
			pending_ = false;
		}
	}

	class Ws281xServer
	{
	public:
		Ws281xServer( uint16_t gpioPin, size_t ledCount );
		~Ws281xServer();

		void run();

	private:
		void start();
		void restart();

		void sendInitMessage();

		void receive();

		bool handleError( error_code const& ec );

		Ws2811Wrapper wrapper_;
        bool signaled_;
		io_service service_;
		signal_set signals_;
		tcp::acceptor acceptor_;
		tcp::socket socket_;
		asio::streambuf incoming_;
		asio::streambuf outgoing_;
	};

	Ws281xServer::Ws281xServer( uint16_t gpioPin, size_t ledCount )
		: wrapper_( gpioPin, ledCount )
        , signaled_()
		, signals_( service_, SIGINT, SIGTERM )
		, acceptor_( service_, tcp::endpoint( tcp::v4(), 9999 ) )
		, socket_( service_ )
	{
		signals_.async_wait( [this]( error_code ec, int signo ) {
			logger.info( "received signal, shutting down" );
            signaled_ = true;
			service_.stop();
		} );
	}

	Ws281xServer::~Ws281xServer()
	{
        if ( !signaled_ ) {
            killGracefully( getppid() );
        }
	}

	void Ws281xServer::run()
	{
		start();
		service_.run();
	}

	void Ws281xServer::start()
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

	void Ws281xServer::restart()
	{
		socket_.close();
		incoming_.consume( incoming_.size() );
		outgoing_.consume( outgoing_.size() );
		start();
	}

	void Ws281xServer::sendInitMessage()
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

	void Ws281xServer::receive()
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

	bool Ws281xServer::handleError( error_code const& ec )
	{
		if ( ec ) {
			logger.error( "socket error in ws281x server: ", ec.message() );
			restart();
			return true;
		}
		return false;
	}

	class Ws281xLauncher
	{
	public:
		Ws281xLauncher( Manager& manager, uint16_t gpioPin, size_t ledCount );
		~Ws281xLauncher();

	private:
		void start();

		Manager& manager_;
		uint16_t gpioPin_;
		size_t ledCount_;
		pid_t pid_;
	};

	Ws281xLauncher::Ws281xLauncher( Manager& manager, uint16_t gpioPin, size_t ledCount )
		: manager_( manager )
		, gpioPin_( gpioPin )
		, ledCount_( ledCount )
		, pid_()
	{
		start();
	}

	Ws281xLauncher::~Ws281xLauncher()
	{
        killGracefully( pid_ );
	}

	void Ws281xLauncher::start()
	{
		auto& service = manager_.service();
		service.notify_fork( io_service::fork_prepare );
		auto pid = fork();
		if ( pid == -1 ) {
			throw system_error( errno, std::system_category(), "couldn't start ws281x server process" );
		}
		if ( pid == 0 ) {
			service.notify_fork( io_service::fork_child );
			throw Ws281xLaunchException( gpioPin_, ledCount_ );
		}
		service.notify_fork( io_service::fork_parent );
		pid_ = pid;
	}

	Ws281xLaunchException::Ws281xLaunchException( uint16_t gpioPin, size_t ledCount )
		: gpioPin_( gpioPin )
		, ledCount_( ledCount )
	{
	}

	void Ws281xLaunchException::run()
	{
		Ws281xServer( gpioPin_, ledCount_ ).run();
	}


    struct Ws281xInternals
    {
        Ws281xInternals( io_service& service )
                : socket( service )
        {
        }

        ip::tcp::socket socket;
        asio::streambuf incoming;
        asio::streambuf outgoing;
    };

    static PropertyKey const gpioPinProperty( "gpioPin" );
	static PropertyKey const ledCountProperty( "ledCount" );

	Ws281x::Ws281x( string&& id, Manager& manager, PropertyNode const& properties )
            : Component( move( id ) )
            , manager_( manager )
            , gpioPin_( properties[ gpioPinProperty ].as< uint16_t >() )
            , ledCount_( properties[ ledCountProperty ].as< size_t >() )
            , launcher_( new Ws281xLauncher( manager_, gpioPin_, ledCount_ ) )
            , internals_( new Ws281xInternals( manager_.service() ) )
            , values_( ledCount_ * 3 )
	{
		manager_.readyEvent().subscribe( [this] { connect(); }, true );
	}

    Ws281x::~Ws281x() = default;

	void Ws281x::send( size_t start, ChannelBuffer const& values )
	{
        copy( values.begin(), values.end(), next( values_.begin(), start * 3 ) );

		if ( !internals_->socket.is_open() ) {
			return;
		}

		ostream os( &internals_->outgoing );
        for ( auto it = values_.begin() ; it != values_.end() ; ++it ) {
			os << setw( 2 ) << setfill( '0' ) << hex << (unsigned) it->scale( 0, 255 );
		}
		os << ws281xSeparator << flush;

		async_write(
                internals_->socket, internals_->outgoing,
				[this] ( error_code ec, size_t ) {
					handleError( ec );
				} );
	}

	void Ws281x::connect()
	{
		logger.info( "connecting to ws281xsrv at localhost:9999" );

		tcp::resolver resolver( manager_.service() );
		async_connect(
                internals_->socket, resolver.resolve( tcp::resolver::query( "localhost", "9999" ) ),
				[this] ( error_code ec, tcp::resolver::iterator ) {
					if ( handleError( ec ) ) {
						return;
					}

					receiveInitMessage();
				} );
	}

	void Ws281x::reconnect()
	{
        internals_->socket.close();
        internals_->incoming.consume( internals_->incoming.size() );
        internals_->outgoing.consume( internals_->outgoing.size() );
		retryConnect();
	}

	void Ws281x::retryConnect()
	{
		int retryMs = 1000;

		auto timer = make_shared< steady_timer >( manager_.service() );
		timer->expires_from_now( chrono::milliseconds( retryMs ) );
		timer->async_wait( [this, timer] ( error_code ec ) {
			if ( ec.value() == (int) errc::operation_canceled ) {
				return;
			}
			connect();
		} );
	}

	void Ws281x::receiveInitMessage()
	{
		async_read_until(
                internals_->socket, internals_->incoming, ws281xSeparator,
				[this] ( error_code ec, size_t ) {
					if ( handleError( ec ) ) {
						return;
					}

					auto begin = buffers_begin( internals_->incoming.data() );
					auto end = buffers_end( internals_->incoming.data() );
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

                    internals_->incoming.consume( distance( begin, next( it, 2 ) ) );

					logger.info( "connection to ws281xsrv established" );
				} );
	}

	bool Ws281x::handleError( error_code const& ec )
	{
		if ( ec ) {
			logger.error( "error in ws281xsrv communication: ", ec.message() );
			reconnect();
			return true;
		}
		return false;
	}

	Ws281xDevice::Ws281xDevice(
            Manager& manager, string const& requester, string const& ws281xId, size_t start, size_t count )
	    	: ws281x_( manager.get< Ws281x >( requester, ws281xId ) )
	        , start_( start )
            , count_( count != maxCount ? count : ws281x_.ledCount() - start )
	{
        if ( start_ > ws281x_.ledCount() ) {
            throw runtime_error( str(
                    "component \"", requester, "\" requested partial led chain beginning at ", start_,
                    ", but ws281x \"", ws281xId, "\" has only ", ws281x_.ledCount(), " leds" ) );
        }
        if ( start_ + count_ > ws281x_.ledCount() ) {
            throw runtime_error( str(
                    "component \"", requester, "\" requested partial led chain of ", count_, " leds, but ws281x \"",
                    ws281xId, "\" has only ", ws281x_.ledCount() - start, " leds beginning at ", start_ ) );
        }
	}

    void Ws281xDevice::send( ChannelBuffer const& values )
    {
        assert( values.size() == count_ * 3 );

        ws281x_.send( start_, values );
    }

    static ComponentRegistry< Ws281x > registry( "ws281x" );

} // namespace sc
