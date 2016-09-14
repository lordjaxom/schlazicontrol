#ifndef SCHLAZICONTROL_MANAGER_HPP
#define SCHLAZICONTROL_MANAGER_HPP

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "event.hpp"
#include "properties.hpp"

namespace asio {
    class io_service;
} // namespace asio

namespace sc {

	class CommandLine;
    class Component;

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
		Type& get( std::string const& requester, std::string const& name ) const
		{
            auto component = findComponent( requester, name );
			Type* result = dynamic_cast< Type* >( std::get< 1 >( component ) );
            checkValidComponent( requester, std::get< 0 >( component ), result );
			return *result;
		}

		template< typename Type >
        Type& get( std::string const& requester, PropertyNode const& node )
        {
            auto component = node.is< std::string >()
                    ? findComponent( requester, node.as< std::string >() )
                    : createComponent( node, true );
            Type* result = dynamic_cast< Type* >( std::get< 1 >( component ) );
            checkValidComponent( requester, std::get< 0 >( component ), result );
            return *result;
        }

        ReadyEvent::Interface& readyEvent() { return readyEvent_.interface(); }
        PollEvent::Interface& pollEvent() { return pollEvent_.interface(); }

		void run();

	private:
        std::tuple< std::string, Component* > createComponent( PropertyNode const& properties, bool adhoc );
        std::tuple< std::string, Component* > findComponent( std::string const& requester, std::string const& name ) const;
        void checkValidComponent( std::string const& requester, std::string const& name, void* component ) const;

		void startPolling();

		Properties properties_;
		std::chrono::nanoseconds updateInterval_;
        std::unique_ptr< ManagerInternals > internals_;
        std::map< std::string, std::unique_ptr< Component > > components_;
		ReadyEvent readyEvent_;
		PollEvent pollEvent_;

	};

} // namespace sc

#endif // SCHLAZICONTROL_MANAGER_HPP
