#ifndef SCHLAZICONTROL_VDCD_HPP
#define SCHLAZICONTROL_VDCD_HPP

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <asio.hpp>

#include "component.hpp"

namespace Json {
	class StreamWriter;
	class Value;
} // namespace Json

namespace sc {

	class Manager;
	class PropertyNode;
	class VdcdDevice;

	class Vdcd
		: public Component
	{
	public:
		Vdcd( Manager& manager, std::string id, PropertyNode const& properties );

		void add( VdcdDevice* device );
		void remove( VdcdDevice* device );

	private:
		void connect();
		void reconnect();
		void retryConnect();

		void sendInitMessage();
		void receiveInitResponse();
		void sendChannelMessage( std::size_t device );
		void sendSyncedMessage( std::size_t device );

		void send( Json::Value const& json, std::function< void () > const& handler );
		void receive( std::function< void ( Json::Value const& ) > const& handler, bool repeat = true );

		void parse( Json::Value const& json );

		bool handleError( std::error_code const& ec );

		Manager& manager_;
		std::string host_;
		std::string port_;
		asio::ip::tcp::socket socket_;
		asio::streambuf outgoing_;
		asio::streambuf incoming_;
		std::unique_ptr< Json::StreamWriter > jsonWriter_;
		std::vector< VdcdDevice* > devices_;
	};

	class VdcdDevice
	{
		using SetterFunction = std::function< void ( double ) >;

	public:
		VdcdDevice(
				Manager& manager, std::string const& requester, std::string const& vdcdId, std::string const& dsuid, int group,
				std::string const& outputType, bool dimmable, SetterFunction const& setterFunction = SetterFunction() );
		VdcdDevice( VdcdDevice const& ) = delete;
		~VdcdDevice();

		double get() const { return value_; }
		void set( double value );

		std::string const& name() const { return name_; }
		std::string const& dsuid() const { return dsuid_; }
		int group() const { return group_; }
		std::string const& outputType() const { return outputType_; }
		bool dimmable() const { return dimmable_; }

	private:
		Vdcd& vdcd_;
		std::string name_;
		std::string dsuid_;
		int group_;
		std::string outputType_;
		bool dimmable_;
		double value_;
		SetterFunction setterFunction_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_VDCD_HPP
