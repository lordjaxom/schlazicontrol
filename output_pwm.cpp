#include <algorithm>
#include <utility>

#include "input.hpp"
#include "logging.hpp"
#include "manager.hpp"
#include "output_pwm.hpp"
#include "types.hpp"
#include "utility.hpp"

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
                gpioPins.cbegin(), gpioPins.cend(), back_inserter( gpioPins_ ),
				[this]( uint16_t gpioPin ) {
					device_.pinMode( gpioPin, GpioMode::output );
					device_.softPwmCreate( gpioPin );
					return gpioPin;
				} );
    }

	void SoftPwmOutput::set( Input const& input, ChannelBuffer const& values )
	{
        values_ = values;
        forEach( [this]( auto const& value, auto const& pin ) {
                     device_.softPwmWrite( pin, (uint16_t) value.get() );
                 }, values_.cbegin(), values_.cend(), gpioPins_.cbegin() );
	}

    void SoftPwmOutput::doStatistics( ostream& os ) const
    {
        os << "\n\t\tvalues: " << makeStatistics( values_ );
    }

    static OutputRegistry< SoftPwmOutput > registry( "softPwm" );

} // namespace sc
