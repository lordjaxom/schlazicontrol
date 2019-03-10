#include <algorithm>
#include <iterator>
#include <utility>

#include <asio.hpp>

#include "console.hpp"
#include "core/logging.hpp"
#include "manager.hpp"

using namespace std;
using namespace asio;

namespace sc {

    static Logger logger( "console" );

    /**
     * struct ConsoleInternals
     */

    struct ConsoleInternals
    {
        ConsoleInternals( Manager& manager )
                : input( manager.service(), ::dup( STDIN_FILENO ) )
        {
        }

        posix::stream_descriptor input;
        asio::streambuf incoming;
    };

    /**
     * class Console
     */

    Console::Console( std::string&& id, Manager& manager, PropertyNode const& properties )
            : Component( move( id ) )
            , internals_( new ConsoleInternals( manager ) )
    {
        manager.readyEvent().subscribe( [this] { read(); }, true );
    }

    void Console::read()
    {
        async_read_until( internals_->input, internals_->incoming, '\n', [this]( error_code ec, size_t ) {
            if ( handleError( ec ) ) {
                return;
            }

            auto begin = buffers_begin( internals_->incoming.data() );
            auto end = buffers_end( internals_->incoming.data() );

            auto first = begin;
            decltype( first ) last;
            while ( ( last = find( first, end, '\n' ) ) != end ) {
                string tag( first, last );
                auto it = devices_.find( tag );
                if ( it != devices_.end() ) {
                    it->second->toggle();
                }
                else {
                    logger.error( "requested input toggle on unknown console device \"", tag );
                }

                first = last + 1;
            }
            internals_->incoming.consume( distance( begin, first ) );

            read();
        } );
    }

    bool Console::handleError( error_code const& ec )
    {
        if ( ec ) {
            logger.error( "read error from stdin: ", ec.message() );
            return true;
        }
        return false;
    }

    void Console::add( ConsoleDevice* device )
    {
        devices_.emplace( device->tag(), device );
    }

    void Console::remove( ConsoleDevice* device )
    {
        devices_.erase( device->tag() );
    }

    void Console::doStatistics( std::ostream& os ) const
    {
    }

    /**
     * class ConsoleDevice
     */

    ConsoleDevice::ConsoleDevice( Console& console, std::string tag )
            : console_( console )
            , tag_( move( tag ) )
    {
        console_.add( this );
    }

    ConsoleDevice::~ConsoleDevice()
    {
        console_.remove( this );
    }

    void ConsoleDevice::toggle()
    {
        changeEvent_( value_ = !value_ );
    }

    /**
     * registry
     */

    static ComponentRegistry< Console > registry( "console" );

} // namespace sc