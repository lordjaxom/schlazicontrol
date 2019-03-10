#include <utility>

#include "core/manager.hpp"
#include "modules/wiringPi/input_gpio.hpp"
#include "modules/wiringPi/wiringPi.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

	static PropertyKey const wiringPiProperty( "wiringPi" );
	static PropertyKey const gpioPinProperty( "gpioPin" );
	static PropertyKey const pullProperty( "pull", "off" );

	GpioInput::GpioInput( string&& id, Manager& manager, PropertyNode const& properties )
		: Component( move( id ) )
        , manager_( manager )
		, wiringPi_( manager_.get< WiringPi >( *this, properties[ wiringPiProperty ] ) )
		, pin_( properties[ gpioPinProperty ].as< uint16_t >() )
	{
        WiringPi::PullUpDown pull = properties[ pullProperty ].as< WiringPi::PullUpDown >();
        manager_.readyEvent().subscribe( [this, pull] {
            wiringPi_.pinMode( pin_, WiringPi::PinMode::input );
            wiringPi_.pullUpDnControl( pin_, pull );
        } );

		manager_.pollEvent().subscribe( [this]( auto ) { this->poll(); } );
	}

	void GpioInput::poll()
	{
		bool lastValue = value_;
		value_ = wiringPi_.digitalRead( pin_ );
		if ( lastValue != value_ ) {
			inputChangeEvent_( ChannelValue( value_ ) );
		}
	}

	void GpioInput::doStatistics( ostream& os ) const
	{
		os << ", value: " << value_;
	}


	static InputRegistry< GpioInput > registry( "gpio" );

} // namespace sc
