#include <csignal>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <system_error>

#include <asio.hpp>

#include "component.hpp"
#include "commandline.hpp"
#include "logging.hpp"
#include "manager.hpp"
#include "statistics.hpp"
#include "types.hpp"

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
                , statisticsTimer( service )
        {
        }

        io_service service;
        signal_set signals;
        steady_timer pollingTimer;
        steady_timer statisticsTimer;
    };

    /**
     * class Manager
     */

    static PropertyKey const updateIntervalProperty( "updateInterval", 40 );
    static PropertyKey const statisticsIntervalProperty( "statisticsInterval", "0s" );
    static PropertyKey const componentsProperty( "components" );
    static PropertyKey const componentTypeProperty( "type" );
    static PropertyKey const componentIdProperty( "id" );
    static PropertyKey const componentDisabledProperty( "disabled", false );

    Manager::Manager( CommandLine const& cmdLine )
		: properties_( cmdLine.propertiesFile() )
		, updateInterval_( properties_[ updateIntervalProperty ].as< chrono::nanoseconds >() )
        , statisticsInterval_( properties_[ statisticsIntervalProperty ].as< chrono::nanoseconds >() )
        , internals_( new ManagerInternals )
	{
        for ( auto componentNode : properties_[ componentsProperty ] ) {
            createComponent( componentNode );
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
        startStatistics();
        internals_->service.run();
	}

    Component* Manager::createComponent( PropertyNode const& properties, Component const* requester )
    {
        auto& factory = ComponentFactory::instance();
        auto type = properties[ componentTypeProperty ].as< string >();
        auto disabled = properties[ componentDisabledProperty ].as< bool >();
        auto id = requester == nullptr || properties.has( "id" )
                  ? properties[ "id" ].as< string >() : factory.generateId( type );
        if ( disabled ) {
            if ( requester != nullptr ) {
                throw runtime_error( str( "cannot disable component ", Component::describe( type, id, requester ),
                                          ": only top-level component may be disabled" ) );
            }
            return nullptr;
        }

        auto ptr = ComponentFactory::instance().create( type, move( id ), *this, properties );
        auto it = components_.emplace( ptr->id(), move( ptr ) );
        if ( !it.second ) {
            throw runtime_error( str( "unable to create component ",
                                      Component::describe( type, it.first->first, requester ), ": another component ",
                                      it.first->second->describe(), " already exists" ) );
        }
        logger.info( "component ", it.first->second->describe( requester ), " created" );
        return it.first->second.get();
    }

    Component& Manager::findComponent( Component const& requester, string const& id ) const
	{
		auto it = components_.find( id );
		if ( it == components_.end() ) {
			throw runtime_error( str( "component ", requester.describe(), " depends on unknown component \"", id,
                                      "\"" ) );
		}
		return *it->second;
	}

    void Manager::checkValidComponent( Component const& requester, Component const& component, void const* cast ) const
	{
        if ( cast == nullptr ) {
            throw runtime_error( str( "component ", requester.describe(), " depends on component \"", component.id(),
                                      "\" which is not of the required type" ) );
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

    void Manager::startStatistics()
    {
        if ( statisticsInterval_ == chrono::nanoseconds::zero() ) {
            return;
        }

        internals_->statisticsTimer.expires_from_now( statisticsInterval_ );
        internals_->statisticsTimer.async_wait( [this]( error_code ec ) {
            if ( ec.value() == (int) errc::operation_canceled ) {
                return;
            }

            logger.info( makeStatistics( components_ ), makeStatistics( ChannelBuffer::tracker() ) );

            startStatistics();
        } );
    }

} // namespace sc
