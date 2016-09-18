#ifndef SCHLAZICONTROL_TRANSITION_SHIFT_HPP
#define SCHLAZICONTROL_TRANSITION_SHIFT_HPP

#include <cstddef>
#include <string>

#include "transition.hpp"

namespace sc {

    class Manager;
    class PropertyNode;

    class ShiftTransition final
            : public Transition
    {
    public:
        ShiftTransition( std::string&& id, Manager& manager, PropertyNode const& properties );

        virtual std::unique_ptr< TransitionInstance > instantiate() const override;

        bool acceptsChannels( std::size_t channels ) const { return true; }
        std::size_t emitsChannels( std::size_t channels ) const { return channels + offset_; }

        void transform( Connection& connection, ChannelBuffer& values ) const;

    private:
        std::size_t offset_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_TRANSITION_SHIFT_HPP
