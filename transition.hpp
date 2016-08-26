#ifndef SCHLAZICONTROL_TRANSITION_HPP
#define SCHLAZICONTROL_TRANSITION_HPP

#include <memory>
#include <string>

#include "component.hpp"
#include "utility.hpp"

namespace sc {

    class ChannelBuffer;
    class Connection;

    class TransitionInstance
    {
    public:
        TransitionInstance();
        TransitionInstance( TransitionInstance const& ) = delete;
        virtual ~TransitionInstance();

        virtual bool transform( Connection const& connection, ChannelBuffer& values ) = 0;
    };

    class Transition
        : public Component
    {
    public:
        explicit Transition( std::string const& id );

        virtual std::unique_ptr< TransitionInstance > instantiate() const = 0;
    };

    template< typename Type >
    class TransitionRegistry
        : public ComponentRegistry< Type >
    {
    public:
        TransitionRegistry( char const* name ) : ComponentRegistry< Type >( str( "transition.", name ) ) {}
    };

} // namespace sc

#endif // SCHLAZICONTROL_TRANSITION_HPP
