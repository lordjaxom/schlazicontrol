#ifndef SCHLAZICONTROL_OUTPUT_PWM_HPP
#define SCHLAZICONTROL_OUTPUT_PWM_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "gpio.hpp"
#include "core/output.hpp"
#include "types.hpp"

namespace sc {

	class Manager;
	class PropertyNode;

	class SoftPwmOutput final
		: public Output
	{
	public:
		SoftPwmOutput( std::string&& id, Manager& manager, PropertyNode const& properties );

		virtual bool acceptsChannels( std::size_t channels ) const override { return gpioPins_.size() == channels; }

	protected:
        virtual void set( Input const& input, ChannelBuffer const& values ) override;

        virtual void doStatistics( std::ostream& os ) const override;

	private:
		Manager& manager_;
		GpioDevice device_;
		std::vector< std::uint16_t > gpioPins_;
        ChannelBuffer values_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_OUTPUT_PWM_HPP
