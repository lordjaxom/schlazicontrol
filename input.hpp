#ifndef SCHLAZICONTROL_INPUT_HPP
#define SCHLAZICONTROL_INPUT_HPP

#include <cstddef>
#include <string>

#include "component.hpp"
#include "event.hpp"
#include "utility.hpp"

namespace sc {

    class ChannelValue;

	class Input
		: public Component
	{
        using InputChangeEvent = Event< void ( ChannelValue const& ) >;

	public:
        explicit Input( std::string id );

		EventConnection subscribeInputChange( InputChangeEvent::Handler handler );

		virtual std::size_t channels() const = 0;

	protected:
		void raiseInputChange( ChannelValue const& value ) const;

	private:
        InputChangeEvent inputChangeEvent_;
	};

	template< typename Type >
	class InputRegistry
		: public ComponentRegistry< Type >
	{
	public:
		InputRegistry( char const* name ) : ComponentRegistry< Type >( str( "input.", name ) ) {}
	};

} // namespace sc

#endif // SCHLAZICONTROL_INPUT_HPP
