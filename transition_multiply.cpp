#include <iterator>
#include <utility>

#include "properties.hpp"
#include "transition_multiply.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

    /**
     * class MultiplyTransitionInstance
     */

    class MultiplyTransitionInstance : public TransitionInstance
    {
    public:
        MultiplyTransitionInstance( size_t factor ) : factor_( factor ) {}

        virtual bool transform( Connection& connection, ChannelBuffer& values ) override
        {
            size_t oldSize = values.size();
            size_t newSize = oldSize * factor_;
            values.resize( newSize );

            for ( auto it = next( values.begin(), oldSize ) ; it != values.end() ; ) {
                it = copy_n( values.begin(), oldSize, it );
            }
            return true;
        }

    private:
        size_t factor_;
    };

    static PropertyKey factorProperty( "factor" );

    MultiplyTransition::MultiplyTransition( Manager& manager, std::string id, PropertyNode const& properties )
            : Transition( move( id ) )
            , factor_( properties[ factorProperty ].as< size_t >() )
    {
    }

    std::unique_ptr< TransitionInstance > MultiplyTransition::instantiate() const
    {
        return unique_ptr< TransitionInstance > { new MultiplyTransitionInstance( factor_ ) };
    }

    __attribute__(( unused )) static TransitionRegistry< MultiplyTransition > registry( "multiply" );

} // namespace sc
