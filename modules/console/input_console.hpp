#ifndef SCHLAZICONTROL_INPUT_CONSOLE_HPP
#define SCHLAZICONTROL_INPUT_CONSOLE_HPP

#include "console.hpp"
#include "core/input.hpp"

namespace sc {

    /**
     * class ConsoleInput
     */

    class ConsoleInput final
            : public Input
    {
    public:
        ConsoleInput( std::string&& id, Manager& manager, PropertyNode const& properties );

        virtual std::size_t emitsChannels() const override { return 1; }

    protected:
        virtual void doStatistics( std::ostream& os ) const override;

    private:
        void change( bool value );

        ConsoleDevice device_;
        bool value_ {};
    };

} // namespace sc

#endif // SCHLAZICONTROL_INPUT_DEBUG_HPP
