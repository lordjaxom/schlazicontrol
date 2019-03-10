#ifndef SCHLAZICONTROL_GPIO_HPP
#define SCHLAZICONTROL_GPIO_HPP

#include <cstdint>
#include <istream>
#include <ostream>

namespace sc {

	class Manager;

    /**
     * enum class GpioMode
     */

    enum class GpioMode
	{
		input,
		output
	};

	std::istream& operator>>( std::istream& is, GpioMode& dst );
	std::ostream& operator<<( std::ostream& os, GpioMode val );

    /**
     * enum class GpioPull
     */

    enum class GpioPull
	{
		off,
		up,
		down
	};

	std::istream& operator>>( std::istream& is, GpioPull& dst );
	std::ostream& operator<<( std::ostream& os, GpioPull val );

	/**
	 * class GpioDevice
	 */

	class GpioDevice
	{
	public:
		explicit GpioDevice( Manager& manager );

		void pinMode( std::uint16_t gpioPin, GpioMode mode );
		void pullUpDnControl( std::uint16_t gpioPin, GpioPull pull );
		void softPwmCreate( std::uint16_t gpioPin );

		bool digitalRead( std::uint16_t gpioPin ) const;
		void softPwmWrite( std::uint16_t gpioPin, std::uint16_t  value ) const;

	private:
		Manager& manager_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_GPIO_HPP