#include <algorithm>
#include <unordered_map>
#include <utility>

#include "manager.hpp"
#include "timer.hpp"
#include "triggers.hpp"

using namespace std;

namespace sc {
    namespace triggers {

        namespace detail {

            /**
             * class Context
             */

            class Context
            {
            public:
                Context( Manager& manager, State& state, ChannelValue& value )
                        : manager_( manager )
                        , state_( state )
                        , value_( value )
                {
                }

                ~Context()
                {
                    state_.lastInput = move( value_ );
                    value_ = state_.output;
                }

                ChannelValue const& input() const { return value_; }
                ChannelValue const& lastInput() const { return state_.lastInput; }

                bool output( ChannelValue const& value )
                {
                    if ( state_.output != value ) {
                        state_.output = value;
                        return true;
                    }
                    return false;
                }

                void startTimer( size_t timer, chrono::microseconds timeout )
                {
                    stopTimer( timer );
                    state_.timers.emplace( piecewise_construct, forward_as_tuple( timer ), forward_as_tuple( manager_, timeout, [] {} ) );
                }

                void stopTimer( size_t timer )
                {
                    state_.timers.erase( timer );
                }

            private:
                Manager& manager_;
                State& state_;
                ChannelValue& value_;
            };

        } // namespace detail

        /*
         * class Value
         */

        Value::Value( double value )
            : Value( value, [this]( ChannelValue const& other ) { return value_ == other; } )
        {
        }

        Value::Value( ChannelValue const& value, function< bool ( ChannelValue const& other ) > const& condition )
            : value_( value )
            , condition_( condition )
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

        ChangeEvent::ChangeEvent( Value const& value )
            : value_( value )
        {
        }

        bool ChangeEvent::applies( Context const& context ) const
        {
            bool old = value_.matches( context.lastInput() );
            bool current = value_.matches( context.input() );
            return !old && current;
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
            return false;
        }

        /*
         * class Outcome
         */

        Outcome::~Outcome() = default;

        /*
         * class SetOutcome
         */

        SetOutcome::SetOutcome( Value const& value )
            : value_( value )
        {
        }

        bool SetOutcome::invoke( Context& context ) const
        {
            return context.set( value_.get() );
        }

        /*
         * class StartTimerOutcome
         */

        StartTimerOutcome::StartTimerOutcome( size_t timer, chrono::microseconds timeout )
            : timer_( timer )
            , timeout_( timeout )
        {
        }

        bool StartTimerOutcome::invoke( Context& context ) const
        {
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

        Context::Context( Manager& manager, State& state, ChannelValue& value )
            : manager_( manager )
            , state_( state )
            , value_( value )
        {
        }

        Context::~Context()
        {
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

        void Context::startTimer( size_t timer, chrono::microseconds timeout )
        {
            stopTimer( timer );
            state_.timers.emplace(
                    piecewise_construct, forward_as_tuple( timer ), forward_as_tuple( manager_, timeout, [] {} ) );
        }

        void Context::stopTimer( size_t timer )
        {
            state_.timers.erase( timer );
        }

    } // namespace triggers
} // namespace sc
