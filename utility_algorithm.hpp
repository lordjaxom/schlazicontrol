#ifndef SCHLAZICONTROL_UTILITY_ALGORITHM_HPP
#define SCHLAZICONTROL_UTILITY_ALGORITHM_HPP

namespace sc {

    /**
     * for_each algorithm for arbitrary number of iterators
     */

    template< typename Function, typename InputIt, typename ...InputIts >
    void forEach( Function function, InputIt first, InputIt last, InputIts... others )
    {
        for ( ; first != last ; ++first, void( ++others... ) ) {
            function( *first, *others... );
        }
    }

    /**
     * transform algorithm for arbitrary number of iterators
     */

    template< typename Function, typename InputIt, typename OutputIt, typename ...InputIts >
    OutputIt transform( Function function, InputIt first, InputIt last, OutputIt dest, InputIts... others )
    {
        while ( first != last ) {
            *dest++ = function( *first++, *others++... );
        }
        return dest;
    }

} // namespace sc

#endif //SCHLAZICONTROL_UTILITY_ALGORITHM_HPP
