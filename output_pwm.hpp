#ifndef SCHLAZICONTROL_OUTPUT_PWM_HPP
#define SCHLAZICONTROL_OUTPUT_PWM_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "gpio.hpp"
#include "output.hpp"

namespace sc {

	class ChannelBuffer;
	class Manager;
	class PropertyNode;

	class SoftPwmOutput
		: public Output
	{
	public:
		SoftPwmOutput( std::string&& id, Manager& manager, PropertyNode const& properties );

		virtual bool acceptsChannels( std::size_t channels ) const override { return gpioPins_.size() == channels; }

	private:
		void set( ChannelBuffer const& values );

		Manager& manager_;
		GpioDevice device_;
		std::vector< std::uint16_t > gpioPins_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_OUTPUT_PWM_HPP
