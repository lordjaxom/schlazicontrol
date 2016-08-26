#ifndef SCHLAZICONTROL_EVENTS_HPP
#define SCHLAZICONTROL_EVENTS_HPP

#include <boost/signals2/connection.hpp>
#include <boost/signals2/signal.hpp>

namespace sc {

    template< typename ...Args >
    using Event = boost::signals2::signal< void ( Args... ) >;
    using EventConnection = boost::signals2::connection;

} // namespace sc

#endif // SCHLAZICONTROL_EVENTS_HPP
