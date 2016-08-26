#include <csignal>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include "component.hpp"
#include "cmdline.hpp"
#include "events.hpp"
#include "logging.hpp"
#include "manager.hpp"
#include "properties.hpp"

using namespace std;
using namespace asio;
using namespace boost::algorithm;

namespace sc {

	static Logger logger( "manager" );

	static map< string, unique_ptr< Component > > parseComponents( Manager& manager, PropertyNode const& components )
    {
        map< string, unique_ptr< Component > > result;
        for ( auto const& component : components ) {
            auto name = component[ "type" ].as< string >();
            auto id = component[ "id" ].as< string >();
            unique_ptr< Component > ptr( ComponentFactory::instance->create( manager, name, id, component ) );
            auto it = result.emplace( id, move( ptr ) );
            if ( !it.second ) {
                throw runtime_error( str( "duplicate component id ", id ) );
            }

            logger.info( "added component ", name, " with id ", id );
        }
        return result;
    }

    static PropertyKey const pollingIntervalProperty( "pollingInterval", "40" );
    static PropertyKey const componentsProperty( "components" );

    Manager::Manager( CmdLine const& cmdLine )
		: properties_( cmdLine.propertiesFile() )
		, signals_( service_, SIGINT, SIGTERM )
		, pollingTimer_( service_ )
        , components_( parseComponents( *this, properties_[ componentsProperty ] ) )
	{
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

    EventConnection Manager::subscribeReadyEvent( Manager::ReadyEvent::slot_function_type&& handler )
    {
        return readyEvent_.connect( std::move( handler ) );
    }

    EventConnection Manager::subscribePollEvent( Manager::PollEvent::slot_function_type&& handler )
    {
        return pollEvent_.connect( std::move( handler ) );
    }

    void Manager::run()
	{
        readyEvent_();
		startPolling( chrono::milliseconds(
				properties_[ pollingIntervalProperty ].as< chrono::milliseconds::rep >() ) );
		service_.run();
	}

} // namespace sc
