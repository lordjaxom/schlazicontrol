#include <cstdint>

#include "core/properties.hpp"
#include "transition_color.hpp"
#include "types.hpp"

#include "core/logging.hpp"

using namespace std;

namespace sc {

    static Logger logger( "transition_color_gradient" );

    static PropertyKey const startProperty( "start" );
    static PropertyKey const endProperty( "end" );

    class GradientColorTransition final
            : public ColorTransition
    {
    public:
        GradientColorTransition( string&& id, Manager& manager, PropertyNode const& properties )
                : ColorTransition( move( id ) )
                , start_( properties[ startProperty ].as< Rgb >() )
                , end_( properties[ endProperty ].as< Rgb >() )
        {
        }

    protected:
        void transform( ChannelBuffer const& values, ColorBuffer& output ) const override
        {
            auto steps = values.size() - 1;
            auto dr = ( (double) end_.red() - start_.red() ) / steps;
            auto dg = ( (double) end_.green() - start_.green() ) / steps;
            auto db = ( (double) end_.blue() - start_.blue() ) / steps;
            for ( size_t i = 0 ; i < values.size() ; ++i ) {
                output[ i ] = Rgb( (uint8_t) ( start_.red() + dr * i ),
                                   (uint8_t) ( start_.green() + dg * i ),
                                   (uint8_t) ( start_.blue() + db * i )
                                 ).scale( RangedUnit< double >( values[ i ] ).get() );
            }
        }

    private:
        Rgb start_;
        Rgb end_;
    };

    static TransitionRegistry< GradientColorTransition > registry( "color:gradient" );

} // namespace sc