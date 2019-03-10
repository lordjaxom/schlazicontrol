#include <nlohmann/json.hpp>

#include "utility_graphics.hpp"

using namespace std;
using namespace nlohmann;

namespace sc {

    static uint32_t channelsToValue( array< uint8_t, 3 > const& values )
    {
        return ( values[ 0 ] << 16 ) | ( values[ 1 ] << 8 ) | ( values[ 2 ] << 0 );
    }

    static array< uint8_t, 3 > valueToChannels( uint32_t value )
    {
        return { (uint8_t) ( ( value >> 16 ) & 0xff ), (uint8_t) ( ( value >> 8 ) & 0xff ), (uint8_t) ( ( value >> 0 ) & 0xff ) };
    }

    Rgb::Rgb()
            : values_() {}

    Rgb::Rgb( std::uint32_t value )
            : values_( valueToChannels( value ) ) {}

    Rgb::Rgb( std::uint8_t red, std::uint8_t green, std::uint8_t blue )
            : values_ { red, green, blue } {}

    std::uint32_t Rgb::value() const
    {
        return channelsToValue( values_ );
    }

    Rgb& Rgb::scale( double factor )
    {
        values_[ 0 ] = values_[ 0 ] * factor;
        values_[ 1 ] = values_[ 1 ] * factor;
        values_[ 2 ] = values_[ 2 ] * factor;
        return *this;
    }


    void from_json( json const& src, Rgb& dst )
    {
        auto str = src.get< string >();
        size_t consumed = 0;
        uint32_t result;
        if ( str.size() != 6 || ( result = stoul( str, &consumed, 16 ) ), consumed != str.size() ) {
            throw runtime_error( "not a color!" ); // TODO
        }
        dst = Rgb( result );
    }

} // namespace sc
