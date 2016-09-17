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

        auto& input = manager_.get< Input >( this->id(), properties[ inputProperty ] );
        connect( input, *this, [this]( ChannelBuffer const& values ) { set( values ); } );
    }

	void SoftPwmOutput::set( ChannelBuffer const& values )
	{
		if ( values.size() != gpioPins_.size() ) {
			logger.warning(
					"soft pwm output ", id(), " called with ", values.size(), " but has ",
					gpioPins_.size(), " channels" );
		}

		for ( size_t i = 0; i < values.size() || i < gpioPins_.size() ; ++i ) {
			device_.softPwmWrite( gpioPins_[ i % gpioPins_.size() ], (uint16_t) values[ i % values.size() ].get() );
		}
	}

	static OutputRegistry< SoftPwmOutput > registry( "softPwm" );

} // namespace sc
