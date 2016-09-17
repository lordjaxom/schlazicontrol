#include <iterator>
#include <utility>

#include "properties.hpp"
#include "transition_multiply.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

    /**
     * class MultiplyTransition
     */

    static PropertyKey factorProperty( "factor" );

    MultiplyTransition::MultiplyTransition( string&& id, Manager& manager, PropertyNode const& properties )
            : Transition( move( id ) )
            , factor_( properties[ factorProperty ].as< size_t >() )
    {
    }

    unique_ptr< TransitionInstance > MultiplyTransition::instantiate() const
    {
        return unique_ptr< TransitionInstance >( new TransitionInstanceImpl< MultiplyTransition >( *this ) );
    }

    void MultiplyTransition::transform( Connection& connection, ChannelBuffer& values ) const
    {
        size_t oldSize = values.size();
        size_t newSize = oldSize * factor_;
        values.resize( newSize );

        for ( auto it = next( values.begin(), oldSize ) ; it != values.end() ; ) {
            it = copy_n( values.begin(), oldSize, it );
        }
    }

    static TransitionRegistry< MultiplyTransition > registry( "multiply" );

} // namespace sc
