#ifndef SCHLAZICONTROL_EVENT_HPP
#define SCHLAZICONTROL_EVENT_HPP

#include <algorithm>
#include <functional>
#include <list>
#include <memory>
#include <tuple>
#include <utility>

#include <boost/iterator/iterator_adaptor.hpp>

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
        template< typename Signature > struct EventHandlerHolder;
        template< typename Signature > struct EventHandlerIterator;
    } // namespace detail

    template< typename Signature >
    class EventInterface;

    template< typename ...Args >
    class EventInterface< void ( Args... ) >
    {
    public:
        using Holder = detail::EventHandlerHolder< void ( Args... ) >;

        using Handler = std::function< void ( Args... ) >;
        using ExtendedHandler = std::function< void ( EventConnection const&, Args... ) >;

        using const_iterator = detail::EventHandlerIterator< void ( Args... ) >;

        EventConnection subscribe( ExtendedHandler handler )
        {
            auto it = handlers_.emplace( handlers_.end(), locked_ );
            EventConnection connection( [this, it] { erase( it ); } );
            *it = [handler, connection]( Args... args ) { handler( connection, args... ); };
            return connection;
        }

        EventConnection subscribe( Handler handler, bool oneShot = false )
        {
            auto it = handlers_.emplace( handlers_.end(), locked_ );
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
        void lock()
        {
            locked_ = true;
        }

        void unlock()
        {
            if ( locked_ ) {
                handlers_.remove_if( []( Holder& holder ) {
                    holder.inserted = false;
                    return holder.erased;
                } );
                locked_ = false;
            }
        }

        const_iterator begin() const { return const_iterator( handlers_.begin(), handlers_.end() ); }
        const_iterator end() const { return const_iterator( handlers_.end(), handlers_.end() ); }

    private:
        void erase( typename std::list< Holder >::iterator it )
        {
            if ( locked_ ) {
                it->erased = true;
            }
            else {
                handlers_.erase( it );
            }
        }

        std::list< Holder > handlers_;
        bool locked_ {};
    };

    namespace detail {

        template< typename Signature >
        struct EventHandlerHolder
        {
            using Handler = typename EventInterface< Signature >::Handler;

            EventHandlerHolder( bool locked ) : inserted( locked ) {}
            EventHandlerHolder& operator=( Handler&& handler ) { this->handler = std::move( handler ); return *this; }

            Handler handler;
            bool erased {};
            bool inserted;
        };

        template< typename Signature >
        struct EventHandlerIterator
                : boost::iterator_adaptor<
                        EventHandlerIterator< Signature >,
                        typename std::list< typename EventInterface< Signature >::Holder >::const_iterator,
                        typename EventInterface< Signature >::Handler const,
                        boost::incrementable_traversal_tag >
        {
            friend class boost::iterator_core_access;

            explicit EventHandlerIterator(
                    typename EventHandlerIterator::base_type it,
                    typename EventHandlerIterator::base_type last )
                    : EventHandlerIterator::iterator_adaptor_( it )
                    , last_( last )
            {
            }

        private:
            typename EventHandlerIterator::reference dereference() const
            {
                return this->base()->handler;
            }

            void increment()
            {
                do {
                    ++this->base_reference();
                } while ( this->base() != last_ && ( this->base()->erased || this->base()->inserted ) );
            }

            typename EventHandlerIterator::base_type last_;
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

        Interface& interface() { return *static_cast< Interface* >( this ); }

        template< typename ...T >
        void operator()( T&&... args )
        {
            std::shared_ptr< void > guard( nullptr, [this]( void const* ) { this->unlock(); } );
            this->lock();
            std::for_each(
                    this->begin(), this->end(),
                    [&args...]( Handler const& handler ) { handler( std::forward< T >( args )... ); } );
        };
    };

} // namespace sc

#endif // SCHLAZICONTROL_EVENT_HPP
