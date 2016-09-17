#ifndef SCHLAZICONTROL_EVENT_HPP
#define SCHLAZICONTROL_EVENT_HPP

#include <algorithm>
#include <functional>
#include <list>
#include <memory>
#include <tuple>
#include <utility>

#include <boost/iterator/iterator_adaptor.hpp>

#include "scoped.hpp"

namespace sc {

    /**
     * class EventConnection
     */

    class EventConnection
    {
        template< typename Signature > friend class EventInterface;

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
        EventScope();
        EventScope( EventScope const& ) = delete;
        EventScope( EventScope&& other );
        EventScope( EventConnection connection );
        ~EventScope();

        EventScope& operator=( EventScope&& other );
        EventScope& operator=( EventConnection connection );
        EventScope& operator=( std::nullptr_t );

        void swap( EventScope& other );

    private:
        EventConnection connection_;
    };

    /**
     * class EventInterface
     */

    namespace detail {
        template< typename Signature > struct EventHandlerHolder;
    } // namespace detail

    template< typename Signature >
    class EventInterface;

    template< typename ...Args >
    class EventInterface< void ( Args... ) >
    {
        using Signature = void ( Args... );
        using ExtendedSignature = void ( EventConnection const&, Args... );

        using Holder = detail::EventHandlerHolder< Signature >;

    public:
        using Handler = std::function< Signature >;
        using ExtendedHandler = std::function< ExtendedSignature >;

        EventConnection subscribe( ExtendedHandler handler )
        {
            auto guard = lock();
            auto it = inserted_.emplace( handlers_.end() );
            EventConnection connection( [this, it] { erase( it ); } );
            *it = [handler, connection]( Args... args ) { handler( connection, args... ); };
            return connection;
        }

        EventConnection subscribe( Handler handler, bool oneShot = false )
        {
            auto guard = lock();
            auto it = inserted_.emplace( handlers_.end() );
            EventConnection connection( [this, it] { erase( it ); } );
            *it = oneShot
                  ? [handler, connection]( Args... args ) {
                        EventScope scope( std::move( connection ) );
                        handler( args... );
                    }
                  : std::move( handler );
            return connection;
        }

    protected:
        template< typename ...T >
        void operator()( T&&... args )
        {
            auto guard = lock();
            std::for_each(
                    handlers_.begin(), handlers_.end(),
                    [&args...]( Holder const& holder ) {
                        if ( !holder.erased ) {
                            holder.handler( std::forward< T >( args )... );
                        }
                    } );
        };

    private:
        Scoped lock()
        {
            return Scoped(
                    [this] { ++locked_; },
                    [this] {
                        if ( --locked_ == 0 ) {
                            handlers_.remove_if( []( Holder& holder ) { return holder.erased; } );
                            handlers_.splice( handlers_.end(), inserted_ );
                        }
                    } );
        }

        void erase( typename std::list< Holder >::iterator it )
        {
            auto guard = lock();
            it->erased = true;
        }

        std::list< Holder > handlers_;
        std::list< Holder > inserted_;
        std::size_t locked_ {};
    };

    namespace detail {

        template< typename Signature >
        struct EventHandlerHolder
        {
            using Handler = typename EventInterface< Signature >::Handler;

            EventHandlerHolder& operator=( Handler&& handler ) { this->handler = std::move( handler ); return *this; }

            Handler handler;
            bool erased {};
        };

    } // namespace detail

    /**
     * class Event
     */

    template< typename Signature >
    class Event
            : EventInterface< Signature >
    {
    public:
        using Interface = EventInterface< Signature >;
        using typename Interface::Handler;

        Interface& interface() { return static_cast< Interface& >( *this ); }

        using Interface::operator();
    };

} // namespace sc

#endif // SCHLAZICONTROL_EVENT_HPP
