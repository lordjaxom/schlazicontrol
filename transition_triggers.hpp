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
        TriggersTransition( std::string&& id, Manager& manager, PropertyNode const& properties );

        virtual std::unique_ptr< TransitionInstance > instantiate() const override;

        bool acceptsChannels( std::size_t channels ) const { return channels == 1; }
        std::size_t emitsChannels( std::size_t channels ) const { return 1; }

        void transform( triggers::State& state, Connection& connection, ChannelBuffer& values ) const;

    private:
        Manager& manager_;
        std::vector< triggers::Action > actions_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_TRANSITION_TRIGGERS_HPP
