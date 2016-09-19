#include <algorithm>
#include <iterator>
#include <utility>

#include <asio.hpp>

#include "input_debug.hpp"
#include "logging.hpp"
#include "manager.hpp"
#include "properties.hpp"
#include "types.hpp"

using namespace std;
using namespace asio;

namespace sc {

    static Logger logger( "input_debug" );

    /**
     * struct DebugInputInternals
     */

    struct DebugInputInternals
    {
        DebugInputInternals( Manager& manager )
                : input( manager.service(), ::dup( STDIN_FILENO ) )
        {
        }

        posix::stream_descriptor input;
        asio::streambuf incoming;
    };

    /**
     * class DebugInput
     */

    static PropertyKey const tagProperty( "tag" );

    DebugInput::DebugInput( string&& id, Manager& manager, PropertyNode const& properties )
            : Component( move( id ) )
            , internals_( new DebugInputInternals( manager ) )
            , tag_( properties[ tagProperty ].as< string >() )
            , value_()
    {
        manager.readyEvent().subscribe( [this] { read(); }, true );
    }

    void DebugInput::read()
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
                if ( equal( first, last, tag_.cbegin(), tag_.cend() ) ) {
                    toggle();
                }

                first = last + 1;
            }
            internals_->incoming.consume( distance( begin, first ) );

            read();
        } );
    }

    bool DebugInput::handleError( error_code const& ec )
    {
        if ( ec ) {
            logger.error( "read error from stdin: ", ec.message() );
            return true;
        }
        return false;
    }

    void DebugInput::doStatistics( ostream& os ) const
    {
        os << ", value: " << value_;
    }

    void DebugInput::toggle()
    {
        value_ = !value_;
        logger.debug( "detected toggle: ", value_ );
        inputChangeEvent_( ChannelBuffer { value_ } );
    }

    static InputRegistry< DebugInput > registry( "debug" );

} // namespace sc



