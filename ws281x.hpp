#ifndef SCHLAZICONTROL_WS281X_HPP
#define SCHLAZICONTROL_WS281X_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include <asio.hpp>

#include "component.hpp"

namespace sc {

	class ChannelBuffer;
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

	class Ws281x
		: public Component
	{
	public:
		Ws281x( Manager& manager, std::string id, PropertyNode const& properties );

		std::size_t channelCount() const { return ledCount_ * 3; }

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
		asio::ip::tcp::socket socket_;
		asio::streambuf incoming_;
		asio::streambuf outgoing_;
	};

	class Ws281xDevice
	{
	public:
		Ws281xDevice( Manager& manager, std::string const& ws281xId );

		std::size_t channelCount() const { return ws281x_.channelCount(); }

		void send( ChannelBuffer const& values ) { ws281x_.send( values ); }

	private:
		Ws281x& ws281x_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_WS281X_HPP
