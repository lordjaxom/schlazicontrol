#ifndef SCHLAZICONTROL_INPUT_HPP
#define SCHLAZICONTROL_INPUT_HPP

#include <cstddef>
#include <string>

#include "component.hpp"
#include "event.hpp"
#include "utility.hpp"

namespace sc {

    class ChannelBuffer;
    class Output;

    /**
     * class Input
     */

    class Input
            : public virtual Component
    {
    public:
        using InputChangeEvent = Event< void ( ChannelBuffer const& ) >;

        template< typename = std::nullptr_t > Input() {}

        virtual std::size_t emitsChannels() const = 0;

        InputChangeEvent::Interface& inputChangeEvent() { return inputChangeEvent_.interface(); }

    protected:
        InputChangeEvent inputChangeEvent_;
    };

    /**
     * class InputRegistry
     */

    template< typename Type >
	class InputRegistry final
		: public ComponentRegistry< Type >
	{
	public:
		InputRegistry( char const* name )
                : ComponentRegistry< Type >( "input", name )
        {
        }
	};

    /**
     * free functions
     */

    void checkConnection( Component const& input, Component const& output, std::size_t channels, bool accepts );
    void checkConnection( Input const& input, Output const& output );

    template< typename Receiver >
    void checkConnection(
            Component const& input, Component const& output, Receiver& receiver, std::size_t channels )
    {
        return checkConnection( input, output, channels, receiver.acceptsChannels( channels ) );
    }

    void connect( Input& input, Output const& output, Input::InputChangeEvent::Handler handler );

} // namespace sc

#endif // SCHLAZICONTROL_INPUT_HPP
