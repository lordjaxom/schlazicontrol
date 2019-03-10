#include "event.hpp"
#include "core/logging.hpp"

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

    EventScope::EventScope() = default;

    EventScope::EventScope( EventScope&& other )
        : connection_( move( other.connection_ ) )
    {
        other.connection_ = nullptr;
    }

    EventScope::EventScope( EventConnection connection )
        : connection_( move( connection ) )
    {
    }

    EventScope::~EventScope()
    {
        connection_.disconnect();
    }

    EventScope& EventScope::operator=( EventScope&& other )
    {
        EventScope( move( other ) ).swap( *this );
        return *this;
    }

    EventScope& EventScope::operator=( EventConnection connection )
    {
        EventScope( move( connection ) ).swap( *this );
        return *this;
    }

    EventScope& EventScope::operator=( std::nullptr_t )
    {
        EventScope().swap( *this );
        return *this;
    }

    void EventScope::swap( EventScope& other )
    {
        using std::swap;
        swap( connection_, other.connection_ );
    }

} // namespace sc