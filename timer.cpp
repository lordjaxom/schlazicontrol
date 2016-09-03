#include <system_error>

#include "logging.hpp"
#include "manager.hpp"
#include "timer.hpp"

using namespace std;

namespace sc {

    static Logger logger( "timer" );

    Timer::Timer( Manager& manager, chrono::nanoseconds const& timeout, function< void () > handler )
        : timer_( manager.service() )
        , handler_( move( handler ) )
    {
        timer_.expires_from_now( timeout );
        timer_.async_wait( [this]( error_code ec ) {
            if ( ec.value() == (int) errc::operation_canceled ) {
                logger.debug( "timer canceled" );
                return;
            }
            logger.debug( "timer fired, ec = ", ec, " (canceled is ", (int) errc::operation_canceled, ")" );
            handler_();
        } );
    }

    Timer::~Timer()
    {
        logger.debug( "timer destructing" );
    }

} // namespace sc
