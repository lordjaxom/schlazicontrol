#include <algorithm>
#include <stdexcept>

#include "core/logging.hpp"
#include "modules/rpi_ws281x/ws281x_wrapper.hpp"
#include "utility_string.hpp"

using namespace std;

namespace sc {

    static Logger logger( "ws281x" );

    /**
     * class Ws281xWrapper
     */

    Ws281xWrapper::Ws281xWrapper( uint16_t gpioPin, size_t ledCount, int stripType, uint32_t frequency, int dmaNumber )
            : wrapped_()
            , channel_( &wrapped_.channel[ 0 ] )
            , pending_()
    {
        wrapped_.freq = frequency;
        wrapped_.dmanum = dmaNumber;

        channel_->count = ledCount;
        channel_->gpionum = gpioPin;
        channel_->strip_type = stripType;
        channel_->brightness = 255;

        logger.debug( "initializing rpi_ws281x api" );

        if ( auto state = ws2811_init( &wrapped_ )) {
            throw runtime_error(
                    str( "couldn't initialize ws2811 api on pin ", gpioPin, ": ", ws2811_get_return_t_str( state )));
        }

        fill_n( channel_->leds, channel_->count, 0 );
        update();
    }

    void Ws281xWrapper::update()
    {
        wait();
        ws2811_render( &wrapped_ );
        pending_ = true;
    }

    void Ws281xWrapper::wait()
    {
        if ( pending_ ) {
            ws2811_wait( &wrapped_ );
            pending_ = false;
        }
    }

} // namespace sc


