#include <utility>

#include "core/manager.hpp"
#include "core/properties.hpp"
#include "modules/mqtt/mqtt.hpp"
#include "modules/mqtt/output_publish.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

    static PropertyKey const inputProperty( "input" );
    static PropertyKey const mqttProperty( "mqtt" );
    static PropertyKey const topicProperty( "topic" );

    MqttPublish::MqttPublish( string&& id, Manager& manager, PropertyNode const& properties )
            : Component( move( id ) )
            , Output( manager, properties[ inputProperty ] )
            , mqtt_( manager.get< Mqtt >( *this, properties[ mqttProperty ] ) )
            , topic_( properties[ topicProperty ].as< string >() )
    {
    }

    MqttPublish::~MqttPublish() = default;

    void MqttPublish::set( Input const& input, ChannelBuffer const& values )
    {
        mqtt_.publish( topic_, values[ 0 ].on() ? "ON" : "OFF" );
    }

    void MqttPublish::doStatistics( std::ostream& os ) const
    {
    }


    static OutputRegistry< MqttPublish > registry( "mqtt" );

} // namespace sc