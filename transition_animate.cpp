#include <utility>

#include "connection.hpp"
#include "event.hpp"
#include "manager.hpp"
#include "transition_animate.hpp"
#include "scoped.hpp"
#include "types.hpp"
#include "utility_colorwheel.hpp"

using namespace std;

namespace sc {

    /**
     * class AnimateTransitionBase
     */

    struct AnimateTransitionState
    {
        bool polling {};
        double elapsed {};
        ChannelBuffer output;
        EventScope pollEventScope;
        std::shared_ptr< void > data;
    };

    AnimateTransitionBase::AnimateTransitionBase( string&& id, Manager& manager )
            : Transition( move( id ) )
            , manager_( manager )
    {
    }

    unique_ptr< TransitionInstance > AnimateTransitionBase::instantiate() const
    {
        return unique_ptr< TransitionInstance >( new TransitionInstanceImpl< AnimateTransitionBase, AnimateTransitionState >( *this ) );
    }

    void AnimateTransitionBase::transform( AnimateTransitionState& state, Connection& connection, ChannelBuffer& values ) const
    {
        if ( state.data == nullptr ) {
            state.data = instantiateData();
        }
        if ( state.output.empty() ) {
            state.output = ChannelBuffer( values.size() );
        }

        Scoped scoped( [&state, &values] { values = state.output; state.polling = false; } );

        if ( find_if( values.cbegin(), values.cend(), []( auto const& value ) { return value.on(); } ) == values.cend() ) {
            state.output.fill();
            state.pollEventScope = nullptr;
            return;
        }

        animate( state.output, state.data.get(), state.elapsed );

        if ( !state.polling ) {
            Connection* safeConnection = &connection;
            AnimateTransitionState* safeState = &state;
            state.pollEventScope = manager_.pollEvent().subscribe(
                    [this, safeConnection, safeState]( chrono::nanoseconds elapsed ) {
                        poll( *safeState, *safeConnection, elapsed );
                    } );
        }
    }

    void AnimateTransitionBase::poll( AnimateTransitionState& state, Connection& connection, chrono::nanoseconds elapsed ) const
    {
        state.polling = true;
        state.elapsed = chrono::duration< double >( elapsed ).count();
        connection.transfer();
    }

} // namespace sc