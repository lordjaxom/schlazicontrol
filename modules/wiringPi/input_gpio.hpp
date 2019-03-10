#ifndef SCHLAZICONTROL_INPUT_GPIO_HPP
#define SCHLAZICONTROL_INPUT_GPIO_HPP

#include <cstdint>

#include "core/input.hpp"

namespace sc {

    class WiringPi;

	class GpioInput final
		    : public Input
	{
	public:
		GpioInput( std::string&& id, Manager& manager, PropertyNode const& properties );

		std::size_t emitsChannels() const override { return 1; }

    protected:
		void doStatistics( std::ostream& os ) const override;

	private:
		void poll();

		Manager& manager_;
		WiringPi& wiringPi_;
		std::uint16_t pin_;
		bool value_ {};
	};

} // namespace sc

#endif // SCHLAZICONTROL_INPUT_GPIO_HPP
