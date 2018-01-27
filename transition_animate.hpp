#ifndef SCHLAZICONTROL_TRANSITION_ANIMATE_HPP
#define SCHLAZICONTROL_TRANSITION_ANIMATE_HPP

#include <cstddef>
#include <chrono>

#include "transition.hpp"

namespace sc {

    class Manager;
    class PropertyNode;
    struct AnimateTransitionState;

    /**
     * class AnimateTransition
     */

    class AnimateTransition final
            : public Transition
    {
    public:
        AnimateTransition( std::string&& id, Manager& manager, PropertyNode const& properties );

        virtual std::unique_ptr< TransitionInstance > instantiate() const override;

        bool acceptsChannels( std::size_t channels ) const { return channels % 3 == 0; }
        std::size_t emitsChannels( std::size_t channels ) const { return channels; }

        void transform( AnimateTransitionState& state, Connection& connection, ChannelBuffer& values ) const;
        void poll( AnimateTransitionState& state, Connection& connection, std::chrono::nanoseconds elapsed ) const;

    private:
        void animate( AnimateTransitionState& state ) const;

        Manager& manager_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_TRANSITION_ANIMATE_HPP