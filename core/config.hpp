#ifndef SCHLAZICONTROL_CORE_CONFIG_HPP
#define SCHLAZICONTROL_CORE_CONFIG_HPP

#if defined( __MINGW32__ )
#   define SCHLAZICONTROL_FORK 0
/* ... */
#else
#   define SCHLAZICONTROL_FORK 1
#endif

#define SCHLAZICONTROL_IF( feature, result ) SCHLAZICONTROL_IF_IMPL1( SCHLAZICONTROL_ ## feature, result )
#define SCHLAZICONTROL_IF_IMPL1( feature, result ) SCHLAZICONTROL_IF_IMPL2( feature, result )
#define SCHLAZICONTROL_IF_IMPL2( feature, result ) SCHLAZICONTROL_IF_ ## feature ( result )
#define SCHLAZICONTROL_IF_1( result ) result
#define SCHLAZICONTROL_IF_0( result )

#define SCHLAZICONTROL_UNLESS( feature, result ) SCHLAZICONTROL_UNLESS_IMPL1( SCHLAZICONTROL_ ## feature, result )
#define SCHLAZICONTROL_UNLESS_IMPL1( feature, result ) SCHLAZICONTROL_UNLESS_IMPL2( feature, result )
#define SCHLAZICONTROL_UNLESS_IMPL2( feature, result ) SCHLAZICONTROL_UNLESS_ ## feature ( result )
#define SCHLAZICONTROL_UNLESS_1( result )
#define SCHLAZICONTROL_UNLESS_0( result ) result

#endif //SCHLAZICONTROL_CORE_CONFIG_HPP
