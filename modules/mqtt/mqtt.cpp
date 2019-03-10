#include <cstdint>
#include <algorithm>
#include <list>
#include <thread>
#include <unordered_map>
#include <utility>

#include <asio.hpp>
#include <mosquitto.h>

#include "core/logging.hpp"
#include "core/manager.hpp"
#include "core/properties.hpp"
#include "modules/mqtt/mqtt.hpp"

using namespace std;
using namespace asio;

namespace sc {

    static Logger logger( "mqtt" );

    static PropertyKey const hostProperty( "host" );
    static PropertyKey const portProperty( "port", 1883 );
    static PropertyKey const clientIdProperty( "clientId", "" );
    static PropertyKey const willTopicProperty( "willTopic", "" );

    namespace detail {

        struct MqttInfo final
        {
            string host;
            int port;
            string clientId;

            friend ostream& operator<<( ostream& os, MqttInfo const& info )
            {
                return os << "MQTT{" << info.host << ":" << info.port << "/"
                          << ( !info.clientId.empty() ? info.clientId : "<anonymous>" )
                          << "}: ";
            }
        };

        class Mqtt final
        {
            using MessageHandler = sc::Mqtt::MessageHandler;

            struct Publication
            {
                string topic;
                string payload;
                bool retain;
            };

        public:
            Mqtt( Manager& manager, PropertyNode const& properties )
                    : manager_( manager )
                    , info_ {
                        properties[ hostProperty ].as< string >(),
                        properties[ portProperty ].as< uint16_t >(),
                        properties[ clientIdProperty ].as< string >() }
                    , willTopic_( properties[ willTopicProperty ].as< string >() )
            {
                if ( !initialized ) {
                    mosquitto_lib_init();
                    initialized = true;
                }

                mosq_ = mosquitto_new( !info_.clientId.empty() ? info_.clientId.c_str() : nullptr, info_.clientId.empty(), this );
                mosquitto_connect_callback_set( mosq_, []( mosquitto*, void* obj, int rc ) { static_cast< Mqtt* >( obj )->on_connect( rc ); } );
                mosquitto_disconnect_callback_set( mosq_, []( mosquitto*, void* obj, int rc ) { static_cast< Mqtt* >( obj )->on_disconnect( rc ); } );
                mosquitto_message_callback_set( mosq_, []( mosquitto*, void* obj, mosquitto_message const* msg ) { static_cast< Mqtt* >( obj )->on_message( msg ); } );
                if ( !willTopic_.empty() ) {
                    mosquitto_will_set( mosq_, willTopic_.c_str(), 2, "NO", 1, true );
                }

                manager_.readyEvent().subscribe( [this] { connect(); }, true );
            }

            Mqtt( Mqtt const& ) = delete;

            ~Mqtt()
            {
                mosquitto_connect_callback_set( mosq_, nullptr );
                mosquitto_disconnect_callback_set( mosq_, nullptr );
                mosquitto_message_callback_set( mosq_, nullptr );

                if ( connected_ ) {
                    doPublish( { willTopic_, "NO", true } );
                }

                mosquitto_disconnect( mosq_ );
                mosquitto_loop_stop( mosq_, false );
            }

            void publish( string&& topic, string&& payload, bool retain = false )
            {
                Publication publication { move( topic ), move( payload ), retain };
                if ( connected_ ) {
                    doPublish( move( publication ) );
                } else {
                    logger.debug( info_, "registering publication to ", topic );
                    publications_.push_back( move( publication ) );
                }
            }

            void subscribe( string&& topic, MessageHandler&& handler )
            {
                logger.debug( info_, "registering subscription for ", topic );

                auto subscription = subscriptions_.emplace( move( topic ), move( handler ) );
                if ( connected_ ) {
                    doSubscribe( subscription->first );
                }
            }

            void doStatistics( ostream& os )
            {
            }

        private:
            void connect()
            {
                if ( int rc = mosquitto_loop_start( mosq_ ) ) {
                    throw runtime_error( str( info_, "couldn't start communications thread: ", mosquitto_strerror( rc ) ) );
                }

                logger.info( info_, "connecting to broker" );

                if ( int rc = mosquitto_connect_async( mosq_, info_.host.c_str(), info_.port, 60 ) ) {
                    logger.error( info_, "couldn't connect: ", mosquitto_strerror( rc ) );
                    retryConnect();
                }
            }

            void retryConnect() {
                auto retryTimeout = std::chrono::seconds( static_cast< long >( pow( 2, retries_++ ) ) );

                logger.info( info_, "retrying to connect in ", retryTimeout.count(), " seconds" );

                auto timer = make_shared< steady_timer >( manager_.service(), retryTimeout );
                timer->async_wait( [this, timer]( auto ec ) { if ( !ec ) this->connect(); } );
            }

            void doPublish( Publication&& publication )
            {
                logger.info( info_, "publishing ", publication.payload, " to ", publication.topic );

                if ( int rc = mosquitto_publish( mosq_, nullptr, publication.topic.c_str(), publication.payload.length(),
                                                 publication.payload.data(), 1, publication.retain ) ) {
                    logger.error( info_, "couldn't publish to ", publication.topic, ": ", mosquitto_strerror( rc ) );
                    // TODO: what now?
                }
            }

            void doSubscribe( string const& topic )
            {
                logger.info( info_, "subscribing to topic ", topic );

                if ( int rc = mosquitto_subscribe( mosq_, nullptr, topic.c_str(), 0 ) ) {
                    logger.error( info_, "couldn't subscribe to ", topic, ": ", mosquitto_strerror( rc ) );
                    // TODO: what now?
                }
            }

            void on_connect( int rc )
            {
                if ( rc ) {
                    logger.error( info_, "couldn't connect: ", mosquitto_strerror( rc ), ", retrying automatically" );
                    mosquitto_reconnect_async( mosq_ );
                    return;
                }

                logger.info( info_, "connection established successfully" );

                manager_.service().post( [this] {
                    connected_ = true;
                    retries_ = 0;
                    if ( !willTopic_.empty() ) {
                        this->doPublish( { willTopic_, "YES" } );
                    }
                    for_each( subscriptions_.begin(), subscriptions_.end(), [&]( auto const& subscription ) {
                        this->doSubscribe( subscription.first );
                    } );
                    for_each( publications_.begin(), publications_.end(), [&]( auto& publication ) {
                        this->doPublish( move( publication ) );
                    } );
                    publications_.clear();
                } );
            }

            void on_disconnect( int rc )
            {
                logger.error( info_, "connection lost: ", mosquitto_strerror( rc ), ", retrying automatically" );

                manager_.service().post( [this] {
                    connected_ = false;
                    mosquitto_reconnect_async( mosq_ );
                } );
            }

            void on_message( mosquitto_message const* message )
            {
                auto topic = make_shared< string >( message->topic );
                auto payload = make_shared< string >( static_cast< char const* >( message->payload ), static_cast< size_t >( message->payloadlen ) );

                logger.debug( info_, "received ", *payload, " from topic ", *topic );

                manager_.service().post( [this, topic, payload] {
                    auto subscriptions = subscriptions_.equal_range( *topic );
                    for_each( subscriptions.first, subscriptions.second, [&]( auto const& subscription ) {
                        subscription.second( *payload );
                    } );
                } );
            }

            static bool initialized;

            Manager& manager_;
            MqttInfo info_;
            string willTopic_;

            mosquitto* mosq_;
            bool connected_ {};
            bool shutdown_ {};
            size_t retries_ {};
            list< Publication > publications_;
            unordered_multimap< string, MessageHandler > subscriptions_;
        };

        bool Mqtt::initialized;

    } // namespace detail

    /**
     * class Mqtt
     */

    Mqtt::Mqtt( string&& id, Manager& manager, PropertyNode const& properties )
            : Component( move( id ) )
            , internals_( make_unique< detail::Mqtt >( manager, properties ) ) {}

    Mqtt::~Mqtt() = default;

    void Mqtt::publish( string topic, string payload )
    {
        internals_->publish( move( topic ), move( payload ) );
    }

    void Mqtt::subscribe( string topic, MessageHandler handler )
    {
        internals_->subscribe( move( topic ), move( handler ) );
    }

    void Mqtt::doStatistics( ostream& os ) const
    {
        internals_->doStatistics( os );
    }


    static ComponentRegistry< Mqtt > registry( "mqtt" );

} // namespace sc