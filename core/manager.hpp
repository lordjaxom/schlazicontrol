#ifndef SCHLAZICONTROL_MANAGER_HPP
#define SCHLAZICONTROL_MANAGER_HPP

#include <chrono>
#include <functional>
#include <unordered_map>
#include <memory>
#include <string>
#include <utility>

#include <asio/io_context.hpp>

#include "core/config.hpp"
#include "event.hpp"
#include "core/properties.hpp"

namespace sc {

	class CommandLine;
    class Component;

    /**
     * class ManagerProcess
     */

    class ManagerProcess final
    {
    public:
        ManagerProcess();
#if SCHLAZICONTROL_FORK
        ManagerProcess( Component const& component, std::function< bool () >&& handler );
#endif

        explicit operator bool() const SCHLAZICONTROL_UNLESS( FORK, { return false; } );

        std::string const& name() const { return name_; }
        std::string const& id() const { return id_; }

        void operator()() const SCHLAZICONTROL_UNLESS( FORK, {} );

    private:
        std::string name_;
        std::string id_;
#if SCHLAZICONTROL_FORK
        std::function< bool () > handler_;
#endif
    };

    /**
     * class Manager
     */

    struct ManagerInternals;

	class Manager final
	{
        using ReadyEvent = Event< void () >;
        using PollEvent = Event< void ( std::chrono::nanoseconds ) >;

    public:
        explicit Manager( CommandLine const& commandLine );
		Manager( Manager const& ) = delete;
        ~Manager();

        std::chrono::nanoseconds updateInterval() const { return updateInterval_; }

		asio::io_context& service();

		template< typename Type >
        Type& get( Component const& requester, PropertyNode const& node )
        {
            auto& component = node.is< std::string >()
                    ? findComponent( requester, node.as< std::string >() )
                    : *createComponent( node, &requester );
            Type* result = dynamic_cast< Type* >( &component );
            checkValidComponent( requester, component, result );
            return *result;
        }

        ReadyEvent::Interface& readyEvent() { return readyEvent_.interface(); }
        PollEvent::Interface& pollEvent() { return pollEvent_.interface(); }

        ManagerProcess forkProcesses();

		void run();

	private:
        Component* createComponent( PropertyNode const& properties, Component const* requester = nullptr );
        Component& findComponent( Component const& requester, std::string const& id ) const;
        void checkValidComponent( Component const& requester, Component const& component, void const* cast ) const;

		void startPolling();
        void startStatistics();
        void checkProcesses();

        void stop();

		Properties properties_;
		std::chrono::nanoseconds updateInterval_;
        std::chrono::nanoseconds statisticsInterval_;
        std::unique_ptr< ManagerInternals > internals_;
        std::unordered_map< std::string, std::unique_ptr< Component > > components_;
		ReadyEvent readyEvent_;
		PollEvent pollEvent_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_MANAGER_HPP
