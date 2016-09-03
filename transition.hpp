#ifndef SCHLAZICONTROL_TRANSITION_HPP
#define SCHLAZICONTROL_TRANSITION_HPP

#include <memory>
#include <string>

#include "component.hpp"
#include "utility.hpp"

namespace sc {

    class ChannelBuffer;
    class Connection;

    namespace detail {

        struct EmptyTransitionState {};

    } // namespace detail

    /**
     * class TransitionStateBase
     */

    class TransitionStateBase
    {
    public:
        TransitionStateBase();
        TransitionStateBase( TransitionStateBase const& ) = delete;
        virtual ~TransitionStateBase();

        virtual bool transform( Connection& connection, ChannelBuffer& values ) = 0;
    };

    /**
     * class TransitionState
     */

    template< typename Transition, typename State = detail::EmptyTransitionState >
    class TransitionState
            : public TransitionStateBase
    {
    public:
        TransitionState( Transition const& transition ) : transition_( transition ) {}

        virtual bool transform( Connection& connection, ChannelBuffer& values ) override
        {
            return transform( connection, values, state_ );
        }

    private:
        bool transform( Connection& connection, ChannelBuffer& values, detail::EmptyTransitionState )
        {
            return transition_.transform( connection, values );
        }

        template< typename Other >
        bool transform( Connection& connection, ChannelBuffer& values, Other& state )
        {
            return transition_.transform( state, connection, values );
        }

        Transition const& transition_;
        State state_;
    };

    class Transition
        : public Component
    {
    public:
        explicit Transition( std::string id );

        virtual std::size_t channels( std::size_t channels ) const = 0;
        virtual bool acceptsChannels( std::size_t channels ) const = 0;

        virtual std::unique_ptr< TransitionStateBase > instantiate() const = 0;
    };

    template< typename Type >
    class TransitionRegistry
        : public ComponentRegistry< Type >
    {
    public:
        TransitionRegistry( char const* name ) : ComponentRegistry< Type >( str( "transition.", name ) ) {}
    };

} // namespace sc

#endif // SCHLAZICONTROL_TRANSITION_HPP
