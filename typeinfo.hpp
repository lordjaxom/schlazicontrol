#ifndef SCHLAZICONTROL_TYPEINFO_HPP
#define SCHLAZICONTROL_TYPEINFO_HPP

#include <chrono>
#include <string>

#include <typestring.hh>

namespace sc {

    namespace detail {

        template< typename S > struct TypeString { using String = S; };

        template< typename Type, typename Enable = void > struct TypeName;
        template<> struct TypeName< std::string > : TypeString< typestring_is( "identifier" ) > {};
        template< typename Type > struct TypeName< Type, std::enable_if_t< std::is_integral< Type >::value > >
                : TypeString< typestring_is( "number" ) > {};
        template< typename Type > struct TypeName< Type, std::enable_if_t< std::is_floating_point< Type >::value > >
                : TypeString< typestring_is( "decimal" ) > {};
        template< typename Rep, typename Period > struct TypeName< std::chrono::duration< Rep, Period > >
                : TypeString< typestring_is( "duration" ) > {};

    } // namespace detail

    template< typename Type >
    char const* typeName()
    {
        return detail::TypeName< std::decay_t< Type > >::String::data();
    }

    template< typename Type >
    char const* typeName( Type const& value )
    {
        return typeName< Type >();
    }

} // namespace sc

#endif //SCHLAZICONTROL_TYPEINFO_HPP
