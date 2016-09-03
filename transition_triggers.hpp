#ifndef SCHLAZICONTROL_TRANSITION_TRIGGERS_HPP
#define SCHLAZICONTROL_TRANSITION_TRIGGERS_HPP

#include <map>
#include <string>
#include <vector>

#include "transition.hpp"

namespace sc {

    class Connection;
    class Manager;
    class PropertyNode;

    namespace triggers {
        class Action;
        struct State;
    } // namespace triggers

    class TriggersTransition
        : public Transition
    {
    public:
        TriggersTransition( Manager& manager, std::string id, PropertyNode const& properties );

        virtual std::size_t channels( std::size_t channels ) const override { return 1; }
        virtual bool acceptsChannels( std::size_t channels ) const override { return channels == 1; }

        virtual std::unique_ptr< TransitionStateBase > instantiate() const override;

        bool transform( triggers::State& state, Connection& connection, ChannelBuffer& values ) const;

    private:
        Manager& manager_;
        std::vector< triggers::Action > actions_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_TRANSITION_TRIGGERS_HPP
