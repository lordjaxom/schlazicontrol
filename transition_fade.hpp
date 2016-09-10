#ifndef SCHLAZICONTROL_TRANSITION_FADE_HPP
#define SCHLAZICONTROL_TRANSITION_FADE_HPP

#include <cstddef>
#include <chrono>
#include <string>

#include "transition.hpp"

namespace sc {

    class ChannelBuffer;
    class Connection;
    class Manager;
    class PropertyNode;
    class FadeTransitionState;

    /**
     * class FadeTransition
     */

    class FadeTransition
            : public Transition
    {
    public:
        FadeTransition( Manager& manager, std::string id, PropertyNode const& properties );

        virtual std::size_t channels( std::size_t channels ) const override { return channels; }
        virtual bool acceptsChannels( std::size_t channels ) const override { return true; }

        virtual std::unique_ptr< TransitionStateBase > instantiate() const override;

        void transform( FadeTransitionState& state, Connection& connection, ChannelBuffer& values ) const;
        void poll( FadeTransitionState& state, Connection& connection, double factor ) const;

    private:
        bool calculateDeltas( FadeTransitionState& state ) const;
        bool calculateOutput( FadeTransitionState& state ) const;

        Manager& manager_;
        std::chrono::nanoseconds inverseSpeed_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_TRANSITION_FADE_HPP
