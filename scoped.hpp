#ifndef SCHLAZICONTROL_SCOPED_HPP
#define SCHLAZICONTROL_SCOPED_HPP

#include <functional>
#include <utility>

namespace sc {

    class Scoped
    {
    public:
        Scoped( std::function< void () > handler = []{} );
        Scoped( Scoped const& ) = delete;
        Scoped( Scoped&& other );
        ~Scoped();

        template< typename Type, typename ...Args >
        Scoped( Type& target, Args&&... args )
                : Scoped()
        {
            push( target, std::forward< Args >( args )... );
        }

        void push( std::function< void () > handler );

        template< typename Type, typename ...Args >
        void push( Type& target, Args&&... args )
        {
            push( [&target, &args...] { target = Type( std::forward< Args >( args )... ); } );
        }

    private:
        std::function< void () > handler_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_SCOPED_HPP
