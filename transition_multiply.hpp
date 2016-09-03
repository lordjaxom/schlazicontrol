#ifndef SCHLAZICONTROL_TRANSITION_MULTIPLY_HPP
#define SCHLAZICONTROL_TRANSITION_MULTIPLY_HPP

#include <cstddef>
#include <string>

#include "transition.hpp"

namespace sc {

    class ChannelBuffer;
    class Connection;
    class Manager;
    class PropertyNode;

    class MultiplyTransition
            : public Transition
    {
    public:
        MultiplyTransition( Manager& manager, std::string id, PropertyNode const& properties );

        virtual std::size_t channels( std::size_t channels ) const override { return channels * factor_; }
        virtual bool acceptsChannels( std::size_t channels ) const override { return true; }

        virtual std::unique_ptr< TransitionStateBase > instantiate() const override;

        bool transform( Connection& connection, ChannelBuffer& values ) const;

    private:
        std::size_t factor_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_TRANSITION_MULTIPLY_HPP
