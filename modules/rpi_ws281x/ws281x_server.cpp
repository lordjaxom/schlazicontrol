#include "core/logging.hpp"
#include "modules/rpi_ws281x/ws281x_wrapper.hpp"

using namespace std;

namespace sc {

    static Logger logger( "ws281x" );


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

} // namespace sc

int main( int argc, char* argv[] )
{
    
}
