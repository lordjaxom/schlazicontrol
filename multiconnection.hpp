#ifndef SCHLAZICONTROL_MULTICONNECTION_HPP
#define SCHLAZICONTROL_MULTICONNECTION_HPP

#include <unordered_map>

#include "core/input.hpp"
#include "core/output.hpp"
#include "types.hpp"

namespace sc {

    class Manager;
    class PropertyNode;

    class MultiConnection final
            : public Output
            , public Input
    {
    public:
        MultiConnection( std::string&& id, Manager& manager, PropertyNode const& properties );

        virtual bool acceptsChannels( std::size_t channels ) const override { return true; }
        virtual std::size_t emitsChannels() const override { return channels_; }

    protected:
        virtual void set( Input const& input, ChannelBuffer const& values ) override;

        virtual void doStatistics( std::ostream& os ) const override;

    private:
        std::size_t channels_;
        ChannelBuffer values_;
        std::unordered_map< std::string, ChannelBuffer > buffers_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_MULTICONNECTION_HPP
