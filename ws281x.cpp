#include <csignal>
#include <chrono>
#include <iomanip>
#include <iterator>
#include <ostream>
#include <system_error>
#include <thread>
#include <utility>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include <asio.hpp>
#include <boost/lexical_cast.hpp>

#include "events.hpp"
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

		logger.debug( "initializing ws281x hardware" );

		if ( ws2811_init( &wrapped_ ) ) {
			logger.error( "couldn't initialize ws2811 api" );
			throw runtime_error( "couldn't initialize ws2811 api" );
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

			incoming_.consume( distance( srcBegin, src ) + 2 );
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

	static PropertyKey const gpioPinProperty( "gpioPin" );
	static PropertyKey const ledCountProperty( "ledCount" );

	Ws281x::Ws281x( Manager& manager, string id, PropertyNode const& properties )
		: Standalone( move( id ) )
		, manager_( manager )
		, gpioPin_( properties[ gpioPinProperty ].as< uint16_t >() )
		, ledCount_( properties[ ledCountProperty ].as< size_t >() )
		, launcher_( new Ws281xLauncher( manager_, gpioPin_, ledCount_ ) )
		, socket_( manager_.service() )
	{
		manager_.subscribeReadyEvent( [this] { connect(); } );
	}

	void Ws281x::send( ChannelBuffer const& values )
	{
		if ( !socket_.is_open() ) {
			return;
		}

		if ( values.size() != ledCount_ * 3 ) {
			logger.warning( "wx281x output called with ", values.size(), " but has ", ledCount_ * 3, " channels" );
		}

		ostream os( &outgoing_ );
		for ( size_t i = 0 ; i < ledCount_ * 3 ; ++i ) {
			auto const& value = values[ i % values.size() ];
			os << setw( 2 ) << setfill( '0' ) << hex << (unsigned) value.scale( 0, 255 );
		}
		os << ws281xSeparator << flush;

		async_write(
				socket_, outgoing_,
				[this] ( error_code ec, size_t ) {
					handleError( ec );
				} );
	}

	void Ws281x::connect()
	{
		logger.info( "connecting to ws281xsrv at localhost:9999" );

		tcp::resolver resolver( manager_.service() );
		async_connect(
				socket_, resolver.resolve( tcp::resolver::query( "localhost", "9999" ) ),
				[this] ( error_code ec, tcp::resolver::iterator ) {
					if ( handleError( ec ) ) {
						return;
					}

					receiveInitMessage();
				} );
	}

	void Ws281x::reconnect()
	{
		socket_.close();
		incoming_.consume( incoming_.size() );
		outgoing_.consume( outgoing_.size() );
		retryConnect();
	}

	void Ws281x::retryConnect()
	{
		int retryMs = 1000;

		auto timer = make_shared< steady_timer >( manager_.service() );
		timer->expires_from_now( chrono::milliseconds( retryMs ) );
		timer->async_wait( [this, timer] ( error_code ec ) {
			if ( ec == errc::operation_canceled ) {
				return;
			}
			connect();
		} );
	}

	void Ws281x::receiveInitMessage()
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

	bool Ws281x::handleError( error_code const& ec )
	{
		if ( ec ) {
			logger.error( "error in ws281xsrv communication: ", ec.message() );
			reconnect();
			return true;
		}
		return false;
	}

	Ws281xDevice::Ws281xDevice( Manager& manager, string const& ws281xId )
		: ws281x_( manager.get< Ws281x >( ws281xId ) )
	{
	}

	__attribute__(( unused )) static ComponentRegistry< Ws281x > registry( "ws281x" );

} // namespace sc
