#ifndef SCHLAZICONTROL_VDCD_HPP
#define SCHLAZICONTROL_VDCD_HPP

#include <functional>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#include "component.hpp"
#include "event.hpp"

namespace Json {
	class StreamWriter;
	class Value;
} // namespace Json

namespace sc {

	class Manager;
	class PropertyNode;
	class VdcdDevice;

    struct VdcdInternals;

	class Vdcd final
		: public Component
	{
	public:
		Vdcd( std::string&& id, Manager& manager, PropertyNode const& properties );

		void add( VdcdDevice* device );
		void remove( VdcdDevice* device );

    protected:
        virtual void doStatistics( std::ostream& os ) const override;

    private:
		void connect();
		void reconnect();
		void retryConnect();

		void sendInitMessage();
		void receiveInitResponse();
		void sendChannelMessage( std::size_t device );
		void sendSyncedMessage( std::size_t device );
		void sendPongMessage( std::size_t device );

		void send( Json::Value const& json, std::function< void () > const& handler );
		void receive( std::function< void ( Json::Value const& ) > const& handler, bool repeat = true );

		void parse( Json::Value const& json );

		bool handleError( std::error_code const& ec );

		Manager& manager_;
		std::string host_;
		std::string port_;
        std::unique_ptr< VdcdInternals > internals_;
		std::unique_ptr< Json::StreamWriter > jsonWriter_;
		std::vector< VdcdDevice* > devices_;
	};

	class VdcdDevice final
	{
	public:
		using SetEvent = Event< void ( double ) >;

		VdcdDevice( Vdcd& vdcd, std::string name, std::string dsuid, int group, std::string outputType, bool dimmable );
		VdcdDevice( VdcdDevice const& ) = delete;
		~VdcdDevice();

		double get() const { return value_; }
		void set( double value );

		std::string const& name() const { return name_; }
		std::string const& dsuid() const { return dsuid_; }
		int group() const { return group_; }
		std::string const& outputType() const { return outputType_; }
		bool dimmable() const { return dimmable_; }

        SetEvent::Interface& setEvent() { return setEvent_.interface(); }

	private:
		Vdcd& vdcd_;
		std::string name_;
		std::string dsuid_;
		int group_;
		std::string outputType_;
		bool dimmable_;
		double value_ {};
        SetEvent setEvent_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_VDCD_HPP
