#ifndef SCHLAZICONTROL_UTILITY_MATH_HPP
#define SCHLAZICONTROL_UTILITY_MATH_HPP

#include <type_traits>

namespace sc {

    template< typename Type >
    Type clip( Type value, Type min, Type max )
    {
        return value < min ? min : value > max ? max : value;
    }

    template< typename To, typename From >
    To scale( From value, From fromMin, From fromMax, To toMin, To toMax )
    {
        return (To) ( ( (std::common_type_t< To, From >) value - fromMin )
                      * ( toMax - toMin ) / ( fromMax - fromMin ) + toMin );
    }

} // namespace sc

#endif //SCHLAZICONTROL_UTILITY_MATH_HPP
