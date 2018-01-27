#ifndef SCHLAZICONTROL_FORWARD_HPP
#define SCHLAZICONTROL_FORWARD_HPP

namespace sc {

    class ChannelValue;
    class ColorBuffer;
    class Connection;
    class Input;
    class Manager;
    class PropertyNode;
    class Transition;
    class VdcdDevice;

    namespace detail {

        class SmartChannelBuffer;
        class SimpleChannelBuffer;

    } // namespace detail

    using ChannelBuffer =
#if defined( USE_SMART_CHANNEL_BUFFER )
            detail::SmartChannelBuffer
#else
            detail::SimpleChannelBuffer
#endif
            ;

} // namespace sc

#endif //SCHLAZICONTROL_FORWARD_HPP
