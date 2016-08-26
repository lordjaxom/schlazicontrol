#ifndef SCHLAZICONTROL_INPUT_GPIO_HPP
#define SCHLAZICONTROL_INPUT_GPIO_HPP

#include <cstdint>
#include <chrono>
#include <string>

#include "gpio.hpp"
#include "input.hpp"

namespace sc {

	class Manager;
	class PropertyNode;

	class GpioInput
		: public Input
	{
	public:
		GpioInput( Manager& manager, std::string const& id, PropertyNode const& properties );

		bool get() const { return value_; }

		virtual std::size_t channels() const override { return 1; }

	private:
		void poll( std::chrono::microseconds const& elapsed );

		Manager& manager_;
		GpioDevice device_;
		std::uint16_t gpioPin_;
		bool value_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_INPUT_GPIO_HPP
