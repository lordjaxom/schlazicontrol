#ifndef SCHLAZICONTROL_WS281X_HPP
#define SCHLAZICONTROL_WS281X_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <system_error>

#include "core/component.hpp"
#include "forward.hpp"

namespace sc {

    /**
     * class Ws281x
     */

    struct Ws281xInternals;

    class Ws281x final
		: public Component
	{
	public:
		Ws281x( std::string&& id, Manager& manager, PropertyNode const& properties );

        virtual std::function< bool () > forkedProcess() const override;

		std::size_t ledCount() const { return ledCount_; }

		void send( ChannelBuffer const& values );

    protected:
        virtual void doStatistics( std::ostream& os ) const override;

	private:
		void connect();
		void reconnect();
		void retryConnect();

		void receiveInitMessage();

		bool handleError( std::error_code const& ec );

		Manager& manager_;
		std::uint16_t gpioPin_;
		std::size_t ledCount_;
        std::unique_ptr< Ws281xInternals > internals_;
	};

    /**
     * class Ws281xDevice
     */

    class Ws281xDevice final
	{
	public:
        Ws281xDevice( Ws281x& ws281x );

		std::size_t channelCount() const { return ws281x_.ledCount() * 3; }

		void send( ChannelBuffer const& values );

	private:
		Ws281x& ws281x_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_WS281X_HPP
