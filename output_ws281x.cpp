#include <limits>
#include <utility>

#include "input.hpp"
#include "manager.hpp"
#include "output_ws281x.hpp"
#include "properties.hpp"

using namespace std;

namespace sc {

    static PropertyKey const ws281xProperty( "ws281x" );
    static PropertyKey const firstProperty( "first", 0 );
    static PropertyKey const countProperty( "count", Ws281xDevice::maxCount );
    static PropertyKey const inputProperty( "input" );

    Ws281xOutput::Ws281xOutput( string&& id, Manager& manager, PropertyNode const& properties )
		: Component( move( id ) )
        , Output( manager, properties[ inputProperty ] )
		, manager_( manager )
		, device_(
					*this, manager_, properties[ ws281xProperty ].as< string >(),
                    properties[ firstProperty ].as< size_t >(), properties[ countProperty ].as< size_t >() )
	{
	}

    void Ws281xOutput::set( Input const& input, ChannelBuffer const& values )
    {
        device_.send( values );
    }

    static OutputRegistry< Ws281xOutput > registry( "ws281x" );

} // namespace sc
