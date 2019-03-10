#ifndef SCHLAZICONTROL_CORE_OPTIONAL_HPP
#define SCHLAZICONTROL_CORE_OPTIONAL_HPP

#if __cplusplus >= 201700 && __has_include( <optional> )
#   include <optional>
#   define SCHLAZICONTROL_OPTIONAL_NAMESPACE std
#elif __has_include( <experimental/optional> )
#   include <experimental/optional>
#   define SCHLAZICONTROL_OPTIONAL_NAMESPACE std::experimental
#else
#   include <boost/utility/string_view.hpp>
#   define SCHLAZICONTROL_OPTIONAL_NAMESPACE boost
#endif

namespace sc {

    using SCHLAZICONTROL_OPTIONAL_NAMESPACE::optional;
    using SCHLAZICONTROL_OPTIONAL_NAMESPACE::make_optional;
    using SCHLAZICONTROL_OPTIONAL_NAMESPACE::nullopt_t;
    using SCHLAZICONTROL_OPTIONAL_NAMESPACE::nullopt;
    using SCHLAZICONTROL_OPTIONAL_NAMESPACE::bad_optional_access;

} // namespace sc

#undef SCHLAZICONTROL_OPTIONAL_NAMESPACE

#endif //SCHLAZICONTROL_CORE_OPTIONAL_HPP
