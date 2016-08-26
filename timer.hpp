#ifndef SCHLAZICONTROL_TIMER_HPP
#define SCHLAZICONTROL_TIMER_HPP

#include <chrono>
#include <functional>

#include <asio.hpp>

namespace sc {

    class Manager;

    class Timer
    {
    public:
        Timer( Manager& manager, std::chrono::microseconds timeout, std::function< void () > const& handler );

    private:
        asio::steady_timer timer_;
        std::function< void () > handler_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_TIMER_HPP