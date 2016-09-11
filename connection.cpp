#include <functional>
#include <stdexcept>
#include <utility>

#include "connection.hpp"
#include "input.hpp"
#include "manager.hpp"
#include "output.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

    static void checkReveiverAcceptsChannels(
            Component const& sender, Component const& receiver, size_t channels, bool accepts )
    {
        if ( !accepts ) {
            throw runtime_error( str(
                    "invalid connection between ", sender.category(), " ", sender.id(), " and ", receiver.category(),
                    " ", receiver.id(), ": receiver won't accept ", channels, " output channels" ) );
        }
    }

	static PropertyKey const inputProperty( "input" );
	static PropertyKey const transitionsProperty( "transitions" );
	static PropertyKey const outputProperty( "output" );

	Connection::Connection( Manager& manager, string id, PropertyNode const& properties )
		: Component( "connection", move( id ) )
        , manager_( manager )
		, output_( manager_.get< Output >( this->id(), properties[ outputProperty ].as< string >() ) )
	{
        auto& input = manager_.get< Input >( this->id(), properties[ inputProperty ].as< string >() );

        size_t channels = input.channels();
        Component const* sender = &input;
		for ( auto const& transitionNode : properties[ transitionsProperty ] ) {
			auto& transition = manager_.get< Transition >( this->id(), transitionNode );
            checkReveiverAcceptsChannels( *sender, transition, channels, transition.acceptsChannels( channels ) );
            channels = transition.channels( channels );
            sender = &transition;
			transitions_.push_back( transition.instantiate() );
		}
        checkReveiverAcceptsChannels( *sender, output_, channels, output_.acceptsChannels( channels ) );

        input.inputChangeEvent().subscribe( [this]( ChannelValue const& value ) { transfer( value ); } );
	}

    void Connection::transfer( ChannelValue const& value )
	{
		ChannelBuffer values { lastValue_ = value };
		for ( auto& transition : transitions_ ) {
			transition->transform( *this, values );
		}
        output_.set( values );
	}

	void Connection::retransfer()
	{
		transfer( lastValue_ );
	}

	__attribute__(( unused )) static ComponentRegistry< Connection > registry( "connection" );

} // namespace sc
