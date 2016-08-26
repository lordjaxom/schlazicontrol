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

#include "config.hpp"
#include "connection.hpp"
#include "cmdline.hpp"
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

    CmdLine cmdLine( argv, argc );
    if ( !cmdLine.logFile().empty()) {
        Logger::output( cmdLine.logFile().c_str());
    }

    try {
        Logger logger( "schlazicontrol" );
        try {
            logger.info( "schlazicontrol starting" );

            Manager manager( cmdLine );

            /*
            Gpio::Triggers pirSenseTriggers;
            pirSenseTriggers.add( Gpio::Change( Gpio::Edge::Rising ), Gpio::Set( true ), Gpio::StopTimer( 0 ) );
            pirSenseTriggers.add( Gpio::Change( Gpio::Edge::Falling ), Gpio::StartTimer( 0, chrono::seconds( 5 ) ) );
            pirSenseTriggers.add( Gpio::Timeout( 0 ), Gpio::Set( false ) );

            Gpio::Triggers debounceTriggers;
            debounceTriggers.add( Gpio::Change( Gpio::Edge::Rising ), Gpio::StartTimer( 0, chrono::milliseconds( 100 ) ), Gpio::StopTimer( 1 ) );
            debounceTriggers.add( Gpio::Change( Gpio::Edge::Falling ), Gpio::StartTimer( 1, chrono::milliseconds( 100 ) ), Gpio::StopTimer( 0 ) );
            debounceTriggers.add( Gpio::Timeout( 0 ), Gpio::Set( true ) );
            debounceTriggers.add( Gpio::Timeout( 1 ), Gpio::Set( false ) );

            Transformers bedlightTransformer;
            bedlightTransformer.add( Transform::GreyToRgb() );
            bedlightTransformer.add( Transform::Multiply( cfg::ws281x_led_count ) );

            Transformers headlightTransformer;
            headlightTransformer.add( Transform::GreyToRgb() );

            manager.add( "input", "vdcd", "bedlight_switch", _()( "dsName", "Unterbodenbeleuchtung Bett" )( "dsuid", "fbf90820-3501-11e6-bdf4-0800200c9a66" )( "group", "1" )( "outputType", "light" )( "dimmable", "true" ) );
            */

            /*
            manager.add< GpioInput >( "floor_right", cfg::pir_sensor_right_pin, pirSenseTriggers );
            manager.add< GpioInput >( "floor_center", cfg::pir_sensor_center_pin, pirSenseTriggers );
            manager.add< GpioInput >( "floor_left", cfg::pir_sensor_left_pin, pirSenseTriggers );
            manager.add< VdcdInput >( "bedlight_switch", "Unterbodenbeleuchtung Bett", "fbf90820-3501-11e6-bdf4-0800200c9a66", 1, "light", true );
            manager.add< VdcdInput >( "headlight_switch", "Beleuchtung Kopfteil", "443bb7f0-35a1-11e6-bdf4-0800200c9a66", 1, "light", true );

            manager.add< SoftPwmOutput >( "headlight_leds", cfg::rgb_chain_red_pin, cfg::rgb_chain_green_pin, cfg::rgb_chain_blue_pin );
            manager.add< Ws281xOutput >( "bedlight_leds", cfg::ws281x_led_pin, cfg::ws281x_led_count );

            manager.add< Connection >( "1", "floor_right", "bedlight_leds" );
            manager.add< Connection >( "2", "floor_center", "bedlight_leds" );
            manager.add< Connection >( "3", "floor_left", "bedlight_leds" );
            manager.add< Connection >( "4", "bedlight_switch", "bedlight_leds", bedlightTransformer );
            manager.add< Connection >( "5", "headlight_switch", "headlight_leds", headlightTransformer );
            */

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

    /*
    log( "setting up wiringPi and GPIO" );

    wiringPiSetup();

    sc::stopwatch stopwatch;

    sc::ws281x_client ws281x_client( stopwatch, "localhost", 9999 );
    sc::rgb_chain actor_headroom( cfg::rgb_chain_red_pin, cfg::rgb_chain_green_pin, cfg::rgb_chain_blue_pin );

    sc::vdcd vdcd( stopwatch, "192.168.178.57", 8999 );
    auto device_bedlight = vdcd.emplace_device( "Unterbodenbeleuchtung Bett", "fbf90820-3501-11e6-bdf4-0800200c9a66", 1, "light", true );
    auto device_headroom = vdcd.emplace_device( "Beleuchtung Kopfteil", "443bb7f0-35a1-11e6-bdf4-0800200c9a66", 1, "light", true );

    sc::motion_light motion_right_device( stopwatch, 3 );
    sc::motion_light motion_center_device( stopwatch, 2 );
    sc::motion_light motion_left_device( stopwatch, 0 );

    sc::warp_animation bedlight_mood( stopwatch, cfg::ws281x_led_count );
    sc::white_animation bedlight_animation( stopwatch );
    sc::white_animation headroom_animation( stopwatch );
    sc::white_animation motion_right_animation( stopwatch, 127 );
    sc::white_animation motion_center_animation( stopwatch, 127 );
    sc::white_animation motion_left_animation( stopwatch, 127 );

    while ( true ) {
        stopwatch.advance();

        ws281x_client.prepare();
        actor_headroom.prepare();

        vdcd.poll();
        motion_right_device.poll();
        motion_center_device.poll();
        motion_left_device.poll();

        auto&& ws281x_pixels = ws281x_client.pixels();
        auto&& headroom_pixels = actor_headroom.pixels();

        //bed_light_mood.animate( ws281x_pixels );
        bedlight_animation.animate( device_bedlight->get(), ws281x_pixels );
        headroom_animation.animate( device_headroom->get(), headroom_pixels );
        motion_right_animation.animate( motion_right_device.get(), ws281x_pixels.range( 0, 12 ) );
        motion_center_animation.animate( motion_center_device.get(), ws281x_pixels.range( 11, 14 ) );
        motion_left_animation.animate( motion_left_device.get(), ws281x_pixels.range( 24, 12 ) );

        ws281x_client.update();
        actor_headroom.update();

        auto elapsed = stopwatch.elapsed();
        if ( elapsed < cfg::update_duration ) {
            std::this_thread::sleep_for( cfg::update_duration - elapsed );
        }
    }
    */
}
