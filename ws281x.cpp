#include <csignal>
#include <chrono>
#include <iomanip>
#include <iterator>
#include <ostream>
#include <ratio>
#include <utility>

#include <signal.h>

#include <asio.hpp>
#include <boost/lexical_cast.hpp>

#include "logging.hpp"
#include "manager.hpp"
#include "types.hpp"
#include "utility_gamma.hpp"
#include "ws281x.hpp"

#include <ws2811.h>

using namespace std;
using namespace asio;
using namespace ip;

namespace sc {

	static Logger logger( "ws281x" );

	static constexpr char const ws281xSeparator[] = "\015\012";

    /**
     * class Ws281x
     */

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
            , internals_( new Ws281xInternals( manager_.service() ) )
	{
		manager_.readyEvent().subscribe( [this] { connect(); }, true );
	}

    void Ws281x::send( ChannelBuffer const& values )
	{
		assert( values.size() == ledCount_ * 3 );

		if ( !internals_->socket.is_open() ) {
			return;
		}

		ostream os( &internals_->outgoing );
        for ( auto it = values.begin() ; it != values.end() ; ++it ) {
            uint8_t value = GammaTable< ratio< 10, 25 > >::get( RangedType< std::uint8_t >( *it ).get() );
			os << setw( 2 ) << setfill( '0' ) << hex << (unsigned) value;
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

	void Ws281x::doStatistics( ostream& os ) const
	{
		os << ", connected: " << internals_->socket.is_open();
	}

    /**
     * class Ws281xDevice
     */

	Ws281xDevice::Ws281xDevice( Ws281x& ws281x )
	    	: ws281x_( ws281x )
	{
	}

    void Ws281xDevice::send( ChannelBuffer const& values )
    {
        ws281x_.send( values );
    }

    static ComponentRegistry< Ws281x > registry( "ws281x" );

} // namespace sc
