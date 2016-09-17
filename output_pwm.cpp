#include <algorithm>
#include <utility>

#include "input.hpp"
#include "logging.hpp"
#include "manager.hpp"
#include "output_pwm.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

	static Logger logger( "output_pwm" );

	static PropertyKey const gpioPinsProperty( "gpioPins" );
    static PropertyKey const inputProperty( "input" );

	SoftPwmOutput::SoftPwmOutput( string&& id, Manager& manager, PropertyNode const& properties )
		: Component( move( id ) )
        , Output( manager, properties[ inputProperty ] )
		, manager_( manager )
		, device_( manager_ )
	{
		auto gpioPins = properties[ gpioPinsProperty ].as< uint16_t[] >();
		transform(
                gpioPins.begin(), gpioPins.end(), back_inserter( gpioPins_ ),
				[this]( uint16_t gpioPin ) {
					device_.pinMode( gpioPin, GpioMode::output );
					device_.softPwmCreate( gpioPin );
					return gpioPin;
				} );
    }

	void SoftPwmOutput::set( Input const& input, ChannelBuffer const& values )
	{
        auto valuesIt = values.begin(), valuesEnd = values.end();
        auto pinsIt = gpioPins_.begin(), pinsEnd = gpioPins_.end();
        for ( ; valuesIt != valuesEnd ; ++valuesIt, ++pinsIt ) {
            device_.softPwmWrite( *pinsIt, (uint16_t) valuesIt->get() );
        }
	}

	static OutputRegistry< SoftPwmOutput > registry( "softPwm" );

} // namespace sc
