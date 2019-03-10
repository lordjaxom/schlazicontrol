#ifndef SCHLAZICONTROL_MQTT_SUBSCRIPTION_HPP
#define SCHLAZICONTROL_MQTT_SUBSCRIPTION_HPP

#include <string>
#include <unordered_map>

#include "core/input.hpp"

namespace sc {

    class MqttSubscribe final
            : public Input
    {
    public:
        MqttSubscribe( std::string&& id, Manager& manager, PropertyNode const& properties );
        ~MqttSubscribe() override;

        std::size_t emitsChannels() const override { return 1; }

    protected:
        void doStatistics( std::ostream& os ) const override;

    private:
        std::unordered_map< std::string, int > commands_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_MQTT_SUBSCRIPTION_HPP
