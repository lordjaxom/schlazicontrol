#ifndef SCHLAZICONTROL_INPUT_DEBUG_HPP
#define SCHLAZICONTROL_INPUT_DEBUG_HPP

#include <memory>
#include <system_error>

#include "input.hpp"

namespace sc {

    /**
     * class DebugInput
     */

    struct DebugInputInternals;

    class DebugInput final
            : public Input
    {
    public:
        DebugInput( std::string&& id, Manager& manager, PropertyNode const& properties );

        virtual std::size_t emitsChannels() const override { return 1; }

    protected:
        virtual void doStatistics( std::ostream& os ) const override;

    private:
        void read();
        void toggle();

        bool handleError( std::error_code const& ec );

        std::unique_ptr< DebugInputInternals > internals_;
        std::string tag_;
        bool value_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_INPUT_DEBUG_HPP
