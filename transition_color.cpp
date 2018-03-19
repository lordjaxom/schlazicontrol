#include <cstdint>

#include "transition_color.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

    ColorTransition::ColorTransition( string&& id )
            : Transition( move( id ) )
    {
    }

    unique_ptr< TransitionInstance > ColorTransition::instantiate() const
    {
        return unique_ptr< TransitionInstance >( new TransitionInstanceImpl< ColorTransition >( *this ) );
    }

    void ColorTransition::transform( Connection& connection, ChannelBuffer& values ) const
    {
        ChannelBuffer output( values.size() * 3 );
        ColorBuffer colorBuffer( output );
        transform( values, colorBuffer );
        values = move( output );
    }

} // namespace sc


