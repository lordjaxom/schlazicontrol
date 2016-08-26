#include <algorithm>

#include "events.hpp"
#include "input_gpio.hpp"
#include "manager.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

	static PropertyKey const gpioPinProperty( "gpioPin" );
	static PropertyKey const pullProperty( "pull", "off" );

	GpioInput::GpioInput( Manager& manager, string const& id, PropertyNode const& properties )
		: Input( id )
        , manager_( manager )
		, device_( manager_ )
		, gpioPin_( properties[ gpioPinProperty ].as< uint16_t >() )
	{
		device_.pinMode( gpioPin_, GpioMode::input );
		device_.pullUpDnControl( gpioPin_, properties[ pullProperty ].as< GpioPull >() );

		manager_.subscribePollEvent( [this]( chrono::microseconds const& elapsed ) { poll( elapsed ); } );
	}

	void GpioInput::poll( chrono::microseconds const& elapsed )
	{
		bool lastValue = value_;
		value_ = device_.digitalRead( gpioPin_ );
		if ( lastValue != value_ ) {
			raiseInputChange( { value_ } );
		}
	}

	__attribute__(( unused )) static InputRegistry< GpioInput > registry( "gpio" );

} // namespace sc
