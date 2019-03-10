#ifndef SCHLAZICONTROL_MQTT_MQTT_HPP
#define SCHLAZICONTROL_MQTT_MQTT_HPP

#include <functional>
#include <memory>

#include "core/component.hpp"

namespace sc {

    namespace detail {
        class Mqtt;
    } // namespace detail

    /**
     * class Mqtt
     */

    class Mqtt final
            : public Component
    {
    public:
        using MessageHandler = std::function< void ( std::string const& payload ) >;

        Mqtt( std::string&& id, Manager& manager, PropertyNode const& properties );
        ~Mqtt() override;

        void publish( std::string topic, std::string payload );
        void subscribe( std::string topic, MessageHandler handler );

    protected:
        void doStatistics( std::ostream& os ) const override;

    private:
        std::unique_ptr< detail::Mqtt > internals_;
    };

} // namespace sc

#endif //SCHLAZICONTROL_MQTT_MQTT_HPP
