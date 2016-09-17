#include <algorithm>
#include <functional>
#include <stdexcept>
#include <utility>

#include <boost/iterator/transform_iterator.hpp>

#include "connection.hpp"
#include "input.hpp"
#include "manager.hpp"
#include "output.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

    static unique_ptr< TransitionInstance > createInstance( Manager& manager, string const& id, PropertyNode const& properties )
    {
        auto& transition = manager.get< Transition >( id, properties );
        return transition.instantiate();
    }

    static vector< unique_ptr< TransitionInstance > > createInstances(
            Manager& manager, string const& id, PropertyNode const& properties )
    {
        auto transformer = [&]( PropertyNode const& node ) { return createInstance( manager, id, node ); };
        auto first = boost::make_transform_iterator( properties.begin(), transformer );
        auto last = boost::make_transform_iterator( properties.end(), transformer );
        return vector< unique_ptr< TransitionInstance > >( first, last );
    }

	static PropertyKey const inputProperty( "input" );
	static PropertyKey const transitionsProperty( "transitions" );

	Connection::Connection( string&& id, Manager& manager, PropertyNode const& properties )
            : Component( move( id ) )
            , manager_( manager )
            , instances_( createInstances( manager_, this->id(), properties[ transitionsProperty ] ) )
	{
        auto& input = manager_.get< Input >( this->id(), properties[ inputProperty ] );

        Component const* sender = &input;
        channels_ = input.emitsChannels();
        for ( auto const& instance : instances_ ) {
            checkConnection( *sender, instance->transition(), *instance, channels_ );
            sender = &instance->transition();
            channels_ = instance->emitsChannels( channels_ );
        }

        connect( input, *this, [this]( ChannelBuffer const& value ) { transfer( value ); } );
	}

    bool Connection::acceptsChannels( size_t channels ) const
    {
        return !instances_.empty() ? instances_.front()->acceptsChannels( channels ) : true;
    }

    void Connection::transfer( ChannelBuffer values )
	{
        lastValues_ = values;
		for ( auto& instance : instances_ ) {
            instance->transform( *this, values );
		}
        inputChangeEvent_( values );
	}

	void Connection::retransfer()
	{
		transfer( move( lastValues_ ) );
	}

    static ComponentRegistry< Connection > registry( "connection" );

} // namespace sc
