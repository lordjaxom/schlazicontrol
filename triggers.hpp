#ifndef SCHLAZICONTROL_TRIGGERS_HPP
#define SCHLAZICONTROL_TRIGGERS_HPP

#include <cstddef>
#include <chrono>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "timer.hpp"
#include "types.hpp"
#include "connection.hpp"

namespace sc {

    class Manager;

    namespace triggers {

        class Context;

        /*
         * class Value
         */

        class Value
        {
        public:
            Value( double value );
            Value( ChannelValue const& value, std::function< bool ( ChannelValue const& ) > condition );

            ChannelValue const& get() const { return value_; }
            bool matches( ChannelValue const& value ) const;

        private:
            ChannelValue value_;
            std::function< bool ( ChannelValue const& ) > condition_;
        };

        /*
         * class Event
         */

        class Event
        {
        public:
            virtual ~Event();

            virtual bool applies( Context const& context ) const = 0;
        };

        /*
         * class ChangeEvent
         */

        class ChangeEvent : public Event
        {
        public:
            explicit ChangeEvent( Value value );

            virtual bool applies( Context const& context ) const override;

        private:
            Value value_;
        };

        /*
         * class TimeoutEvent
         */

        class TimeoutEvent : public Event
        {
        public:
            explicit TimeoutEvent( std::size_t timer );

            virtual bool applies( Context const& context ) const override;

        private:
            std::size_t timer_;
        };

        /*
         * class Outcome
         */

        class Outcome
        {
        public:
            virtual ~Outcome();
            virtual void invoke( Context& context ) const = 0;
        };

        /*
         * class SetOutcome
         */

        class SetOutcome : public Outcome
        {
        public:
            explicit SetOutcome( Value value );

            virtual void invoke( Context& context ) const override;

        private:
            Value value_;
        };

        /*
         * class StartTimerOutcome
         */

        class StartTimerOutcome : public Outcome
        {
        public:
            StartTimerOutcome( std::size_t timer, std::chrono::nanoseconds const& timeout );

            virtual void invoke( Context& context ) const override;

        private:
            std::size_t timer_;
            std::chrono::nanoseconds timeout_;
        };

        /*
         * class StopTimerOutcome
         */

        class StopTimerOutcome : public Outcome
        {
        public:
            explicit StopTimerOutcome( std::size_t timer );

            virtual void invoke( Context& context ) const override;

        private:
            std::size_t timer_;
        };

        /**
         * class Action
         */

        class Action
        {
        public:
            Action( std::unique_ptr< Event >&& event, std::vector< std::unique_ptr< Outcome > >&& outcomes );

            void invoke( Context& context ) const;

        private:
            std::unique_ptr< Event > event_;
            std::vector< std::unique_ptr< Outcome > > outcomes_;
        };

        /**
         * struct State
         */

        struct State
        {
            ChannelValue lastInput;
            ChannelValue output;
            std::unordered_map< std::size_t, std::unique_ptr< Timer > > timers;
            std::unordered_set< std::size_t > expiredTimers;
        };

        /**
         * class Context
         */

        class Context
        {
        public:
            Context( Connection& connection, Manager& manager, State& state, ChannelValue& value );
            ~Context();

            ChannelValue const& input() const { return value_; }
            ChannelValue const& lastInput() const { return state_.lastInput; }

            void set( ChannelValue const& value );

            void startTimer( std::size_t timer, std::chrono::nanoseconds const& timeout );
            void stopTimer( std::size_t timer );
            bool timerExpired( std::size_t timer ) const;

        private:
            Connection& connection_;
            Manager& manager_;
            State& state_;
            ChannelValue& value_;
        };

    } // namespace triggers
} // namespace sc

#endif // SCHLAZICONTROL_TRIGGERS_HPP
