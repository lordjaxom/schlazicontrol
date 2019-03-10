#include <system_error>
#include <utility>

#include "core/manager.hpp"
#include "timer.hpp"

using namespace std;

namespace sc {

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

        EventScope scope( move( pollEventScope_ ) );
        handler_();
    }

} // namespace sc
