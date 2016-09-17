#ifndef SCHLAZICONTROL_OUTPUT_HPP
#define SCHLAZICONTROL_OUTPUT_HPP

#include <cstddef>
#include <string>

#include "component.hpp"
#include "utility.hpp"

namespace sc {

    class ChannelBuffer;
    class Input;

    /**
     * class Output
     */

    class Output
            : public virtual Component
    {
    public:
        template< typename = std::nullptr_t > Output() {}

        virtual bool acceptsChannels( std::size_t channels ) const = 0;
    };

    /**
     * class OutputRegistry
     */

    template< typename Type >
	class OutputRegistry
		: public ComponentRegistry< Type >
	{
	public:
		OutputRegistry( char const* name )
                : ComponentRegistry< Type >( "output", name )
        {
        }
	};

} // namespace sc

#endif // SCHLAZICONTROL_OUTPUT_HPP
