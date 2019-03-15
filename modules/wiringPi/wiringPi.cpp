#include <system_error>
#include <utility>

#include <nlohmann/json.hpp>

#include <wiringPi.h>
#include <softPwm.h>

#include "core/manager.hpp"
#include "modules/wiringPi/wiringPi.hpp"
#include "wiringPi.hpp"

using namespace std;
using namespace nlohmann;

namespace sc {

    static int mapPinMode( WiringPi::PinMode mode )
    {
        switch ( mode ) {
            case WiringPi::input: return INPUT;
            case WiringPi::output: return OUTPUT;
        }
        throw invalid_argument( "invalid constant for PinMode" );
    }

    static int mapPullUpDown( WiringPi::PullUpDown pull )
    {
        switch ( pull ) {
            case WiringPi::off: return PUD_OFF;
            case WiringPi::up: return PUD_UP;
            case WiringPi::down: return PUD_DOWN;
        }
        throw invalid_argument( "invalid constant for PullUpDown" );
    }

    WiringPi::WiringPi( string&& id, Manager& manager, PropertyNode const& properties )
            : Component( move( id ) )
    {
        manager.readyEvent().subscribe( [this] {
            if ( ::wiringPiSetupGpio() == -1 ) {
                throw system_error( errno, system_category(), "couldn't initialize wiringPi" );
            }
        }, true );
    }

    WiringPi::~WiringPi() = default;

    void WiringPi::pinMode( uint16_t pin, PinMode mode )
    {
        ::pinMode( pin, mapPinMode( mode ) );
    }

    void WiringPi::pullUpDnControl( uint16_t pin, PullUpDown pull )
    {
        ::pullUpDnControl( pin, mapPullUpDown( pull ) );
    }

    void WiringPi::softPwmCreate( uint16_t pin )
    {
        if ( ::softPwmCreate( pin, 0, 100 ) != 0 ) {
            throw system_error( errno, system_category(), str( "couldn't initialize softpwm on pin ", pin ) );
        }
    }

    bool WiringPi::digitalRead( uint16_t pin ) const
    {
        return ::digitalRead( pin ) == HIGH;
    }

    void WiringPi::digitalWrite( uint16_t pin, bool value ) const
    {
        ::digitalWrite( pin, value ? HIGH : LOW );
    }

    void WiringPi::softPwmWrite( uint16_t pin, uint16_t value ) const
    {
        ::softPwmWrite( pin, value );
    }

    void WiringPi::doStatistics( std::ostream& os ) const
    {
    }


    void from_json( json const& src, WiringPi::PullUpDown& dst )
    {
        auto str = src.get< string >();
        if ( str == "off" ) {
            dst = WiringPi::PullUpDown::off;
        } else if ( str == "up" ) {
            dst = WiringPi::PullUpDown::up;
        } else if ( str == "down" ) {
            dst = WiringPi::PullUpDown::down;
        } else {
            throw runtime_error( "not a GpioPull!" ); // TODO
        }
    }


    static ComponentRegistry< WiringPi > registry( "wiringPi" );

} // namespace sc
