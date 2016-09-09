#include "event.hpp"
#include "logging.hpp"

using namespace std;

namespace sc {

    static Logger logger( "event" );

    /**
     * class EventConnection
     */

    EventConnection::EventConnection( std::function< void() >&& disconnect )
            : disconnect_( move( disconnect ) )
    {
    }

    void EventConnection::disconnect() const
    {
        if ( disconnect_ ) {
            logger.debug( "disconnecting event" );
            disconnect_();
        }
    }

} // namespace sc