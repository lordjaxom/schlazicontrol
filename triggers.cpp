#include <algorithm>
#include <utility>

#include "logging.hpp"
#include "manager.hpp"
#include "timer.hpp"
#include "triggers.hpp"

using namespace std;

namespace sc {

    static Logger logger( "triggers" );

    namespace triggers {

        /*
         * class Value
         */

        Value::Value( double value )
            : Value( value, [this]( ChannelValue const& other ) { return value_ == other; } )
        {
        }

        Value::Value( ChannelValue const& value, function< bool ( ChannelValue const& other ) > condition )
            : value_( value )
            , condition_( move( condition ) )
        {
        }

        bool Value::matches( ChannelValue const& value ) const
        {
            return condition_( value );
        }

        /*
         * class Event
         */

        Event::~Event() = default;

        /*
         * class ChangeEvent
         */

        ChangeEvent::ChangeEvent( Value value )
            : value_( move( value ) )
        {
        }

        bool ChangeEvent::applies( Context const& context ) const
        {
            bool old = value_.matches( context.lastInput() );
            bool current = value_.matches( context.input() );
            if ( !old && current ) {
                logger.debug(
                        "change event applies, changed from ", context.lastInput().get(), " to ", context.input().get() );
                return true;
            }
            return false;
        }

        /*
         * class TimeoutEvent
         */

        TimeoutEvent::TimeoutEvent( std::size_t timer )
            : timer_( timer )
        {
        }

        bool TimeoutEvent::applies( Context const& context ) const
        {
            if ( context.timerExpired( timer_ ) ) {
                logger.debug( "timer ", timer_, " expired" );
                return true;
            }
            return false;
        }

        /*
         * class Outcome
         */

        Outcome::~Outcome() = default;

        /*
         * class SetOutcome
         */

        SetOutcome::SetOutcome( Value value )
            : value_( move( value ) )
        {
        }

        bool SetOutcome::invoke( Context& context ) const
        {
            logger.debug( "set outcome triggered with value ", value_.get().get() );

            return context.set( value_.get() );
        }

        /*
         * class StartTimerOutcome
         */

        StartTimerOutcome::StartTimerOutcome( size_t timer, chrono::nanoseconds const& timeout )
            : timer_( timer )
            , timeout_( timeout )
        {
        }

        bool StartTimerOutcome::invoke( Context& context ) const
        {
            logger.debug( "starting timer ", timer_, " with ", timeout_.count(), "ns" );
            context.startTimer( timer_, timeout_ );
            return false;
        }

        /*
         * class StopTimerOutcome
         */

        StopTimerOutcome::StopTimerOutcome( size_t timer )
            : timer_( timer )
        {
        }

        bool StopTimerOutcome::invoke( Context& context ) const
        {
            logger.debug( "explicitly stopping timer ", timer_ );
            context.stopTimer( timer_ );
            return false;
        }

        /**
         * class Action
         */

        Action::Action( std::unique_ptr< Event >&& event, std::vector< std::unique_ptr< Outcome > >&& outcomes )
            : event_( move( event ) )
            , outcomes_( move( outcomes ) )
        {
        }

        bool Action::invoke( Context& context ) const
        {
            return event_->applies( context )
                    ? accumulate(
                            outcomes_.begin(), outcomes_.end(), false,
                            [&]( bool result, unique_ptr< Outcome > const& outcome ) { return result | outcome->invoke( context ); } )
                    : false;
        }

        /**
         * class Context
         */

        Context::Context( Connection& connection, Manager& manager, State& state, ChannelValue& value )
                : connection_( connection )
                , manager_( manager )
                , state_( state )
                , value_( value )
        {
            logger.debug( "context create" );
        }

        Context::~Context()
        {
            logger.debug( "context destroy" );
            state_.lastInput = move( value_ );
            value_ = state_.output;
        }

        bool Context::set( ChannelValue const& value )
        {
            if ( state_.output != value ) {
                state_.output = value;
                return true;
            }
            return false;
        }

        void Context::startTimer( size_t timer, chrono::nanoseconds const& timeout )
        {
            stopTimer( timer );

            Connection* safeConnection = &connection_;
            State* safeState = &state_;
            state_.timers.emplace(
                    timer, unique_ptr< Timer >( new Timer( manager_, timeout, [timer, safeConnection, safeState] {
                        logger.debug( "timeout handler fired" );

                        auto it = safeState->timers.find( timer );
                        __attribute__(( unused )) unique_ptr< Timer > ptr = move( it->second );
                        safeState->timers.erase( it );

                        safeState->expiredTimers.insert( timer );
                        safeConnection->transfer( safeState->lastInput );
                    } ) ) );
        }

        void Context::stopTimer( size_t timer )
        {
            state_.timers.erase( timer );
        }

        bool Context::timerExpired( std::size_t timer ) const
        {
            return state_.expiredTimers.erase( timer ) > 0;
        }

    } // namespace triggers
} // namespace sc
