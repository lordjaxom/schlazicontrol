#ifndef SCHLAZICONTROL_CONNECTION_HPP
#define SCHLAZICONTROL_CONNECTION_HPP

#include <memory>
#include <string>
#include <vector>

#include "component.hpp"
#include "transition.hpp"
#include "types.hpp"

namespace sc {

	class ChannelValue;
	class Manager;
    class Output;
	class PropertyNode;

	class Connection
		: public Component
	{
	public:
		Connection( Manager& manager, std::string id, PropertyNode const& properties );

		void transfer( ChannelValue const& value );
        void retransfer();

	private:
		Manager& manager_;
		Output& output_;
		std::vector< std::unique_ptr< TransitionStateBase > > transitions_;
		ChannelValue lastValue_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_CONNECTION_HPP
