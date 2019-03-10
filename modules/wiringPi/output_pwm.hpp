#ifndef SCHLAZICONTROL_OUTPUT_PWM_HPP
#define SCHLAZICONTROL_OUTPUT_PWM_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "core/output.hpp"
#include "types.hpp"

namespace sc {

	class WiringPi;

	/**
	 * class SoftPwmOutput
	 */

	class SoftPwmOutput final
		: public Output
	{
	public:
		SoftPwmOutput( std::string&& id, Manager& manager, PropertyNode const& properties );

		bool acceptsChannels( std::size_t channels ) const override { return pins_.size() == channels; }

	protected:
        void set( Input const& input, ChannelBuffer const& values ) override;

        void doStatistics( std::ostream& os ) const override;

	private:
		Manager& manager_;
		WiringPi& wiringPi_;
		std::vector< std::uint16_t > pins_;
        ChannelBuffer values_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_OUTPUT_PWM_HPP
