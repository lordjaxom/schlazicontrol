#include <csignal>

#include "component.hpp"
#include "cmdline.hpp"
#include "logging.hpp"
#include "manager.hpp"

using namespace std;
using namespace asio;

namespace sc {

	static Logger logger( "manager" );

    static PropertyKey const pollingIntervalProperty( "pollingInterval", 40 );
    static PropertyKey const componentsProperty( "components" );

    Manager::Manager( CmdLine const& cmdLine )
		: properties_( cmdLine.propertiesFile() )
		, signals_( service_, SIGINT, SIGTERM )
		, pollingTimer_( service_ )
	{
        for ( auto component : properties_[ componentsProperty ] ) {
            auto name = component[ "type" ].as< string >();
            auto ptr = ComponentFactory::instance->create(
                    *this, name, component[ "id" ].as< string >(), component );
            auto it = components_.emplace( ptr->id(), move( ptr ) );
            if ( !it.second ) {
                throw runtime_error( str(
                        "unable to create component \"", it.first->first,
                        "\": another component with the same id exists" ) );
            }
            logger.info( "component \"", it.first->first, "\" of type \"", name, "\" created" );
        }

		signals_.async_wait( [this] ( error_code ec, int ) {
			logger.info( "received signal, shutting down" );
			service_.stop();
		} );
	}

	void Manager::startPolling( std::chrono::microseconds interval )
	{
		pollingTimer_.expires_from_now( interval );
		pollingTimer_.async_wait( [this, interval]( error_code ec ) {
			if ( ec == errc::operation_canceled ) {
				return;
			}

            pollEvent_( chrono::duration_cast< chrono::microseconds >( interval ) );
			startPolling( interval );
		} );
	}

    void Manager::subscribeReadyEvent( Manager::ReadyEvent::Handler handler )
    {
        readyEvent_.subscribe( move( handler ) );
    }

    void Manager::subscribePollEvent( Manager::PollEvent::Handler handler )
    {
        pollEvent_.subscribe( move( handler ) );
    }

    void Manager::run()
	{
		logger.info( "running..." );
        readyEvent_();
		startPolling( chrono::milliseconds(
				properties_[ pollingIntervalProperty ].as< chrono::milliseconds::rep >() ) );
		service_.run();
	}

} // namespace sc
