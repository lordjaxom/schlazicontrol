#ifndef SCHLAZICONTROL_TRANSITION_COLOR_HPP
#define SCHLAZICONTROL_TRANSITION_COLOR_HPP

#include "forward.hpp"
#include "transition.hpp"

namespace sc {

    class ColorTransition
            : public Transition
    {
    public:
        std::unique_ptr< TransitionInstance > instantiate() const override;

        bool acceptsChannels( std::size_t channels ) const { return true; }
        std::size_t emitsChannels( std::size_t channels ) const { return channels * 3; }

        void transform( Connection& connection, ChannelBuffer& values ) const;

    protected:
        explicit ColorTransition( std::string&& id );

        virtual void transform( ChannelBuffer const& values, ColorBuffer& output ) const = 0;
    };

} // namespace sc

#endif // SCHLAZICONTROL_TRANSITION_COLOR_HPP
