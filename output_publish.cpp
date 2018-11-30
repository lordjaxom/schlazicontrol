#include <utility>

#include "manager.hpp"
#include "properties.hpp"
#include "mqtt.hpp"
#include "output_publish.hpp"
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
        mqtt_.publish( topic_, to_string( values[ 0 ].get() ) );
    }

    void MqttPublish::doStatistics( std::ostream& os ) const
    {

    }


    static OutputRegistry< MqttPublish > registry( "mqtt" );

} // namespace sc