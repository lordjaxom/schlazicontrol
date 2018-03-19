#ifndef SCHLAZICONTROL_UTILITY_GRAPHICS_HPP
#define SCHLAZICONTROL_UTILITY_GRAPHICS_HPP

#include <cstdint>
#include <array>

namespace sc {

    class Rgb
    {
    public:
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

} // namespace sc

#endif // SCHLAZICONTROL_UTILITY_GRAPHICS_HPP
