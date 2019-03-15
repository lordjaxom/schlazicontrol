#ifndef SCHLAZICONTROL_INPUT_HPP
#define SCHLAZICONTROL_INPUT_HPP

#include <cstddef>
#include <string>
#include <utility>

#include "core/component.hpp"
#include "event.hpp"
#include "forward.hpp"

namespace sc {

    /**
     * class Input
     */

    class Input
            : public virtual Component
    {
    public:
        using InputChangeEvent = Event< void ( ChannelBuffer const& ) >;

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
		explicit InputRegistry( char const* name ) noexcept
                : ComponentRegistry< Type >( "input", name ) {}
	};

} // namespace sc

#endif // SCHLAZICONTROL_INPUT_HPP
