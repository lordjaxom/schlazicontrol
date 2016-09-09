#include <system_error>

#include "logging.hpp"
#include "manager.hpp"
#include "timer.hpp"

using namespace std;

namespace sc {

    static Logger logger( "timer" );

    Timer::Timer( Manager& manager, chrono::nanoseconds timeout, function< void () > handler )
            : remaining_( timeout )
            , handler_( move( handler ) )
            , pollEventScope_(
                    manager.pollEvent().subscribe( [this]( chrono::nanoseconds elapsed ) { poll( elapsed ); } ) )
    {
    }

    void Timer::poll( chrono::nanoseconds elapsed )
    {
        if ( elapsed < remaining_ ) {
            remaining_ -= elapsed;
            return;
        }

        __attribute__(( unused )) EventScope scope( pollEventScope_.release() );
        handler_();
    }

} // namespace sc
