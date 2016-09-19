#ifndef SCHLAZICONTROL_CONSOLE_HPP
#define SCHLAZICONTROL_CONSOLE_HPP

#include <memory>
#include <system_error>
#include <unordered_map>

#include "component.hpp"
#include "event.hpp"

namespace sc {

    /**
     * class Console
     */

    class ConsoleDevice;
    struct ConsoleInternals;

    class Console final
            : public Component
    {
    public:
        Console( std::string&& id, Manager& manager, PropertyNode const& properties );

        void add( ConsoleDevice* device );
        void remove( ConsoleDevice* device );

    protected:
        virtual void doStatistics( std::ostream& os ) const override;

    private:
        void read();

        bool handleError( std::error_code const& ec );

        std::unique_ptr< ConsoleInternals > internals_;
        std::unordered_map< std::string, ConsoleDevice* > devices_;
    };

    /**
     * class ConsoleDevice
     */

    class ConsoleDevice final
    {
    public:
        using ChangeEvent = Event< void ( bool ) >;

        ConsoleDevice( Console& console, std::string tag );
        ConsoleDevice( ConsoleDevice const& ) = delete;
        ~ConsoleDevice();

        std::string const& tag() const { return tag_; }

        void toggle();

        ChangeEvent::Interface& changeEvent() { return changeEvent_.interface(); }

    private:
        Console& console_;
        std::string tag_;
        bool value_ {};
        ChangeEvent changeEvent_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_CONSOLE_HPP
