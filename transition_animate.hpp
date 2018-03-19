#ifndef SCHLAZICONTROL_TRANSITION_ANIMATE_HPP
#define SCHLAZICONTROL_TRANSITION_ANIMATE_HPP

#include <cstddef>
#include <chrono>

#include "forward.hpp"
#include "transition.hpp"

namespace sc {

    struct AnimateTransitionState;

    /**
     * class AnimateTransitionBase
     */

    class AnimateTransitionBase
            : public Transition
    {
    public:
        std::unique_ptr< TransitionInstance > instantiate() const override;

        bool acceptsChannels( std::size_t channels ) const { return channels % 3 == 0; }
        std::size_t emitsChannels( std::size_t channels ) const { return channels; }

        void transform( AnimateTransitionState& state, Connection& connection, ChannelBuffer& values ) const;
        void poll( AnimateTransitionState& state, Connection& connection, std::chrono::nanoseconds elapsed ) const;

    protected:
        AnimateTransitionBase( std::string&& id, Manager& manager );

        virtual std::shared_ptr< void > instantiateData() const = 0;
        virtual void animate( ChannelBuffer& output, void* dataPtr, double elapsed ) const = 0;

    private:
        Manager& manager_;
    };

    /**
     * class AnimateTransition
     */

    template< typename Data >
    class AnimateTransition
            : public AnimateTransitionBase
    {
    protected:
        using AnimateTransitionBase::AnimateTransitionBase;

        std::shared_ptr< void > instantiateData() const override
        {
            return std::shared_ptr< void >( new Data, []( void const* p ) { delete static_cast< Data const* >( p ); } );
        }

        void animate( ChannelBuffer& output, void* dataPtr, double elapsed ) const override
        {
            animate( output, *static_cast< Data* >( dataPtr ), elapsed );
        }

        virtual void animate( ChannelBuffer& output, Data& data, double elapsed ) const = 0;
    };

} // namespace sc

#endif // SCHLAZICONTROL_TRANSITION_ANIMATE_HPP