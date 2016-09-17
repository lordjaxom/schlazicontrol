#include <cmath>
#include <algorithm>
#include <vector>

#include <boost/iterator/zip_iterator.hpp>

#include "connection.hpp"
#include "event.hpp"
#include "logging.hpp"
#include "manager.hpp"
#include "properties.hpp"
#include "scoped.hpp"
#include "timer.hpp"
#include "transition_fade.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

    static Logger logger( "transition_fade" );

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
            , speed_( chrono::milliseconds( properties[ speedProperty ].as< chrono::milliseconds::rep >() ) )
    {
    }

    std::unique_ptr< TransitionInstance > FadeTransition::instantiate() const
    {
        return unique_ptr< TransitionInstance >( new TransitionInstanceImpl< FadeTransition, FadeTransitionState >( *this ) );
    }

    void FadeTransition::transform( FadeTransitionState& state, Connection& connection, ChannelBuffer& values ) const
    {
        if ( state.output.empty() ) {
            logger.debug( "fade transition first transform" );
            state.output.resize( values.size() );
            state.deltas.resize( values.size() );
        }

        Scoped scoped( [&state, &values] { values = state.output; state.deltasKnown = false; } );
        state.target = move( values );

        bool changed = false;
        if ( !state.deltasKnown ) {
            changed = calculateDeltas( state );
            logger.debug(
                    "fade through input change from ", state.output[ 0 ].get(), " to ", state.target[ 0 ].get(),
                    " - delta is ", state.deltas[ 0 ] );
        }

        if ( calculateOutput( state ) ) {
            logger.debug( "fade to ", state.output[ 0 ].get() );
            changed = true;
        }

        if ( !changed ) {
            logger.debug( "no change, resetting poll receiver" );
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
        std::transform(
                state.target.begin(), state.target.end(), state.output.begin(), state.deltas.begin(),
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
        std::transform(
                state.output.begin(), state.output.end(),
                boost::make_zip_iterator( boost::make_tuple( state.deltas.begin(), state.target.begin() ) ),
                state.output.begin(),
                [&changed, factor]( ChannelValue const& output, boost::tuple< double, ChannelValue const& > const& inputs ) {
                    double delta = inputs.get< 0 >();
                    double target = inputs.get< 1 >().get();
                    double result = output.get() + factor * delta;
                    result = ( delta > 0.0 && result > target ) || ( delta < 0.0 && result < target ) ? target : result;
                    changed = changed || result != output.get();
                    return result;
                } );
        return changed;
    }

    static TransitionRegistry< FadeTransition > registry( "fade" );

} // namespace sc
