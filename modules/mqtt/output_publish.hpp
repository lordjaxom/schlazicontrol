#ifndef SCHLAZICONTROL_MQTT_OUTPUT_PUBLISH_HPP
#define SCHLAZICONTROL_MQTT_OUTPUT_PUBLISH_HPP

#include "core/output.hpp"

namespace sc {

    class Mqtt;

    class MqttPublish final
            : public Output
    {
    public:
        MqttPublish( std::string&& id, Manager& manager, PropertyNode const& properties );
        ~MqttPublish() override;

        bool acceptsChannels( std::size_t channels ) const override { return channels == 1; }

    protected:
        void set( Input const& input, ChannelBuffer const& values ) override;

        void doStatistics( std::ostream& os ) const override;

    private:
        Mqtt& mqtt_;
        std::string topic_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_MQTT_OUTPUT_PUBLISH_HPP
