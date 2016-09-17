#include <utility>

#include "input_gpio.hpp"
#include "logging.hpp"
#include "manager.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

	static Logger logger( "input_gpio" );

	static PropertyKey const gpioPinProperty( "gpioPin" );
	static PropertyKey const pullProperty( "pull", "off" );

	GpioInput::GpioInput( string&& id, Manager& manager, PropertyNode const& properties )
		: Component( move( id ) )
        , manager_( manager )
		, device_( manager_ )
		, gpioPin_( properties[ gpioPinProperty ].as< uint16_t >() )
        , value_()
	{
		device_.pinMode( gpioPin_, GpioMode::input );
		device_.pullUpDnControl( gpioPin_, properties[ pullProperty ].as< GpioPull >() );

		manager_.pollEvent().subscribe( [this]( chrono::nanoseconds elapsed ) { poll(); } );
	}

	void GpioInput::poll()
	{
		bool lastValue = value_;
		value_ = device_.digitalRead( gpioPin_ );
		if ( lastValue != value_ ) {
            logger.debug( "gpio on pin ", gpioPin_, " changed to ", value_ );
			inputChangeEvent_( ChannelBuffer { value_ } );
		}
	}

	void GpioInput::statistics( ostream& os ) const
	{
		os << "gpio: " << gpioPin_ << "; "
           << "value: " << value_;
	}

	static InputRegistry< GpioInput > registry( "gpio" );

} // namespace sc
