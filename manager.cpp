#include <csignal>
#include <stdexcept>
#include <system_error>

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

    static PropertyKey const updateIntervalProperty( "updateInterval", 40 );
    static PropertyKey const componentsProperty( "components" );

    Manager::Manager( CommandLine const& cmdLine )
		: properties_( cmdLine.propertiesFile() )
		, updateInterval_(
					chrono::milliseconds( properties_[ updateIntervalProperty ].as< chrono::milliseconds::rep >() ) )
        , internals_( new ManagerInternals )
	{
        for ( auto componentNode : properties_[ componentsProperty ] ) {
            createComponent( componentNode, false );
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

    void Manager::run()
	{
		readyEvent_();
		startPolling();
        internals_->service.run();
	}

    Component& Manager::createComponent( PropertyNode const& properties, bool adhoc )
    {
        auto type = properties[ "type" ].as< string >();
        auto id = !adhoc || properties.has( "id" )
                  ? properties[ "id" ].as< string >()
				  : ComponentFactory::instance().generateId( type );

        auto ptr = ComponentFactory::instance().create( type, move( id ), *this, properties );
        auto it = components_.emplace( ptr->id(), move( ptr ) );
        if ( !it.second ) {
            throw runtime_error( str( "unable to create component \"", it.first->first,
                                      "\": another component with the same id exists" ) );
        }
        logger.info( "component \"", it.first->first, "\" of type \"", type, "\" created" );
        return *it.first->second;
    }

    Component& Manager::findComponent( Component const& requester, string const& id ) const
	{
		auto it = components_.find( id );
		if ( it == components_.end() ) {
			throw runtime_error( str( "component \"", requester.id(), "\" depends on unknown component \"", id, "\"" ) );
		}
		return *it->second;
	}

    void Manager::checkValidComponent( Component const& requester, Component const& component, void const* cast ) const
	{
        if ( cast == nullptr ) {
            throw runtime_error( str( "component \"", requester.id(), "\" depends on component \"", component.id(),
                                      "\", but \"", component.id(), "\" is not of the required type" ) );
        }
	}

	void Manager::startPolling()
	{
        internals_->pollingTimer.expires_from_now( updateInterval_ );
        internals_->pollingTimer.async_wait( [this]( error_code ec ) {
			if ( ec.value() == (int) errc::operation_canceled ) {
				return;
			}

            pollEvent_( updateInterval_ );
			startPolling();
		} );
	}

} // namespace sc
