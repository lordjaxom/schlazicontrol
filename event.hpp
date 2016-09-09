#ifndef SCHLAZICONTROL_EVENT_HPP
#define SCHLAZICONTROL_EVENT_HPP

#include <functional>
#include <list>
#include <memory>
#include <utility>

namespace sc {

    /**
     * class EventConnection
     */

    class EventConnection
    {
        template< typename Signature > friend class Event;

    public:
        void disconnect() const;

    private:
        explicit EventConnection( std::function< void () >&& disconnect );

        std::function< void () > disconnect_;
    };

    /**
     * class Event
     */

    template< typename Signature >
    class Event
    {
    public:
        using Handler = std::function< Signature >;

        template< typename ...Args >
        void operator()( Args&&... args ) const
        {
            for ( auto const& handler : handlers_ ) {
                handler( std::forward< Args >( args )... );
            }
        }

        EventConnection subscribe( Handler handler )
        {
            handlers_.emplace_back( std::move( handler ) );
            auto it = --handlers_.end();
            return EventConnection( [this, it] { handlers_.erase( it ); } );
        }

    private:
        std::list< Handler > handlers_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_EVENT_HPP
