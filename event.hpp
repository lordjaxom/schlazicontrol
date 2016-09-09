#ifndef SCHLAZICONTROL_EVENT_HPP
#define SCHLAZICONTROL_EVENT_HPP

#include <functional>
#include <list>
#include <utility>

namespace sc {

    /**
     * class EventConnection
     */

    class EventConnection
    {
        template< typename Signature >
        friend class EventInterface;

    public:
        EventConnection();

        EventConnection& operator=( std::nullptr_t );

        void disconnect() const;

    private:
        explicit EventConnection( std::function< void () > disconnect );

        mutable std::function< void () > disconnect_;
    };

    /**
     * class EventScope
     */

    class EventScope
    {
    public:
        EventScope( EventConnection connection );
        EventScope( EventScope const& ) = delete;
        ~EventScope();

        EventConnection release();

    private:
        EventConnection connection_;
    };

    /**
     * class EventInterface
     */

    template< typename Signature >
    class Event;

    template< typename Signature >
    class EventInterface;

    template< typename ...Args >
    class EventInterface< void ( Args... ) >
    {
        friend class Event< void ( Args... ) >;

    public:
        using Handler = std::function< void ( Args... ) >;
        using ExtendedHandler = std::function< void ( EventConnection const&, Args... ) >;

        EventConnection subscribe( ExtendedHandler handler )
        {
            auto it = handlers_.emplace( handlers_.end() );
            EventConnection connection( [this, it] { handlers_.erase( it ); } );
            *it = [handler, connection]( Args... args ) {
                handler( connection, args... );
            };
            return connection;
        }

        EventConnection subscribe( Handler handler, bool oneShot = false )
        {
            auto it = handlers_.emplace( handlers_.end() );
            EventConnection connection( [this, it] { handlers_.erase( it ); } );
            *it = oneShot
                    ? [handler, connection]( Args... args ) {
                        EventScope scope( std::move( connection ) );
                        handler( args... );
                    }
                    : std::move( handler );
            return connection;
        }

    private:
        std::list< Handler > handlers_;
    };

    /**
     * class Event
     */

    template< typename Signature >
    class Event
    {
    public:
        using Interface = EventInterface< Signature >;
        using Handler = typename Interface::Handler;

        Interface& interface() { return interface_; }

        template< typename ...T >
        void operator()( T&&... args ) const
        {
            for ( auto it = interface_.handlers_.begin() ; it != interface_.handlers_.end() ; ) {
                // make sure handlers can delete themselves
                auto current = it++;
                ( *current )( std::forward< T >( args )... );
            }
        }

    private:
        Interface interface_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_EVENT_HPP
