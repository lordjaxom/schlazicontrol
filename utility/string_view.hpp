#ifndef SCHLAZICONTROL_UTILITY_STRING_VIEW_HPP
#define SCHLAZICONTROL_UTILITY_STRING_VIEW_HPP

#if __cplusplus >= 201700 && __has_include( <string_view> )
#   include <string_view>
#   define SCHLAZICONTROL_STRING_VIEW_NAMESPACE std
#elif __has_include( <experimental/string_view> )
#   include <experimental/string_view>
#   define SCHLAZICONTROL_STRING_VIEW_NAMESPACE std::experimental
#else
#   include <boost/utility/string_view.hpp>
#   define SCHLAZICONTROL_STRING_VIEW_NAMESPACE boost
#endif

namespace sc {

template< typename CharT, typename Traits = std::char_traits< CharT > >
using basic_string_view = SCHLAZICONTROL_STRING_VIEW_NAMESPACE::basic_string_view< CharT, Traits >;

using string_view = SCHLAZICONTROL_STRING_VIEW_NAMESPACE::string_view;
using wstring_view = SCHLAZICONTROL_STRING_VIEW_NAMESPACE::wstring_view;
using u16string_view = SCHLAZICONTROL_STRING_VIEW_NAMESPACE::u16string_view;
using u32string_view = SCHLAZICONTROL_STRING_VIEW_NAMESPACE::u32string_view;

} // namespace sc

#undef SCHLAZICONTROL_STRING_VIEW_NAMESPACE

#endif //SCHLAZICONTROL_UTILITY_STRING_VIEW_HPP
