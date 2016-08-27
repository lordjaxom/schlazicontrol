#include <utility>

#include "output_ws281x.hpp"
#include "properties.hpp"

using namespace std;
using namespace asio;
using namespace ip;

namespace sc {

	static PropertyKey const ws281xProperty( "ws281x" );

	Ws281xOutput::Ws281xOutput( Manager& manager, string id, PropertyNode const& properties )
		: Output( move( id ) )
		, manager_( manager )
		, device_( manager_, properties[ ws281xProperty ].as< string >() )
	{
	}

    void Ws281xOutput::set( ChannelBuffer const& values )
    {
        device_.send( values );
    }

    __attribute__(( unused )) static OutputRegistry< Ws281xOutput > registry( "ws281x" );

} // namespace sc
