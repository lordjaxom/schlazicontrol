#ifndef SCHLAZICONTROL_OUTPUT_WS281X_HPP
#define SCHLAZICONTROL_OUTPUT_WS281X_HPP

#include <cstddef>
#include <string>

#include "output.hpp"
#include "types.hpp"
#include "ws281x.hpp"

namespace sc {

	class Manager;
	class PropertyNode;

	class Ws281xOutput final
		: public Output
	{
	public:
		Ws281xOutput( std::string&& id, Manager& manager, PropertyNode const& properties );

		virtual bool acceptsChannels( std::size_t channels ) const override { return device_.channelCount() == channels; }

        virtual void statistics( std::ostream& os ) const override;

	protected:
		virtual void set( Input const& input, ChannelBuffer const& values ) override;

	private:
		Ws281xDevice device_;
        ChannelBuffer values_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_OUTPUT_WS281X_HPP
