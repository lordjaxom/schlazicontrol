#ifndef SCHLAZICONTROL_WS281X_HPP
#define SCHLAZICONTROL_WS281X_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <system_error>

#include "component.hpp"
#include "types.hpp"

namespace sc {

	class Manager;
	class PropertyNode;
	class Ws281xClient;
	class Ws281xLauncher;
	class Ws281xServer;

	class Ws281xLaunchException
	{
	public:
		Ws281xLaunchException( std::uint16_t gpioPin, std::size_t ledCount );

		void run();

	private:
		std::uint16_t gpioPin_;
		std::size_t ledCount_;
	};

    struct Ws281xInternals;

    class Ws281x
		: public Component
	{
	public:
		Ws281x( std::string&& id, Manager& manager, PropertyNode const& properties );
        ~Ws281x();

        virtual void statistics( std::ostream& os ) const override;

		std::size_t ledCount() const { return ledCount_; }

		void send( ChannelBuffer const& values );

	private:
		void connect();
		void reconnect();
		void retryConnect();

		void receiveInitMessage();

		bool handleError( std::error_code const& ec );

		Manager& manager_;
		std::uint16_t gpioPin_;
		std::size_t ledCount_;
        std::unique_ptr< Ws281xLauncher > launcher_;
        std::unique_ptr< Ws281xInternals > internals_;
	};

	class Ws281xDevice
	{
	public:
        static constexpr size_t maxCount = std::numeric_limits< std::size_t >::max();

        Ws281xDevice( Ws281x& ws281x );

		std::size_t channelCount() const { return ws281x_.ledCount() * 3; }

		void send( ChannelBuffer const& values );

	private:
		Ws281x& ws281x_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_WS281X_HPP
