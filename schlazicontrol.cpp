#if 0
#include <chrono>
#include <thread>
#include <vector>

#include <libcpp/color_wheel.hpp>

#include <wiringPi.h>

#include "leds.hpp"
#include "log.hpp"
#include "motion_light.hpp"
#include "rgb_chain.hpp"
#include "stopwatch.hpp"
#include "vdcd.hpp"
#include "ws281xcli.hpp"

#include "sinetable.hpp" // TODO

namespace sc {

    class white_animation
    {
    public:
        white_animation( stopwatch const& sc, uint8_t max = 255, stopwatch::duration const& fade_time = std::chrono::seconds( 1 ) )
            : stopwatch_( sc )
            , max_( max )
            , fade_time_( fade_time )
            , position_()
        {
        }

        template< typename Pixels >
        void animate( double value, Pixels&& pixels )
        {
            if ( value > 0.0 ) {
                stopwatch_.countup( position_, fade_time_ );
            }
            else {
                stopwatch_.countdown( position_ );
            }

            using namespace std::chrono;
            std::uint8_t channel =
                    duration_cast< milliseconds >( position_ ).count()
                    * max_ / duration_cast< milliseconds >( fade_time_ ).count();
            std::forward< Pixels >( pixels ).fill_greater( { channel, channel, channel } );
        }

    private:
        stopwatch const& stopwatch_;
        uint8_t max_;
        stopwatch::duration fade_time_;
        stopwatch::duration position_;
    };


    class warp_animation
    {
        using color_wheel = libcpp::color_wheel< 36 >; // TODO

        static uint8_t constexpr min_brightness = 48;
        static uint8_t constexpr max_brightness = 207;

        static constexpr auto delay = std::chrono::milliseconds( 100 );

        void cycle_up( double& value, float factor = 1.0f )
        {
            value += 1.0 / led_count_ * factor;
            while ( value >= 1.0 ) {
                value -= 1.0;
            }
        }

        void cycle_down( double& value, float factor = 1.0f )
        {
            value -= 1.0 / led_count_ * factor;
            while ( value < 0.0 ) {
                value += 1.0;
            }
        }

    public:
        warp_animation( stopwatch const& sw, std::size_t led_count )
            : stopwatch_( sw )
            , led_count_( led_count )
            , color_offset_()
            , brightness_offset_()
            , countdown_()
        {
        }

        template< typename Pixels >
        void animate( Pixels& ws281x_pixels )
        {
            double brightness_index = brightness_offset_;
            double color_index = color_offset_;
            for ( uint16_t i = 0 ; i < led_count_ ; ++i ) {
                uint8_t brightness = sinetable_get( (uint8_t) ( brightness_index * 256.0 ) ) * ( max_brightness - min_brightness ) / 256 + min_brightness;
                cycle_up( brightness_index, 4.0 );
                brightness_index += 1.0 / led_count_;
                while ( brightness_index >= 1.0 ) brightness_index -= 1.0;
                std::uint32_t value = color_wheel::get( (uint8_t) ( ( color_index * 256.0 ) * brightness / 255.0 ) );
                ws281x_pixels.range( i, 1 ).fill_greater( { ( value >> 16 ) & 0xff, ( value >>  8 ) & 0xff, ( value >>  0 ) & 0xff } );
                //apply_gamma( *data );
                cycle_up( color_index );
            }

            if ( stopwatch_.countdown( countdown_ ) ) {
                cycle_down( brightness_offset_, 0.7 );
                cycle_up( color_offset_ );
                countdown_ = delay;
            }
        }

    private:
        stopwatch const& stopwatch_;
        std::size_t led_count_;
        double color_offset_;
        double brightness_offset_;
        stopwatch::duration countdown_;
    };

} // namespace sc
#endif // 0

#include <iostream>

#include "connection.hpp"
#include "commandline.hpp"
#include "input_gpio.hpp"
#include "input_vdcd.hpp"
#include "logging.hpp"
#include "manager.hpp"
#include "output_pwm.hpp"
#include "output_ws281x.hpp"

using namespace std;
using namespace sc;

int main( int argc, char* argv[] )
{
    LoggerScope loggerScope;

    CommandLine cmdLine( argv, argc );
    if ( !cmdLine.logFile().empty()) {
        Logger::output( cmdLine.logFile().c_str());
    }

    try {
        Logger logger( "schlazicontrol" );
        try {
            logger.info( "schlazicontrol starting" );

            Manager manager( cmdLine );

            logger.info( "setup finished, running..." );
            manager.run();
            logger.info( "exiting gracefully" );
        }
        catch ( exception const& e ) {
            logger.error( "runtime error, attempting graceful exit: ", e.what() );
        }
        return EXIT_SUCCESS;
    }
    catch ( sc::Ws281xLaunchException& e ) {
        Logger logger( "ws281x_server" );
        try {
            logger.info( "ws281x server starting" );
            e.run();
            logger.info( "exiting gracefully" );
        }
        catch ( exception const& e ) {
            logger.error( "runtime error, attempting graceful exit: ", e.what() );
        }
    }
}
