#ifndef SCHLAZICONTROL_OUTPUT_WS281X_HPP
#define SCHLAZICONTROL_OUTPUT_WS281X_HPP

#include <cstddef>
#include <string>

#include "output.hpp"
#include "ws281x.hpp"

namespace sc {

	class Manager;
	class ChannelBuffer;
	class PropertyNode;

	class Ws281xOutput
		: public Output
	{
	public:
		Ws281xOutput( Manager& manager, std::string id, PropertyNode const& properties );

		virtual bool acceptsChannels( std::size_t channels ) const override { return device_.channelCount() == channels; }

	private:
		virtual void set( ChannelBuffer const& values ) override;

		Manager& manager_;
		Ws281xDevice device_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_OUTPUT_WS281X_HPP
