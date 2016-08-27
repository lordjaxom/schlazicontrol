#ifndef SCHLAZICONTROL_EVENTS_HPP
#define SCHLAZICONTROL_EVENTS_HPP

#include <functional>
#include <list>
#include <utility>

namespace sc {

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

        void subscribe( Handler handler )
        {
            handlers_.emplace_back( std::move( handler ) );
        }

    private:
        std::list< Handler > handlers_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_EVENTS_HPP
