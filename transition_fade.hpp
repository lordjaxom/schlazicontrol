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

    class FadeTransition final
            : public Transition
    {
    public:
        FadeTransition( std::string&& id, Manager& manager, PropertyNode const& properties );

        virtual std::unique_ptr< TransitionInstance > instantiate() const override;

        bool acceptsChannels( std::size_t channels ) const { return true; }
        std::size_t emitsChannels( std::size_t channels ) const { return channels; }

        void transform( FadeTransitionState& state, Connection& connection, ChannelBuffer& values ) const;
        void poll( FadeTransitionState& state, Connection& connection, double factor ) const;

    private:
        bool calculateDeltas( FadeTransitionState& state ) const;
        bool calculateOutput( FadeTransitionState& state ) const;

        Manager& manager_;
        double deltaPerNs_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_TRANSITION_FADE_HPP
