#ifndef SCHLAZICONTROL_INPUT_HPP
#define SCHLAZICONTROL_INPUT_HPP

#include <cstddef>
#include <string>

#include "component.hpp"
#include "events.hpp"
#include "utility.hpp"

namespace sc {

    class ChannelValue;

	class Input
		: public Component
	{
        using InputChangeEvent = Event< void ( ChannelValue const& ) >;

	public:
        explicit Input( std::string id );

		void subscribeInputChange( InputChangeEvent::Handler handler );

		virtual bool acceptsChannels( std::size_t channels ) const final;

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
