#ifndef SCHLAZICONTROL_CONNECTION_HPP
#define SCHLAZICONTROL_CONNECTION_HPP

#include <memory>
#include <string>
#include <vector>

#include "core/input.hpp"
#include "core/output.hpp"
#include "transition.hpp"
#include "types.hpp"

namespace sc {

	class Manager;
	class PropertyNode;

	class Connection final
		: public Output
        , public Input
	{
	public:
		Connection( std::string&& id, Manager& manager, PropertyNode const& properties );

        bool acceptsChannels( std::size_t channels ) const override;
        std::size_t emitsChannels() const override { return channels_; }

        void transfer();

    protected:
        void set( Input const& input, ChannelBuffer const& values ) override;

        void doStatistics( std::ostream& os ) const override;

	private:
		std::vector< std::unique_ptr< TransitionInstance > > instances_;
        std::size_t channels_;
        ChannelBuffer input_;
        ChannelBuffer output_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_CONNECTION_HPP
