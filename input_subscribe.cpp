#include <algorithm>
#include <iterator>

#include "logging.hpp"
#include "manager.hpp"
#include "properties.hpp"
#include "mqtt.hpp"
#include "input_subscribe.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

    static Logger logger( "input_subscribe" );

    static PropertyKey const mqttProperty( "mqtt" );
    static PropertyKey const topicProperty( "topic" );
    static PropertyKey const onProperty( "on" );
    static PropertyKey const offProperty( "off" );

    MqttSubscribe::MqttSubscribe( string&& id, Manager& manager, PropertyNode const& properties )
            : Component( move( id ) )
    {
        auto const& on = properties[ onProperty ].as< string[] >();
        transform( on.cbegin(), on.cend(), inserter( commands_, commands_.end() ), []( auto const& command ) {
            return make_pair( command, 100 );
        } );

        auto const& off = properties[ offProperty ].as< string[] >();
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

