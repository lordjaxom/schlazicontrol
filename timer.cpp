#include <system_error>

#include "manager.hpp"
#include "timer.hpp"

using namespace std;

namespace sc {

    Timer::Timer( Manager& manager, std::chrono::microseconds timeout, std::function< void () > const& handler )
        : timer_( manager.service() )
        , handler_( handler )
    {
        timer_.expires_from_now( timeout );
        timer_.async_wait( [this]( error_code ec ) {
            if ( ec == errc::operation_canceled ) {
                return;
            }
            handler_();
        } );
    }

} // namespace sc
