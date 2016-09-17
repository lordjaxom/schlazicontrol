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

    class MultiplyTransition final
            : public Transition
    {
    public:
        MultiplyTransition( std::string&& id, Manager& manager, PropertyNode const& properties );

        virtual std::unique_ptr< TransitionInstance > instantiate() const override;

        bool acceptsChannels( std::size_t channels ) const { return true; }
        std::size_t emitsChannels( std::size_t channels ) const { return channels * factor_; }

        void transform( Connection& connection, ChannelBuffer& values ) const;

    private:
        std::size_t factor_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_TRANSITION_MULTIPLY_HPP
