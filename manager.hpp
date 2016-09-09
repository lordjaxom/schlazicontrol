#ifndef SCHLAZICONTROL_MANAGER_HPP
#define SCHLAZICONTROL_MANAGER_HPP

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include <boost/signals2/signal.hpp>

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
	public:
        using ReadyEvent = boost::signals2::signal< void () >;
        using PollEvent = boost::signals2::signal< void ( std::chrono::nanoseconds ) >;

        Manager( CommandLine const& commandLine );
		Manager( Manager const& ) = delete;
        ~Manager();

		asio::io_service& service();

		template< typename Type >
		Type& get( std::string const& requester, std::string const& name ) const
		{
			Type* result = dynamic_cast< Type* >( findComponent( requester, name ));
            checkValidComponent( requester, name, result );
			return *result;
		}

        void subscribeReadyEvent( ReadyEvent::slot_type const& handler );
        void subscribeReadyEventEx( ReadyEvent::extended_slot_type const& handler );
        void subscribePollEvent( PollEvent::slot_type const& handler );
        void subscribePollEventEx( PollEvent::extended_slot_type const& handler );

		void run();

	private:
        Component* findComponent( std::string const& requester, std::string const& name ) const;
        void checkValidComponent( std::string const& requester, std::string const& name, void* component ) const;

		void startPolling( std::chrono::nanoseconds interval );

		Properties properties_;
        std::unique_ptr< ManagerInternals > internals_;
        std::map< std::string, std::unique_ptr< Component > > components_;
		ReadyEvent readyEvent_;
		PollEvent pollEvent_;

	};

} // namespace sc

#endif // SCHLAZICONTROL_MANAGER_HPP
