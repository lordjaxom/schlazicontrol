#include <cstdint>

#include "core/properties.hpp"
#include "transition_color.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

    static PropertyKey const colorProperty( "color" );

    class FillColorTransition final
            : public ColorTransition
    {
    public:
        FillColorTransition( string&& id, Manager& manager, PropertyNode const& properties )
                : ColorTransition( move( id ) )
                , color_( properties[ colorProperty ].as< Rgb >() )
        {
        }

    protected:
        void transform( ChannelBuffer const& values, ColorBuffer& output ) const override
        {
            std::transform( values.cbegin(), values.cend(), output.begin(), [this]( auto const& value ) {
                return Rgb( color_ ).scale( RangedUnit< double >( value ).get() );
            } );
        }

    private:
        Rgb color_;
    };

    static TransitionRegistry< FillColorTransition > registry( "color:fill" );

} // namespace sc