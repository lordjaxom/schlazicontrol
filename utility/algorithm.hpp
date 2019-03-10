#ifndef SCHLAZICONTROL_UTILITY_ALGORITHM_HPP
#define SCHLAZICONTROL_UTILITY_ALGORITHM_HPP

namespace sc {

    /**
     * for_each algorithm for arbitrary number of iterators
     */

    template< typename Func, typename InputIt, typename ...InputIts >
    void forEach( Func&& func, InputIt first, InputIt last, InputIts... others )
    {
        for ( ; first != last ; ++first, void( ++others... ) ) {
            std::forward< Func >( func )( *first, *others... );
        }
    }

    /**
     * transform algorithm for arbitrary number of iterators
     */

    template< typename Func, typename InputIt, typename OutputIt, typename ...InputIts >
    OutputIt transform( Func&& func, InputIt first, InputIt last, OutputIt dest, InputIts... others )
    {
        while ( first != last ) {
            *dest++ = std::forward< Func >( func )( *first++, *others++... );
        }
        return dest;
    }

} // namespace sc

#endif //SCHLAZICONTROL_UTILITY_ALGORITHM_HPP
