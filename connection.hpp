#ifndef SCHLAZICONTROL_CONNECTION_HPP
#define SCHLAZICONTROL_CONNECTION_HPP

#include <memory>
#include <string>
#include <vector>

#include "component.hpp"
#include "transition.hpp"

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

	private:
		Manager& manager_;
		Output& output_;
		std::vector< std::unique_ptr< TransitionStateBase > > transitions_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_CONNECTION_HPP
