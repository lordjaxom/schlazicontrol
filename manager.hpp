#ifndef SCHLAZICONTROL_MANAGER_HPP
#define SCHLAZICONTROL_MANAGER_HPP

#include <chrono>
#include <unordered_map>
#include <memory>
#include <string>
#include <utility>

#include "event.hpp"
#include "properties.hpp"

namespace asio {
    class io_service;
} // namespace asio

namespace sc {

	class CommandLine;
    class Component;

    /**
     * class Manager
     */

    struct ManagerInternals;

	class Manager
	{
        using ReadyEvent = Event< void () >;
        using PollEvent = Event< void ( std::chrono::nanoseconds ) >;

    public:
        Manager( CommandLine const& commandLine );
		Manager( Manager const& ) = delete;
        ~Manager();

        std::chrono::nanoseconds updateInterval() const { return updateInterval_; }

		asio::io_service& service();

		template< typename Type >
		Type& get( Component const& requester, std::string const& id ) const
		{
            auto& component = findComponent( requester, id );
			Type* result = dynamic_cast< Type* >( &component );
            checkValidComponent( requester, component, result );
			return *result;
		}

		template< typename Type >
        Type& get( Component const& requester, PropertyNode const& node )
        {
            auto& component = node.is< std::string >()
                    ? findComponent( requester, node.as< std::string >() )
                    : createComponent( node, true );
            Type* result = dynamic_cast< Type* >( &component );
            checkValidComponent( requester, component, result );
            return *result;
        }

        ReadyEvent::Interface& readyEvent() { return readyEvent_.interface(); }
        PollEvent::Interface& pollEvent() { return pollEvent_.interface(); }

		void run();

	private:
        Component& createComponent( PropertyNode const& properties, bool adhoc );
        Component& findComponent( Component const& requester, std::string const& id ) const;
        void checkValidComponent( Component const& requester, Component const& component, void const* cast ) const;

		void startPolling();
        void startStatistics();

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
