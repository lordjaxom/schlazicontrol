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
		Ws281x( Manager& manager, std::string id, PropertyNode const& properties );
        ~Ws281x();

		std::size_t ledCount() const { return ledCount_; }

		void send( std::size_t start, ChannelBuffer const& values );

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
        ChannelBuffer values_;
	};

	class Ws281xDevice
	{
	public:
        static constexpr size_t maxCount = std::numeric_limits< std::size_t >::max();

        Ws281xDevice(
                Manager& manager, std::string const& requester, std::string const& ws281xId,
                std::size_t start, std::size_t count );

		std::size_t channelCount() const { return count_ * 3; }

		void send( ChannelBuffer const& values );

	private:
		Ws281x& ws281x_;
		std::size_t start_;
        std::size_t count_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_WS281X_HPP
