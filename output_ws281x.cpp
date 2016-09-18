#include <limits>
#include <utility>

#include "input.hpp"
#include "manager.hpp"
#include "output_ws281x.hpp"
#include "properties.hpp"

using namespace std;

namespace sc {

    static PropertyKey const ws281xProperty( "ws281x" );
    static PropertyKey const inputProperty( "input" );

    Ws281xOutput::Ws281xOutput( string&& id, Manager& manager, PropertyNode const& properties )
		: Component( move( id ) )
        , Output( manager, properties[ inputProperty ] )
		, device_( manager.get< Ws281x >( *this, properties[ ws281xProperty ] ) )
	{
	}

    void Ws281xOutput::set( Input const& input, ChannelBuffer const& values )
    {
        device_.send( values_ = values );
    }

    void Ws281xOutput::doStatistics( ostream& os ) const
    {
        os << "\n\t\tvalues: " << makeStatistics( values_ );
    }

    static OutputRegistry< Ws281xOutput > registry( "ws281x" );

} // namespace sc
