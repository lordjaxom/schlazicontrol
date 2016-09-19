#include <algorithm>
#include <functional>
#include <stdexcept>
#include <utility>

#include <boost/iterator/transform_iterator.hpp>

#include "connection.hpp"
#include "input.hpp"
#include "manager.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

    static unique_ptr< TransitionInstance > createInstance(
            Connection const& connection, Manager& manager, PropertyNode const& properties )
    {
        auto& transition = manager.get< Transition >( connection, properties );
        return transition.instantiate();
    }

    static vector< unique_ptr< TransitionInstance > > createInstances(
            Connection const& connection, Manager& manager, PropertyNode const& properties )
    {
        auto transformer = [&]( PropertyNode const& node ) { return createInstance( connection, manager, node ); };
        auto first = boost::make_transform_iterator( properties.begin(), transformer );
        auto last = boost::make_transform_iterator( properties.end(), transformer );
        return vector< unique_ptr< TransitionInstance > >( first, last );
    }

	static PropertyKey const inputProperty( "input" );
	static PropertyKey const transitionsProperty( "transitions" );

	Connection::Connection( string&& id, Manager& manager, PropertyNode const& properties )
            : Component( move( id ) )
            , Output( manager, properties[ inputProperty ] )
            , instances_( createInstances( *this, manager, properties[ transitionsProperty ] ) )
	{
        // TODO
        Transition const* sender = nullptr;
        channels_ = inputs().front()->emitsChannels();
        for ( auto const& instance : instances_ ) {
            if ( sender ) {
                checkConnection( *sender, instance->transition(), channels_, instance->acceptsChannels( channels_ ) );
            }
            sender = &instance->transition();
            channels_ = instance->emitsChannels( channels_ );
        }
	}

    bool Connection::acceptsChannels( size_t channels ) const
    {
        return !instances_.empty() ? instances_.front()->acceptsChannels( channels ) : true;
    }

	void Connection::transfer()
	{
        output_ = input_;
        for_each(
                instances_.cbegin(), instances_.cend(),
                [this]( unique_ptr< TransitionInstance > const& instance ) {
                    instance->transform( *this, output_ );
                } );
        inputChangeEvent_( output_ );
	}

    void Connection::set( Input const& input, ChannelBuffer const& values )
    {
        input_ = values;
        transfer();
    }

    void Connection::doStatistics( ostream& os ) const
    {
        os << "\n\t\tinput: " << makeStatistics( input_ )
           << "\n\t\toutput: " << makeStatistics( output_ );
    }

    static ComponentRegistry< Connection > registry( "connection" );

} // namespace sc
