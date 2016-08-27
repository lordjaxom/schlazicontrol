#ifndef SCHLAZICONTROL_MANAGER_HPP
#define SCHLAZICONTROL_MANAGER_HPP

#include <chrono>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>

#include <asio.hpp>

#include "component.hpp"
#include "events.hpp"
#include "properties.hpp"

namespace sc {

	class CmdLine;

	class Manager
	{
		using ReadyEvent = Event< void () >;
		using PollEvent = Event< void ( std::chrono::microseconds ) >;

	public:
		Manager( CmdLine const& cmdLine );
		Manager( Manager const& ) = delete;

		asio::io_service& service() { return service_; }

		template< typename Type >
		Type& get( std::string const& name ) const
		{
			auto it = components_.find( name );
			if ( it == components_.end() ) {
				throw std::runtime_error( str( "dependent component ", name, " not found" ) );
			}
			Type* result = dynamic_cast< Type* >( it->second.get() );
			if ( result == nullptr ) {
				throw std::runtime_error( str( "dependent component ", name, " not of required type" ) );
			}
			return *result;
		}

		void subscribeReadyEvent( ReadyEvent::Handler handler );
		void subscribePollEvent( PollEvent::Handler handler );

		void run();

	private:
		void startPolling( std::chrono::microseconds interval );

		Properties properties_;
		asio::io_service service_;
		asio::signal_set signals_;
		asio::steady_timer pollingTimer_;
        std::map< std::string, std::unique_ptr< Component > > components_;
		ReadyEvent readyEvent_;
		PollEvent pollEvent_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_MANAGER_HPP
