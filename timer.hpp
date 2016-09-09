#ifndef SCHLAZICONTROL_TIMER_HPP
#define SCHLAZICONTROL_TIMER_HPP

#include <chrono>
#include <functional>

#include "event.hpp"

namespace sc {

    class Manager;

    class Timer
    {
    public:
        Timer( Manager& manager, std::chrono::nanoseconds timeout, std::function< void () > handler );
        Timer( Timer const& ) = delete;
        ~Timer();

    private:
        void poll( std::chrono::nanoseconds elapsed );

        std::chrono::nanoseconds remaining_;
        std::function< void () > handler_;
        EventScope pollEventScope_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_TIMER_HPP
