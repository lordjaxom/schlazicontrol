#ifndef SCHLAZICONTROL_WS281X_WRAPPER_HPP
#define SCHLAZICONTROL_WS281X_WRAPPER_HPP

#include <cstddef>
#include <cstdint>

#include <ws2811.h>

namespace sc {

    static constexpr char const ws281xSeparator[] = "\015\012";

    /**
     * class Ws281xWrapper
     */

    class Ws281xWrapper
    {
    public:
        Ws281xWrapper( std::uint16_t gpioPin, std::size_t ledCount, int stripType = WS2811_STRIP_GRB,
                       std::uint32_t frequency = WS2811_TARGET_FREQ, int dmaNumber = 5 );

        size_t ledCount() const { return (size_t) channel_->count; }

        ws2811_led_t const* pixels() const { return channel_->leds; }
        ws2811_led_t* pixels() { wait(); return channel_->leds; }

        void update();

    private:
        void wait();

        ws2811_t wrapped_;
        ws2811_channel_t* channel_;
        bool pending_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_WS281X_WRAPPER_HPP
