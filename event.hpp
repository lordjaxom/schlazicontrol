#ifndef SCHLAZICONTROL_EVENT_HPP
#define SCHLAZICONTROL_EVENT_HPP

#include <algorithm>
#include <functional>
#include <list>
#include <memory>
#include <tuple>
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
        template< typename Signature > struct EventHandlerIterator;
        template< typename Signature > struct EventLock;
    } // namespace detail

    template< typename Signature >
    class EventInterface;

    template< typename ...Args >
    class EventInterface< void ( Args... ) >
    {
        friend class detail::EventLock< void ( Args... ) >;

    public:
        using Handler = std::function< void ( Args... ) >;
        using ExtendedHandler = std::function< void ( EventConnection const&, Args... ) >;

        using const_iterator = detail::EventHandlerIterator< void ( Args... ) >;

        EventConnection subscribe( ExtendedHandler handler )
        {
            auto it = handlers_.emplace( handlers_.end() );
            EventConnection connection( [this, it] { erase( it ); } );
            *it = std::make_tuple( [handler, connection]( Args... args ) { handler( connection, args... ); }, false );
            return connection;
        }

        EventConnection subscribe( Handler handler, bool oneShot = false )
        {
            auto it = handlers_.emplace( handlers_.end() );
            EventConnection connection( [this, it] { erase( it ); } );
            *it = std::make_tuple( oneShot
                    ? [handler, connection]( Args... args ) {
                        EventScope scope( std::move( connection ) );
                        handler( args... );
                    }
                    : std::move( handler ),
                    false );
            return connection;
        }

    protected:
        EventInterface()
                : locked_()
        {
        }

        detail::EventLock lock()
        {
            return detail::EventLock( *this );
        }

        const_iterator begin() const { return const_iterator( *this, handlers_.begin() ); }
        const_iterator end() const { return const_iterator( *this, handlers_.end() ); }

    private:
        using Holder = std::tuple< Handler, bool >;

        void erase( std::list< Holder >::iterator it )
        {
            if ( locked_ ) {
                std::get< 1 >( *it ) = true;
            }
            else {
                handlers_.erase( it );
            }
        }

        std::list< Holder > handlers_;
        bool locked_;
    };

    namespace detail {

        template< typename Signature >
        struct EventHandlerIterator;

        template< typename Signature >
        struct EventLock
        {
            using Interface = EventInterface< Signature >;

            EventLock( Interface& interface )
                    : interface_( &interface )
            {
                interface_->locked_ = true;
            }

            EventLock( EventLock const& ) = delete;

            EventLock( EventLock&& other )
                    : interface_( other.interface_ )
            {
                other.interface_ = nullptr;
            }

            ~EventLock()
            {
                if ( interface_ ) {
                    interface_->handlers_.remove_if(
                            []( Interface::Holder const& holder ) { return std::get< 1 >( holder ); } );
                    interface_->locked_ = false;
                }
            }

        private:
            Interface* interface_;
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
        using Interface::Handler;

        Interface& interface() { return *static_cast< Interface* >( this ); }

        template< typename ...T >
        void operator()( T&&... args )
        {
            auto lock = this->lock();
            std::for_each(
                    this->begin(), this->end(), []( Handler& handler ) { handler( std::forward< T >( args )... ); } );
            for ( auto it = this->begin() ; it != this->end() ; ) {
                // make sure handlers can delete themselves
                auto current = it++;
                ( *it )( std::forward< T >( args )... );
            }
        };
    };

} // namespace sc

#endif // SCHLAZICONTROL_EVENT_HPP
