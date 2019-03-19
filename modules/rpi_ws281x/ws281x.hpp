#ifndef SCHLAZICONTROL_WS281X_HPP
#define SCHLAZICONTROL_WS281X_HPP

#include <cstddef>

#include "core/component.hpp"
#include "forward.hpp"

namespace sc {

    /**
     * class Ws281x
     */

    class Ws281x
		: public Component
	{
	public:
		explicit Ws281x( std::string&& id )
				: Component( std::move( id )) {}

		virtual std::size_t ledCount() const = 0;

		virtual void send( ChannelBuffer const& values ) = 0;
	};

} // namespace sc

#endif // SCHLAZICONTROL_WS281X_HPP
