#include <limits>
#include <utility>

#include "output_ws281x.hpp"
#include "properties.hpp"

using namespace std;

namespace sc {

    static PropertyKey const ws281xProperty( "ws281x" );
    static PropertyKey const firstProperty( "first", 0 );
    static PropertyKey const countProperty( "count", Ws281xDevice::maxCount );

    Ws281xOutput::Ws281xOutput( Manager& manager, string id, PropertyNode const& properties )
		: Output( move( id ) )
		, manager_( manager )
		, device_(
					manager_, this->id(), properties[ ws281xProperty ].as< string >(),
                    properties[ firstProperty ].as< size_t >(), properties[ countProperty ].as< size_t >() )
	{
	}

    void Ws281xOutput::set( ChannelBuffer const& values )
    {
        device_.send( values );
    }

    __attribute__(( unused )) static OutputRegistry< Ws281xOutput > registry( "ws281x" );

} // namespace sc
