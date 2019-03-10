#include <algorithm>
#include <tuple>
#include <utility>

#include <boost/iterator/zip_iterator.hpp>

#include "core/input.hpp"
#include "core/logging.hpp"
#include "core/manager.hpp"
#include "modules/wiringPi/output_pwm.hpp"
#include "modules/wiringPi/wiringPi.hpp"
#include "utility/algorithm.hpp"

using namespace std;

using boost::make_zip_iterator;

namespace sc {

	static Logger logger( "output_pwm" );

	static PropertyKey const wiringPiProperty( "wiringPi" );
	static PropertyKey const gpioPinsProperty( "gpioPins" );
    static PropertyKey const inputProperty( "input" );

	SoftPwmOutput::SoftPwmOutput( string&& id, Manager& manager, PropertyNode const& properties )
		: Component( move( id ) )
        , Output( manager, properties[ inputProperty ] )
		, manager_( manager )
		, wiringPi_( manager_.get< WiringPi >( *this, properties[ wiringPiProperty ] ) )
        , pins_( properties[ gpioPinsProperty ].as< vector< uint16_t > >() )
	{
        manager_.readyEvent().subscribe( [this] {
            for_each( pins_.begin(), pins_.end(), [&]( auto pin ) {
                wiringPi_.pinMode( pin, WiringPi::output );
                wiringPi_.softPwmCreate( pin );
            } );
        }, true );
    }

	void SoftPwmOutput::set( Input const& input, ChannelBuffer const& values )
	{
        values_ = values;
        auto pin = pins_.begin();
        for_each( values_.begin(), values_.end(), [&]( auto const& value ) {
            wiringPi_.softPwmWrite( *pin++, (uint16_t) value.get() );
        } );
	}

    void SoftPwmOutput::doStatistics( ostream& os ) const
    {
        os << "\n\t\tvalues: " << makeStatistics( values_ );
    }


    static OutputRegistry< SoftPwmOutput > registry( "softPwm" );

} // namespace sc
