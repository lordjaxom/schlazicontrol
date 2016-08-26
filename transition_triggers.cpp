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

        virtual bool transform( Connection const& connection, ChannelBuffer& values ) override
        {
            Context context { manager_, state_, values[ 0 ] };
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

    namespace expression {

        template<>
        struct ArgumentConverter< Value >
        {
            static Value parse( string const& function, size_t index, std::intmax_t value )
            {
                if ( value < ChannelValue::minimum || value > ChannelValue::maximum ) {
                    invalidArgument(
                            function, index, "value must be between ", (int) ChannelValue::minimum, " and ",
                            (int) ChannelValue::maximum );
                }
                return { (double) value };
            }

            static Value parse( string const& function, size_t index, string const& value )
            {
                if ( value == "off" ) {
                    return { ChannelValue::offValue(), &ChannelValue::off };
                }
                if ( value == "on" ) {
                    return { ChannelValue::fullOnValue(), &ChannelValue::on };
                }
                if ( value == "fullOn" ) {
                    return { ChannelValue::fullOnValue(), &ChannelValue::fullOn };
                }
                throw invalidArgument(
                        function, index, "value must be numeric or one of \"off\", \"on\" or \"fullOn\"" );
            }
        };

    } // namespace expression

    /**
     * class TriggersTransition
     */

    static PropertyKey const eventProperty( "event" );
    static PropertyKey const outcomesProperty( "outcomes" );
    static PropertyKey const actionsProperty( "actions" );

    static unique_ptr< triggers::Event > parseEvent( string const& event )
    {
        using namespace expression;
        return parseCall< triggers::Event >(
                event,
                Factory< ChangeEvent, Value >( "change" ),
                Factory< TimeoutEvent, size_t >( "timeout" ) );
    }

    static unique_ptr< Outcome > parseOutcome( string const& outcome )
    {
        using namespace expression;
        return parseCall< Outcome >(
                outcome,
                Factory< SetOutcome, Value >( "set" ),
                Factory< StartTimerOutcome, size_t, chrono::microseconds >( "startTimer" ),
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

    TriggersTransition::TriggersTransition( Manager& manager, string const& id, PropertyNode const& properties )
        : Transition( id )
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



