#ifndef SCHLAZICONTROL_CONNECTION_HPP
#define SCHLAZICONTROL_CONNECTION_HPP

#include <memory>
#include <string>
#include <vector>

#include "input.hpp"
#include "output.hpp"
#include "transition.hpp"
#include "types.hpp"

namespace sc {

	class Manager;
    class Output;
	class PropertyNode;

	class Connection final
		: public Output
        , public Input
	{
	public:
		Connection( std::string&& id, Manager& manager, PropertyNode const& properties );

        virtual bool acceptsChannels( std::size_t channels ) const override;
        virtual std::size_t emitsChannels() const override { return channels_; }

        void transfer();

    protected:
        virtual void set( Input const& input, ChannelBuffer const& values ) override;

	private:
        Manager& manager_;
		std::vector< std::unique_ptr< TransitionInstance > > instances_;
        std::size_t channels_;
        ChannelBuffer lastValues_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_CONNECTION_HPP
