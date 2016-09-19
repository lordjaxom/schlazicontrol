#include <cmath>
#include <algorithm>
#include <vector>

#include "connection.hpp"
#include "event.hpp"
#include "manager.hpp"
#include "properties.hpp"
#include "scoped.hpp"
#include "timer.hpp"
#include "transition_fade.hpp"
#include "types.hpp"
#include "utility.hpp"

using namespace std;

namespace sc {

    /**
     * struct FadeTransitionState
     */

    struct FadeTransitionState
    {
        bool deltasKnown;
        ChannelBuffer output;
        ChannelBuffer target;
        vector< double > deltas;
        double factor;
        EventScope pollEventScope;
    };

    /**
     * class FadeTransition
     */

    static PropertyKey const speedProperty( "speed" );

    FadeTransition::FadeTransition( string&& id, Manager& manager, PropertyNode const& properties )
            : Transition( move( id ) )
            , manager_( manager )
            , speed_( properties[ speedProperty ].as< chrono::nanoseconds >() )
    {
    }

    std::unique_ptr< TransitionInstance > FadeTransition::instantiate() const
    {
        return unique_ptr< TransitionInstance >( new TransitionInstanceImpl< FadeTransition, FadeTransitionState >( *this ) );
    }

    void FadeTransition::transform( FadeTransitionState& state, Connection& connection, ChannelBuffer& values ) const
    {
        if ( state.output.empty() ) {
            state.output = ChannelBuffer( values.size() );
            state.deltas.resize( values.size() );
        }

        Scoped scoped( [&state, &values] { values = state.output; state.deltasKnown = false; } );
        state.target = move( values );

        bool changed = false;
        if ( !state.deltasKnown ) {
            changed = calculateDeltas( state );
        }

        if ( calculateOutput( state ) ) {
            changed = true;
        }

        if ( !changed ) {
            state.pollEventScope = nullptr;
            return;
        }

        if ( !state.deltasKnown ) {
            Connection* safeConnection = &connection;
            FadeTransitionState* safeState = &state;
            state.pollEventScope = manager_.pollEvent().subscribe(
                    [this, safeConnection, safeState]( chrono::nanoseconds elapsed ) {
                        poll( *safeState, *safeConnection, (double) elapsed.count() / speed_.count());
                    } );
        }
    }

    void FadeTransition::poll( FadeTransitionState& state, Connection& connection, double factor ) const
    {
        state.deltasKnown = true;
        state.factor = factor;
        connection.transfer();
    }

    bool FadeTransition::calculateDeltas( FadeTransitionState& state ) const
    {
        auto changed = false;
        std::transform( state.target.begin(), state.target.end(), state.output.begin(), state.deltas.begin(),
                        [&changed]( ChannelValue const& target, ChannelValue const& output ) {
                            auto delta = target.get() - output.get();
                            changed = changed || abs( delta ) > 0.0;
                            return delta;
                        } );
        return changed;
    }

    bool FadeTransition::calculateOutput( FadeTransitionState& state ) const
    {
        auto changed = false;
        auto factor = state.factor;
        sc::transform( [&changed, factor]( auto const& output, auto const& delta, auto const& target ) {
                           double result = output.get() + factor * delta;
                           result = ( delta > 0.0 && result > target.get() ) || ( delta < 0.0 && result < target.get() )
                                    ? target.get() : result;
                           changed = changed || result != output.get();
                           return result;
                       }, state.output.begin(), state.output.end(), state.output.begin(), state.deltas.begin(),
                       state.target.begin() );
        return changed;
    }

    static TransitionRegistry< FadeTransition > registry( "fade" );

} // namespace sc
