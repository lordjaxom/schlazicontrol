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
		, manager_( manager )
		, device_(
					manager_, this->id(), properties[ ws281xProperty ].as< string >(),
                    properties[ firstProperty ].as< size_t >(), properties[ countProperty ].as< size_t >() )
	{
		auto& input = manager_.get< Input >( this->id(), properties[ inputProperty ] );
        connect( input, *this, [this]( ChannelBuffer const& values ) { set( values ); } );
	}

    void Ws281xOutput::set( ChannelBuffer const& values )
    {
        device_.send( values );
    }

    static OutputRegistry< Ws281xOutput > registry( "ws281x" );

} // namespace sc
