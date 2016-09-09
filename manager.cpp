#include <csignal>
#include <stdexcept>

#include <asio.hpp>

#include "component.hpp"
#include "commandline.hpp"
#include "logging.hpp"
#include "manager.hpp"

using namespace std;
using namespace asio;

namespace sc {

	static Logger logger( "manager" );

    /**
     * struct ManagerInternals
     */

    struct ManagerInternals
    {
        ManagerInternals()
                : signals( service, SIGINT, SIGTERM )
                , pollingTimer( service )
        {
        }

        io_service service;
        signal_set signals;
        steady_timer pollingTimer;
    };

    /**
     * class Manager
     */

    static PropertyKey const pollingIntervalProperty( "pollingInterval", 40 );
    static PropertyKey const componentsProperty( "components" );

    Manager::Manager( CommandLine const& cmdLine )
		: properties_( cmdLine.propertiesFile() )
        , internals_( new ManagerInternals )
	{
        for ( auto component : properties_[ componentsProperty ] ) {
            auto name = component[ "type" ].as< string >();
            auto ptr = ComponentFactory::instance().create(
                    *this, name, component[ "id" ].as< string >(), component );
            auto it = components_.emplace( ptr->id(), move( ptr ) );
            if ( !it.second ) {
                throw runtime_error( str(
                        "unable to create component \"", it.first->first,
                        "\": another component with the same id exists" ) );
            }
            logger.info( "component \"", it.first->first, "\" of type \"", name, "\" created" );
        }

		internals_->signals.async_wait( [this] ( error_code ec, int ) {
			logger.info( "received signal, shutting down" );
            internals_->service.stop();
		} );
	}

    Manager::~Manager() = default;

    io_service& Manager::service()
    {
        return internals_->service;
    }

	EventConnection Manager::subscribeReadyEvent( Manager::ReadyEvent::Handler handler )
	{
		return readyEvent_.subscribe( move( handler ) );
	}

	EventConnection Manager::subscribePollEvent( Manager::PollEvent::Handler handler )
	{
		return pollEvent_.subscribe( move( handler ) );
	}

	void Manager::run()
	{
		logger.info( "running..." );
		readyEvent_();
		startPolling( chrono::milliseconds(
				properties_[ pollingIntervalProperty ].as< chrono::milliseconds::rep >() ) );
        internals_->service.run();
	}

    Component* Manager::findComponent( string const& requester, string const& name ) const
	{
		auto it = components_.find( name );
		if ( it == components_.end() ) {
			throw runtime_error( str( "component \"", name, "\" (requested by \"", requester, "\" does not exist" ) );
		}
		return it->second.get();
	}

    void Manager::checkValidComponent( string const& requester, string const& name, void* component ) const
	{
        if ( component == nullptr ) {
            throw std::runtime_error( str( "dependent component ", name, " not of required type" ) );
        }
	}

	void Manager::startPolling( std::chrono::nanoseconds const& interval )
	{
        internals_->pollingTimer.expires_from_now( interval );
        internals_->pollingTimer.async_wait( [this, interval]( error_code ec ) {
			if ( ec.value() == (int) errc::operation_canceled ) {
				return;
			}

            pollEvent_();
			startPolling( interval );
		} );
	}

} // namespace sc
