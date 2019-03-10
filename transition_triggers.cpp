#include <functional>
#include <unordered_map>

#include <boost/iterator/transform_iterator.hpp>

#include "expression.hpp"
#include "core/logging.hpp"
#include "properties.hpp"
#include "transition_triggers.hpp"
#include "triggers.hpp"

using namespace std;

namespace sc {

    static Logger logger( "transition_triggers" );

    using namespace triggers;

    /**
     * class TriggersTransition
     */

    static PropertyKey const eventProperty( "event" );
    static PropertyKey const outcomesProperty( "outcomes" );
    static PropertyKey const actionsProperty( "actions" );

    struct ValueLimits
    {
        static constexpr intmax_t minimum = 0;
        static constexpr intmax_t maximum = 100;
    };

    struct ValueParser
            : expression::ArgParser< ValueParser, Value,
                    expression::Range< intmax_t, 0, 100 >,
                    expression::Enumeration< string, typestring_is( "off" ), typestring_is( "on" ), typestring_is( "fullOn" ) >
            >
    {
        ValueParser( intmax_t value ) : BaseType( { rangedPercent( value ) } ) {}
        ValueParser( string const& value ) : BaseType(
                value == "off" ? Value { ChannelValue::offValue(), &ChannelValue::off } :
                value == "on" ? Value { ChannelValue::fullOnValue(), &ChannelValue::on } :
                Value { ChannelValue::fullOnValue(), &ChannelValue::fullOn } ) {}
    };

    static unique_ptr< triggers::Event > parseEvent( string const& event )
    {
        using namespace expression;
        return parseExpression< triggers::Event >(
                event,
                Factory< ChangeEvent, ValueParser >( "change" ),
                Factory< TimeoutEvent, size_t >( "timeout" ));
    }

    static unique_ptr< Outcome > parseOutcome( string const& outcome )
    {
        using namespace expression;
        return parseExpression< Outcome >(
                outcome,
                Factory< SetOutcome, ValueParser >( "set" ),
                Factory< StartTimerOutcome, size_t, chrono::nanoseconds >( "startTimer" ),
                Factory< StopTimerOutcome, size_t >( "stopTimer" ));
    }

    static vector< unique_ptr< Outcome > > parseOutcomes( PropertyList< string > const& outcomes )
    {
        auto first = boost::make_transform_iterator( outcomes.begin(), &parseOutcome );
        auto last = boost::make_transform_iterator( outcomes.end(), &parseOutcome );
        return { first, last };
    }

    static Action parseAction( PropertyNode const& action )
    {
        auto event = parseEvent( action[ eventProperty ].as< string >() );
        auto outcomes = parseOutcomes( action[ outcomesProperty ].as< string[] >() );
        return { move( event ), move( outcomes ) };
    }

    static vector< Action > parseActions( PropertyNode const& actions )
    {
        auto first = boost::make_transform_iterator( actions.begin(), &parseAction );
        auto last = boost::make_transform_iterator( actions.end(), &parseAction );
        return { first, last };
    }

    TriggersTransition::TriggersTransition( string&& id, Manager& manager, PropertyNode const& properties )
        : Transition( move( id ) )
        , manager_( manager )
        , actions_( parseActions( properties[ actionsProperty ] ) )
    {
    }

    unique_ptr< TransitionInstance > TriggersTransition::instantiate() const
    {
        return unique_ptr< TransitionInstance > { new TransitionInstanceImpl< TriggersTransition, State >( *this ) };
    }

    void TriggersTransition::transform( triggers::State& state, Connection& connection, ChannelBuffer& values ) const
    {
        Context context { connection, manager_, state, values[ 0 ] };
        for_each(
                actions_.begin(), actions_.end(),
                [&context]( triggers::Action const& action ) { action.invoke( context ); } );
    }

    static TransitionRegistry< TriggersTransition > registry( "triggers" );

} // namespace sc



