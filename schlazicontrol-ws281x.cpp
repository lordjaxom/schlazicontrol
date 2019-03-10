#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <asio.hpp>

#include "commandline.hpp"
#include "logging.hpp"
#include "properties.hpp"

#include <ws2811.h>

using namespace std;
using namespace asio;
using namespace ip;
using namespace sc;

namespace sc {

    static Logger logger( "schlazicontrol-ws281x" );

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

            if ( ws2811_init( &wrapped_ ) ) {
                throw runtime_error( str( "couldn't initialize ws2811 api on pin ", gpioPin ) );
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

    static PropertyKey const gpioPinProperty( "gpioPin" );
    static PropertyKey const ledCountProperty( "ledCount" );

    class Ws281xServer
    {
    public:
        Ws281xServer( CommandLine const& cmdLine )
                : properties_( cmdLine.propertiesFile() )
                , wrapper_( properties_[ gpioPinProperty ].as< uint16_t >(),
                            properties_[ ledCountProperty ].as< size_t >() )
                , signals_( service_, SIGINT, SIGTERM )
                , acceptor_( service_, tcp::endpoint( tcp::v4(), 9999 ) )
                , socket_( service_ )
        {
            signals_.async_wait( [this]( error_code ec, int signo ) {
                logger.info( "received signal, shutting down" );
                service_.stop();
            } );
        }

        Ws281xServer( Ws281xServer const& ) = delete;

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

        Properties properties_;
        Ws281xWrapper wrapper_;
        io_service service_;
        signal_set signals_;
        tcp::acceptor acceptor_;
        tcp::socket socket_;
        asio::streambuf incoming_;
        asio::streambuf outgoing_;
    };

} // namespace sc

static void writePidToFile( string const& pidFile )
{
    ofstream( pidFile, ios::out | ios::trunc ) << ::getpid() << "\n";
}

static void daemonize()
{
    pid_t pid;

    pid = ::fork();
    if ( pid > 0 ) {
        exit( EXIT_SUCCESS );
    }
    if ( pid != -1 && ::setsid() != -1 ) {
        pid = fork();
        if ( pid > 0 ) {
            exit( EXIT_SUCCESS );
        }
        if ( pid != -1 && ::chdir( "/" ) != -1 ) {
            ::umask( 0 );
            for ( int fd = sysconf( _SC_OPEN_MAX ) ; fd > 0 ; --fd ) {
                ::close( fd );
            }
            return;
        }
    }

    cerr << "couldn't daemonize: " << strerror( errno ) << "\n";
    exit( EXIT_FAILURE );
}

int main( int argc, char* argv[] )
{
    LoggerScope loggerScope;

    try {
        CommandLine commandLine( argv, argc );
        if ( commandLine.daemon() ) {
            daemonize();
        }
        if ( !commandLine.pidFile().empty() ) {
            writePidToFile( commandLine.pidFile() );
        }
        if ( !commandLine.logFile().empty()) {
            Logger::output( commandLine.logFile().c_str());
        }

        logger.info( "schlazicontrol-ws281x starting" );

        Ws281xServer( commandLine ).run();

        logger.info( "schlazicontrol-ws281x exiting" );
    }
    catch ( exception const& e ) {
        logger.error( e.what());
    }
}