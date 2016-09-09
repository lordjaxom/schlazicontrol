#include "event.hpp"
#include "logging.hpp"

using namespace std;

namespace sc {

    static Logger logger( "event" );

    /**
     * class EventConnection
     */

    EventConnection::EventConnection()
    {
    }

    EventConnection::EventConnection( function< void () > disconnect )
            : disconnect_( move( disconnect ) )
    {
    }

    EventConnection& EventConnection::operator=( std::nullptr_t )
    {
        disconnect_ = nullptr;
        return *this;
    }

    void EventConnection::disconnect() const
    {
        if ( disconnect_ ) {
            disconnect_();
            disconnect_ = nullptr;
        }
    }

    /**
     * class EventScope
     */

    EventScope::EventScope( EventConnection connection )
        : connection_( move( connection ) )
    {
    }

    EventScope::~EventScope()
    {
        connection_.disconnect();
    }

    EventConnection EventScope::release()
    {
        EventConnection released( move( connection_ ) );
        connection_ = nullptr;
        return released;
    }

} // namespace sc