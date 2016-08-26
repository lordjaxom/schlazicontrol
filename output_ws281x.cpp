#include <functional>

#include "events.hpp"
#include "manager.hpp"
#include "output_ws281x.hpp"
#include "types.hpp"

using namespace std;
using namespace asio;
using namespace ip;

namespace sc {

	static PropertyKey const ws281xProperty( "ws281x" );

	Ws281xOutput::Ws281xOutput( Manager& manager, string const& id, PropertyNode const& properties )
		: Output( id )
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
