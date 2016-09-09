#ifndef SCHLAZICONTROL_INPUT_HPP
#define SCHLAZICONTROL_INPUT_HPP

#include <cstddef>
#include <string>

#include <boost/signals2/signal.hpp>

#include "component.hpp"
#include "utility.hpp"

namespace sc {

    class ChannelValue;

	class Input
		: public Component
	{
	public:
		using InputChangeEvent = boost::signals2::signal< void ( ChannelValue const& ) >;

		explicit Input( std::string id );

		void subscribeInputChange( InputChangeEvent::slot_type const& handler );
		void subscribeInputChangeEx( InputChangeEvent::extended_slot_type const& handler );

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
