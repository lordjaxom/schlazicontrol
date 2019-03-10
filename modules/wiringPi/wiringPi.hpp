#ifndef SCHLAZICONTROL_WIRINGPI_WIRINGPI_HPP
#define SCHLAZICONTROL_WIRINGPI_WIRINGPI_HPP

#include <nlohmann/json_fwd.hpp>

#include "core/component.hpp"
#include "typeinfo.hpp"

namespace sc {

    class WiringPi final
            : public Component
    {
    public:
        enum PinMode
        {
            input,
            output
        };

        enum PullUpDown
        {
            off,
            up,
            down
        };

        WiringPi( std::string&& id, Manager& manager, PropertyNode const& properties );
        ~WiringPi() override;

        void pinMode( std::uint16_t pin, PinMode mode );
        void pullUpDnControl( std::uint16_t pin, PullUpDown pull );
        void softPwmCreate( std::uint16_t pin );

        bool digitalRead( std::uint16_t pin ) const;
        void digitalWrite( std::uint16_t pin, bool value ) const;
        void softPwmWrite( std::uint16_t pin, std::uint16_t value ) const;

    protected:
        void doStatistics( std::ostream& os ) const override;
    };


    template<> struct TypeName< WiringPi::PullUpDown > : TypeString< typestring_is( "(off|up|down)" ) > {};


    void from_json( nlohmann::json const& src, WiringPi::PullUpDown& dst );

} // namespace sc

#endif // SCHLAZICONTROL_WIRINGPI_WIRINGPI_HPP
