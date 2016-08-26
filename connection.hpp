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
		Connection( Manager& manager, std::string const& id, PropertyNode const& properties );

        virtual std::size_t channels() const override;
        virtual bool acceptsChannels( std::size_t channels ) const override;

	private:
		void transfer( ChannelValue const& value );

		Manager& manager_;
		Output& output_;
		std::vector< std::unique_ptr< TransitionInstance > > transitions_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_CONNECTION_HPP
