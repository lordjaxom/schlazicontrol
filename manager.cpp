#include <csignal>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <system_error>
#include <thread>
#include <vector>

#include <signal.h>

#include <asio.hpp>

#include "core/config.hpp"
#include "core/component.hpp"
#include "core/commandline.hpp"
#include "core/logging.hpp"
#include "manager.hpp"
#include "statistics.hpp"
#include "types.hpp"

#if SCHLAZICONTROL_FORK
#   include <sys/types.h>
#   include <sys/wait.h>
#   include <unistd.h>
#endif

using namespace std;
using namespace asio;

namespace sc {

    static Logger logger( "manager" );

#if SCHLAZICONTROL_FORK
	static bool processExited( pid_t pid )
    {
	    int status;
	    auto result = ::waitpid( pid, &status, WNOHANG );
	    return result == pid || ( result == -1 && errno == ECHILD );
    }

    static void killGracefully( pid_t pid, size_t timeoutMs = 1000 )
    {
        if ( processExited( pid ) ) {
            return;
        }

        logger.debug( "terminating process ", pid );

        ::kill( pid, SIGTERM );
        while ( timeoutMs > 0 ) {
            int status;
            if ( ::waitpid( pid, &status, WNOHANG ) == pid ) {
                return;
            }
            this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
            timeoutMs = timeoutMs > 100 ? timeoutMs - 100 : 0;
        }

        logger.debug( "termination failed, killing process ", pid );

        ::kill( pid, SIGKILL );
    }
#endif

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

        asio::io_context service;
        asio::signal_set signals;
        asio::steady_timer pollingTimer;
        asio::steady_timer statisticsTimer;
#if SCHLAZICONTROL_FORK
        vector< pid_t > processes;
#endif
    };

    /**
     * class ManagerProcess
     */

    ManagerProcess::ManagerProcess() = default;

#if SCHLAZICONTROL_FORK
    ManagerProcess::ManagerProcess( Component const& component, std::function< bool () >&& handler )
            : name_( component.name() )
            , id_( component.id() )
            , handler_( move( handler ) )
    {
    }

    ManagerProcess::operator bool() const
    {
        return (bool) handler_;
    }

    void ManagerProcess::operator()() const
    {
        handler_();
    }
#endif

    /**
     * class Manager
     */

    static PropertyKey const updateIntervalProperty( "updateInterval", "40ms" );
    static PropertyKey const statisticsIntervalProperty( "statisticsInterval", "0s" );
    static PropertyKey const componentsProperty( "components" );
    static PropertyKey const componentTypeProperty( "type" );
    static PropertyKey const componentIdProperty( "id" );
    static PropertyKey const componentDisabledProperty( "disabled", false );

    Manager::Manager( CommandLine const& cmdLine )
		: properties_( cmdLine.propertiesFile() )
		, updateInterval_( properties_[ updateIntervalProperty ].as< std::chrono::nanoseconds >() )
        , statisticsInterval_( properties_[ statisticsIntervalProperty ].as< std::chrono::nanoseconds >() )
        , internals_( new ManagerInternals )
    {
        for ( auto componentNode : properties_[ componentsProperty ] ) {
            createComponent( componentNode );
        }

		internals_->signals.async_wait( [this] ( error_code ec, int ) {
			logger.info( "received signal, shutting down" );
			stop();
		} );
	}

    Manager::~Manager() = default;

    asio::io_context& Manager::service()
    {
        return internals_->service;
    }

    ManagerProcess Manager::forkProcesses()
    {
#if SCHLAZICONTROL_FORK
        for ( auto const& entry : components_ ) {
            if ( auto handler = entry.second->forkedProcess() ) {
                internals_->service.notify_fork( io_service::fork_prepare );
                auto pid = ::fork();
                if ( pid == -1 ) {
                    throw system_error(
                            errno, std::system_category(), str( "couldn't start process for component ", entry.first ) );
                }
                if ( pid == 0 ) {
                    internals_->service.notify_fork( io_service::fork_child );
                    return { *entry.second, move( handler ) };
                }
                internals_->service.notify_fork( io_service::fork_parent );
                internals_->processes.push_back( pid );
            }
        }
#endif
        return {};
    }

    void Manager::run()
    {
        readyEvent_();
        startPolling();
        startStatistics();
        internals_->service.run();
    }

	void Manager::stop()
    {
#if SCHLAZICONTROL_FORK
        for_each( internals_->processes.begin(), internals_->processes.end(),
                  []( auto pid ) { killGracefully( pid ); } );
#endif

        internals_->service.stop();
        components_.clear();
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
            if ( ec == make_error_code( asio::error::operation_aborted ) ) {
                return;
            }

			checkProcesses();
            pollEvent_( updateInterval_ );
            startPolling();
        } );
    }

    void Manager::startStatistics()
    {
        if ( statisticsInterval_ == std::chrono::nanoseconds::zero() ) {
            return;
        }

        internals_->statisticsTimer.expires_from_now( statisticsInterval_ );
        internals_->statisticsTimer.async_wait( [this]( error_code ec ) {
            if ( ec == make_error_code( asio::error::operation_aborted ) ) {
                return;
            }

            logger.info( makeStatistics( components_ ), makeStatistics( ChannelBuffer::tracker() ) );

            startStatistics();
        } );
    }

    void Manager::checkProcesses()
    {
        auto it = find_if( internals_->processes.cbegin(), internals_->processes.cend(),
                           []( auto pid ) { return processExited( pid ); } );
        if ( it != internals_->processes.cend() ) {
            logger.error( "process ", *it, " died unexpectedly, shutting down" );
            stop();
        }
    }

} // namespace sc
