#ifndef SCHLAZICONTROL_OUTPUT_DEBUG_HPP
#define SCHLAZICONTROL_OUTPUT_DEBUG_HPP

#include "output.hpp"
#include "types.hpp"

namespace sc {

    /**
     * class DebugOutput
     */

    class DebugOutput final
            : public Output
    {
    public:
        DebugOutput( std::string&& id, Manager& manager, PropertyNode const& properties );

        virtual bool acceptsChannels( std::size_t channels ) const override { return channels == channels_; }

    protected:
        virtual void set( Input const& input, ChannelBuffer const& values ) override;

        virtual void doStatistics( std::ostream& os ) const override;

    private:
        std::size_t channels_;
        ChannelBuffer values_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_OUTPUT_DEBUG_HPP
