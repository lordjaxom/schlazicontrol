#include <algorithm>
#include <iterator>
#include <vector>

#include "core/manager.hpp"
#include "core/properties.hpp"
#include "modules/mqtt/mqtt.hpp"
#include "modules/mqtt/input_subscribe.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

    static PropertyKey const mqttProperty( "mqtt" );
    static PropertyKey const topicProperty( "topic" );
    static PropertyKey const onProperty( "on" );
    static PropertyKey const offProperty( "off" );

    MqttSubscribe::MqttSubscribe( string&& id, Manager& manager, PropertyNode const& properties )
            : Component( move( id ) )
    {
        auto on = properties[ onProperty ].as< vector< string > >();
        transform( on.cbegin(), on.cend(), inserter( commands_, commands_.end() ), []( auto const& command ) {
            return make_pair( command, 100 );
        } );

        auto off = properties[ offProperty ].as< vector< string > >();
        transform( off.cbegin(), off.cend(), inserter( commands_, commands_.end() ), []( auto const& command ) {
            return make_pair( command, 0 );
        } );

        auto& mqtt = manager.get< Mqtt >( *this, properties[ mqttProperty ] );
        mqtt.subscribe( properties[ topicProperty ].as< string >(), [this]( auto const& payload ) {
            auto it = commands_.find( payload );
            if ( it != commands_.end() ) {
                inputChangeEvent_( ChannelBuffer { rangedPercent( it->second ) } );
            }
        } );
    }

    MqttSubscribe::~MqttSubscribe() = default;

    void MqttSubscribe::doStatistics( std::ostream& os ) const
    {
    }


    static InputRegistry< MqttSubscribe > registry( "mqtt" );

} // namespace sc

