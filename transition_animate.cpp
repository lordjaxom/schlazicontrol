#include <utility>

#include "connection.hpp"
#include "event.hpp"
#include "manager.hpp"
#include "properties.hpp"
#include "transition_animate.hpp"
#include "scoped.hpp"
#include "types.hpp"
#include "utility_colorwheel.hpp"

using namespace std;

namespace sc {

    static void cyclicIncrement( double& value, double increment )
    {
        value += increment;
        while ( value > 1.0 ) {
            value -= 1.0;
        }
    }

    static void cyclicDecrement( double& value, double decrement )
    {
        value -= decrement;
        while ( value < 0.0 ) {
            value += 1.0;
        }
    }

    /**
     * struct AnimateTransitionState
     */

    struct AnimateTransitionState
    {
        bool polling;
        double elapsedSeconds;
        double brightnessOffset;
        double colorOffset;
        ChannelBuffer output;
        EventScope pollEventScope;
    };

    /**
     * class AnimateTransition
     */

    AnimateTransition::AnimateTransition( string&& id, Manager& manager, PropertyNode const& properties )
            : Transition( move( id ) )
            , manager_( manager )
    {
    }

    unique_ptr< TransitionInstance > AnimateTransition::instantiate() const
    {
        return unique_ptr< TransitionInstance >( new TransitionInstanceImpl< AnimateTransition, AnimateTransitionState >( *this ) );
    }

    void AnimateTransition::transform( AnimateTransitionState& state, Connection& connection, ChannelBuffer& values ) const
    {
        if ( state.output.empty() ) {
            state.output = ChannelBuffer( values.size() );
        }

        Scoped scoped( [&state, &values] { values = state.output; state.polling = false; } );

        if ( find_if( values.cbegin(), values.cend(), []( auto const& value ) { return value.on(); } ) == values.cend() ) {
            state.output.fill();
            state.pollEventScope = nullptr;
            return;
        }

        animate( state );

        if ( !state.polling ) {
            Connection* safeConnection = &connection;
            AnimateTransitionState* safeState = &state;
            state.pollEventScope = manager_.pollEvent().subscribe(
                    [this, safeConnection, safeState]( chrono::nanoseconds elapsed ) {
                        poll( *safeState, *safeConnection, elapsed );
                    } );
        }
    }

    void AnimateTransition::poll( AnimateTransitionState& state, Connection& connection, chrono::nanoseconds elapsed ) const
    {
        state.polling = true;
        state.elapsedSeconds = chrono::duration< double >( elapsed ).count();
        connection.transfer();
    }

    void AnimateTransition::animate( AnimateTransitionState& state ) const
    {
        static constexpr double radius = 1.0;
        static constexpr double speed = 1.0;

        ColorBuffer colorBuffer( state.output );

        //auto brightnessIndex = state.brightnessOffset;
        auto colorIndex = state.colorOffset;

        for ( auto pixel : colorBuffer ) {
            pixel = Colorwheel< 256 >::get( colorIndex * 255.0 );
            cyclicIncrement( colorIndex, radius / colorBuffer.size() );
        }

        cyclicIncrement( state.colorOffset, speed * state.elapsedSeconds );

/*
        auto end = state.output.end();
        for ( auto it = state.output.begin() ; it != end ; it += 3 ) {
            std::uint8_t brightness = std::sin( brightnessIndex * 6.283f ) * 255.0f;
            brightnessIndex += 0.004f; while ( brightnessIndex > 1.0f ) brightnessIndex -= 1.0f;
            std::uint32_t color = sc::Colorwheel<256>::get( colorIndex * 255.0f );
            *(it + 1) = ChannelValue( ( ( color >> 16 ) & 0xff ) * 100.0f / 255.0f );
            *(it + 2) = ChannelValue( ( ( color >> 16 ) & 0xff ) * 100.0f / 255.0f );
            *(it + 3) = ChannelValue( ( ( color >> 16 ) & 0xff ) * 100.0f / 255.0f );
            colorIndex += 0.004f; while ( colorIndex > 1.0f ) colorIndex -= 1.0f;
        }

        state.brightnessOffset -= 0.004f; while ( state.brightnessOffset < 0.0f ) state.brightnessOffset += 1.0f;
        state.colorOffset += 0.004f; while ( state.brightnessOffset > 1.0f ) state.brightnessOffset -= 1.0f;
        */
    }

    static TransitionRegistry< AnimateTransition > registry( "animate" );

} // namespace sc