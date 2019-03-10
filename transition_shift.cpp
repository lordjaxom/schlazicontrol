#include <utility>

#include "core/properties.hpp"
#include "transition_shift.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

    static PropertyKey const offsetProperty( "offset" );

    ShiftTransition::ShiftTransition( std::string&& id, Manager& manager, PropertyNode const& properties )
            : Transition( move( id ) )
            , offset_( properties[ offsetProperty ].as< size_t >() )
    {
    }

    unique_ptr< TransitionInstance > ShiftTransition::instantiate() const
    {
        return unique_ptr< TransitionInstance >( new TransitionInstanceImpl< ShiftTransition >( *this ) );
    }

    void ShiftTransition::transform( Connection& connection, ChannelBuffer& values ) const
    {
        values.shift( offset_ );
    }

    static TransitionRegistry< ShiftTransition > registry( "shift" );

} // namespace sc
