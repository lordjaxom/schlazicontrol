#ifndef SCHLAZICONTROL_UTILITY_STRING_HPP
#define SCHLAZICONTROL_UTILITY_STRING_HPP

#include <iosfwd>
#include <sstream>
#include <string>
#include <utility>

namespace sc {

    namespace detail {

        inline void strWrite( std::ostream& os ) {}

        template< typename Arg0, typename ...Args >
        void strWrite( std::ostream& os, Arg0&& arg0, Args&&... args )
        {
            os << std::forward< Arg0 >( arg0 );
            strWrite( os, std::forward< Args >( args )... );
        }

    } // namespace detail

    template< typename ...Args >
    std::string str( Args&&... args )
    {
        std::ostringstream os;
        detail::strWrite( os, std::forward< Args >( args )... );
        return os.str();
    }

} // namespace sc

#endif //SCHLAZICONTROL_UTILITY_STRING_HPP
