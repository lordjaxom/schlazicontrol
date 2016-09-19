#include <utility>

#include "input_console.hpp"
#include "logging.hpp"
#include "manager.hpp"
#include "properties.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

    static Logger logger( "input_debug" );

    /**
     * class ConsoleInput
     */

    static PropertyKey const consoleProperty( "console" );
    static PropertyKey const tagProperty( "tag" );

    ConsoleInput::ConsoleInput( string&& id, Manager& manager, PropertyNode const& properties )
            : Component( move( id ) )
            , device_( manager.get< Console >( *this, properties[ consoleProperty ] ),
                       properties[ tagProperty ].as< std::string >() )
    {
    }

    void ConsoleInput::doStatistics( ostream& os ) const
    {
        os << ", value: " << value_;
    }

    void ConsoleInput::change( bool value )
    {
        value_ = value;
        logger.debug( "console input \"", device_.tag(), "\" changed to ", value_ );
        inputChangeEvent_( ChannelBuffer { value_ } );
    }

    static InputRegistry< ConsoleInput > registry( "debug" );

} // namespace sc


