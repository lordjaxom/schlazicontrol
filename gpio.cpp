#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>

#include <boost/algorithm/string/predicate.hpp>

#include "logging.hpp"
#include "gpio.hpp"
#include "manager.hpp"

#include <wiringPi.h>
#include <softPwm.h>

using namespace std;
using namespace boost::algorithm;

namespace sc {

	static Logger logger( "gpio" );

	static int gpioMapPinMode( GpioMode mode )
	{
		switch ( mode ) {
			case GpioMode::input: return INPUT;
			case GpioMode::output: return OUTPUT;
		}
		throw invalid_argument( "invalid constant for GpioMode" );
	}

	static int gpioMapPinPull( GpioPull pull )
	{
		switch ( pull ) {
			case GpioPull::off: return PUD_OFF;
			case GpioPull::up: return PUD_UP;
			case GpioPull::down: return PUD_DOWN;
		}
		throw invalid_argument( "invalid constant for GpioPull" );
	}

	static void gpioWiringPiSetupGpio()
	{
		static auto once = [] {
			if ( ::wiringPiSetupGpio() == -1 ) {
				throw system_error( errno, system_category(), "couldn't initialize wiringPi" );
			}
			return nullptr;
		}();
	}

	static void gpioPinMode( uint16_t gpioPin, GpioMode mode )
	{
		gpioWiringPiSetupGpio();
		::pinMode( gpioPin, gpioMapPinMode( mode ) );
	}

	static void gpioPullUpDnControl( uint16_t gpioPin, GpioPull pull )
	{
		gpioWiringPiSetupGpio();
		::pullUpDnControl( gpioPin, gpioMapPinPull( pull ));
	}

	static void gpioSoftPwmCreate( uint16_t gpioPin )
	{
		gpioWiringPiSetupGpio();
		if ( ::softPwmCreate( gpioPin, 0, 100 ) != 0 ) {
			throw system_error( errno, system_category(), str( "couldn't initialize softpwm on pin ", gpioPin ) );
		}
	}

	/**
     * enum class GpioMode
     */

	istream& operator>>( istream& is, GpioMode& dst )
	{
		string s;
		if ( is >> s ) {
			if ( iequals( s, "input" ) ) {
				dst = GpioMode::input;
			}
			else if ( iequals( s, "output" ) ) {
				dst = GpioMode::output;
			}
			else {
				is.setstate( ios::badbit );
			}
		}
		return is;
	}

	ostream& operator<<( ostream& os, GpioMode val )
	{
		switch ( val ) {
			case GpioMode::input:
				return os << "input";
			case GpioMode::output:
				return os << "output";
		}
		throw invalid_argument( "invalid constant for GpioMode" );
	}

	/**
     * enum class GpioPull
     */

	istream& operator>>( istream& is, GpioPull& dst )
	{
		string s;
		if ( is >> s ) {
			if ( iequals( s, "off" ) ) {
				dst = GpioPull::off;
			}
			else if ( iequals( s, "up" ) ) {
				dst = GpioPull::up;
			}
			else if ( iequals( s, "down" ) ) {
				dst = GpioPull::down;
			}
			else {
				is.setstate( ios::badbit );
			}
		}
		return is;
	}

	ostream& operator<<( ostream& os, GpioPull val )
	{
		switch ( val ) {
			case GpioPull::off:
				return os << "off";
			case GpioPull::up:
				return os << "up";
			case GpioPull::down:
				return os << "down";
		}
		throw invalid_argument( "invalid constant for GpioPull" );
	}

	/**
	 * class GpioDevice
	 */

	GpioDevice::GpioDevice( Manager& manager )
		: manager_( manager )
	{
	}

	void GpioDevice::pinMode( uint16_t gpioPin, GpioMode mode )
	{
		manager_.readyEvent().subscribe( [gpioPin, mode] { gpioPinMode( gpioPin, mode ); }, true );
	}

	void GpioDevice::pullUpDnControl( uint16_t gpioPin, GpioPull pull )
	{
		manager_.readyEvent().subscribe( [gpioPin, pull] { gpioPullUpDnControl( gpioPin, pull ); }, true );
	}

	void GpioDevice::softPwmCreate( uint16_t gpioPin )
	{
		manager_.readyEvent().subscribe( [gpioPin] { gpioSoftPwmCreate( gpioPin ); }, true );
	}

	bool GpioDevice::digitalRead( uint16_t gpioPin ) const
	{
		return ::digitalRead( gpioPin ) == HIGH;
	}

	void GpioDevice::softPwmWrite( uint16_t gpioPin, uint16_t value ) const
	{
		::softPwmWrite( gpioPin, value );
	}

} // namespace sc
