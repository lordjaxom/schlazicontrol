#include <functional>
#include <unordered_map>

#include "expression.hpp"
#include "logging.hpp"
#include "properties.hpp"
#include "transition_triggers.hpp"
#include "triggers.hpp"

using namespace std;

namespace sc {

    static Logger logger( "transition_triggers" );

    using namespace triggers;

    /**
     * class TriggersTransitionInstance
     */

    class TriggersTransitionInstance : public TransitionInstance
    {
    public:
        TriggersTransitionInstance( Manager& manager, vector< Action > const& actions )
            : manager_( manager )
            , actions_( actions )
        {
        }

        virtual bool transform( Connection& connection, ChannelBuffer& values ) override
        {
            Context context { connection, manager_, state_, values[ 0 ] };
            bool result = false;
            for ( auto const& action : actions_ ) {
                result |= action.invoke( context );
            }
            return result;
        }

    private:
        Manager& manager_;
        vector< Action > const& actions_;
        State state_;
    };

    /**
     * class TriggersTransition
     */

    static PropertyKey const eventProperty( "event" );
    static PropertyKey const outcomesProperty( "outcomes" );
    static PropertyKey const actionsProperty( "actions" );

    struct ValueParser
            : expression::ArgParser< ValueParser, Value,
                    expression::Range< intmax_t, (intmax_t) ChannelValue::minimum, (intmax_t) ChannelValue::maximum >,
                    expression::Enumeration< string, typestring_is( "off" ), typestring_is( "on" ), typestring_is( "fullOn" ) >
            >
    {
        ValueParser( intmax_t value ) : BaseType( (double) value ) {}
        ValueParser( string const& value ) : BaseType(
                value == "off" ? Value { ChannelValue::offValue(), &ChannelValue::off } :
                value == "on" ? Value { ChannelValue::fullOnValue(), &ChannelValue::on } :
                Value { ChannelValue::fullOnValue(), &ChannelValue::fullOn } ) {}
    };

    static unique_ptr< triggers::Event > parseEvent( string const& event )
    {
        using namespace expression;
        return parse< triggers::Event >(
                event,
                Factory< ChangeEvent, ValueParser >( "change" ),
                Factory< TimeoutEvent, size_t >( "timeout" ) );
    }

    static unique_ptr< Outcome > parseOutcome( string const& outcome )
    {
        using namespace expression;
        return parse< Outcome >(
                outcome,
                Factory< SetOutcome, ValueParser >( "set" ),
                Factory< StartTimerOutcome, size_t, chrono::nanoseconds >( "startTimer" ),
                Factory< StopTimerOutcome, size_t >( "stopTimer" ) );
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

    TriggersTransition::TriggersTransition( Manager& manager, string id, PropertyNode const& properties )
        : Transition( move( id ) )
        , manager_( manager )
        , actions_( parseActions( properties[ actionsProperty ] ) )
    {
    }

    unique_ptr< TransitionInstance > TriggersTransition::instantiate() const
    {
        return unique_ptr< TransitionInstance > { new TriggersTransitionInstance( manager_, actions_ ) };
    }

    __attribute__(( unused )) static TransitionRegistry< TriggersTransition > registry( "triggers" );

} // namespace sc



