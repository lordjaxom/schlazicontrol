#ifndef SCHLAZICONTROL_INPUT_GPIO_HPP
#define SCHLAZICONTROL_INPUT_GPIO_HPP

#include <cstdint>

#include "gpio.hpp"
#include "input.hpp"

namespace sc {

	class GpioInput final
		    : public Input
	{
	public:
		GpioInput( std::string&& id, Manager& manager, PropertyNode const& properties );

		virtual std::size_t emitsChannels() const override { return 1; }

    protected:
		virtual void doStatistics( std::ostream& os ) const override;

	private:
		void poll();

		Manager& manager_;
		GpioDevice device_;
		std::uint16_t gpioPin_;
		bool value_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_INPUT_GPIO_HPP
