#ifndef SCHLAZICONTROL_MULTICONNECTION_HPP
#define SCHLAZICONTROL_MULTICONNECTION_HPP

#include <unordered_map>

#include "input.hpp"
#include "output.hpp"
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

        virtual void statistics( std::ostream& os ) const override;

    protected:
        virtual void set( Input const& input, ChannelBuffer const& values ) override;

    private:
        std::size_t channels_;
        ChannelBuffer values_;
        std::unordered_map< std::string, ChannelBuffer > buffers_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_MULTICONNECTION_HPP
