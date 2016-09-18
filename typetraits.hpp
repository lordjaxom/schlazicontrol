#ifndef SCHLAZICONTROL_TYPETRAITS_HPP
#define SCHLAZICONTROL_TYPETRAITS_HPP

#include <type_traits>

namespace sc {

    template< typename Type > constexpr bool IsIntegral() { return std::is_integral< Type >::value; }
    template< typename Type > constexpr bool IsArithmetic() { return std::is_arithmetic< Type >::value; }
    template< typename Type > constexpr bool IsUnsigned() { return std::is_unsigned< Type >::value; }
    template< typename Type > constexpr bool IsSigned() { return std::is_signed< Type >::value; }
    template< typename Type > constexpr bool IsEnum() { return std::is_enum< Type >::value; }

    template< typename Type, typename Other > constexpr bool IsSame() { return std::is_same< Type, Other >::value; }
    template< typename Base, typename Derived > constexpr bool IsBaseOf() { return std::is_base_of< Base, Derived >::value; }

} // namespace sc

#endif //SCHLAZICONTROL_TYPETRAITS_HPP
