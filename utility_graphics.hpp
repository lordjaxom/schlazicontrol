#ifndef SCHLAZICONTROL_UTILITY_GRAPHICS_HPP
#define SCHLAZICONTROL_UTILITY_GRAPHICS_HPP

#include <cstdint>
#include <array>

#include <nlohmann/json_fwd.hpp>

#include "typeinfo.hpp"

namespace sc {

    class Rgb
    {
    public:
        Rgb();
        explicit Rgb( std::uint32_t value );
        Rgb( std::uint8_t red, std::uint8_t green, std::uint8_t blue );

        std::uint32_t value() const;
        std::uint8_t red() const { return values_[ 0 ]; }
        std::uint8_t green() const { return values_[ 1 ]; }
        std::uint8_t blue() const { return values_[ 2 ]; }

        Rgb& scale( double factor );

    private:
        std::array< std::uint8_t, 3 > values_;
    };


    template<> struct TypeName< Rgb > : TypeString< typestring_is( "colour" ) > {};


    void from_json( nlohmann::json const& src, Rgb& dst );

} // namespace sc

#endif // SCHLAZICONTROL_UTILITY_GRAPHICS_HPP
