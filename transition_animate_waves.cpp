#include "properties.hpp"
#include "transition_animate.hpp"
#include "types.hpp"
#include "utility_colorwheel.hpp"

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

    struct WaveAnimationData
    {
        double brightnessOffset {};
        double colorOffset {};
    };

    static PropertyKey colorRangeProperty( "colorRange", 1.0 );
    static PropertyKey colorSpeedProperty( "colorSpeed" );
    static PropertyKey pulseRangeProperty( "pulseRange", 1.0 );
    static PropertyKey pulseSpeedProperty( "pulseSpeed" );
    static PropertyKey minBrightProperty( "minBright" );
    static PropertyKey maxBrightProperty( "maxBright" );

    class TransitionAnimateWaves
            : public AnimateTransition< WaveAnimationData >
    {
    public:
        TransitionAnimateWaves( std::string&& id, Manager& manager, PropertyNode const& properties )
                : AnimateTransition< WaveAnimationData >( move( id ), manager )
                , colorRange_( properties[ colorRangeProperty ].as< double >() )
                , colorSpeed_( properties[ colorSpeedProperty ].as< double >() )
                , pulseRange_( properties[ pulseRangeProperty ].as< double >() )
                , pulseSpeed_( properties[ pulseSpeedProperty ].as< double >() )
                , minBright_( properties[ minBrightProperty ].as< double >() )
                , maxBright_( properties[ maxBrightProperty ].as< double >() )
        {
        }

    protected:
        void animate( ChannelBuffer& output, WaveAnimationData& data, double elapsed ) const override
        {
            ColorBuffer colorBuffer( output );

            auto brightnessIndex = data.brightnessOffset;
            auto colorIndex = data.colorOffset;

            for ( auto pixel : colorBuffer ) {
                auto brightness = sin( brightnessIndex * 6.283 ) * ( maxBright_ - minBright_ ) + minBright_;
                pixel = Colorwheel< 256 >::get( colorIndex * 255.0 ).scale( brightness );
                cyclicIncrement( brightnessIndex, pulseRange_ / colorBuffer.size() );
                cyclicIncrement( colorIndex, colorRange_ / colorBuffer.size() );
            }

            cyclicDecrement( data.brightnessOffset, pulseSpeed_ * elapsed );
            cyclicIncrement( data.colorOffset, colorSpeed_ * elapsed );
        }

    private:
        double colorRange_;
        double colorSpeed_;
        double pulseRange_;
        double pulseSpeed_;
        double minBright_;
        double maxBright_;
    };

    static TransitionRegistry< TransitionAnimateWaves > registry( "animate:waves" );

} // namespace sc



