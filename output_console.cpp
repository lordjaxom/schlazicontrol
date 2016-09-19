#include "logging.hpp"
#include "output_console.hpp"
#include "properties.hpp"
#include "statistics.hpp"

using namespace std;

namespace sc {

    static Logger logger( "output_debug" );

    /**
     * class ConsoleOutput
     */

    static PropertyKey const inputProperty( "input" );
    static PropertyKey const channelsProperty( "channels" );

    ConsoleOutput::ConsoleOutput( string&& id, Manager& manager, PropertyNode const& properties )
            : Component( move( id ) )
            , Output( manager, properties[ inputProperty ] )
            , channels_( properties[ channelsProperty ].as< size_t >() )
    {
    }

    void ConsoleOutput::set( Input const& input, ChannelBuffer const& values )
    {
        values_ = values;
        logger.debug( "set to ", makeStatistics( values_ ) );
    }

    void ConsoleOutput::doStatistics( ostream& os ) const
    {
        os << "\n\t\tvalues: " << makeStatistics( values_ );
    }

    static OutputRegistry< ConsoleOutput > registry( "debug" );

} // namespace sc