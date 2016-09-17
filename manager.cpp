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
		logger.info( "running..." );
		readyEvent_();
		startPolling();
        internals_->service.run();
	}

    tuple< string, Component* > Manager::createComponent( PropertyNode const& properties, bool adhoc )
    {
        auto type = properties[ "type" ].as< string >();
        auto id = !adhoc || properties.has( "id" )
                  ? properties[ "id" ].as< string >()
				  : ComponentFactory::instance().generateId( type );
        auto ptr = ComponentFactory::instance().create( type, move( id ), *this, properties );
        auto it = components_.emplace( ptr->id(), move( ptr ) );
        if ( !it.second ) {
            throw runtime_error( str(
                    "unable to create component \"", it.first->first,
                    "\": another component with the same id exists" ) );
        }
        logger.info( "component \"", it.first->first, "\" of type \"", type, "\" created" );
        return make_tuple( type, it.first->second.get() );
    }

    tuple< string, Component* > Manager::findComponent( string const& requester, string const& name ) const
	{
		auto it = components_.find( name );
		if ( it == components_.end() ) {
			throw runtime_error( str( "component \"", name, "\" (requested by \"", requester, "\" does not exist" ) );
		}
		return make_tuple( name, it->second.get() );
	}

    void Manager::checkValidComponent( string const& requester, string const& name, void* component ) const
	{
        if ( component == nullptr ) {
            throw std::runtime_error( str( "dependent component ", name, " not of required type" ) );
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
