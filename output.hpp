#ifndef SCHLAZICONTROL_OUTPUT_HPP
#define SCHLAZICONTROL_OUTPUT_HPP

#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

#include "component.hpp"
#include "utility.hpp"

namespace sc {

    class ChannelBuffer;
    class Input;
    class Manager;
    class PropertyNode;

    /**
     * class Output
     */

    class Output
            : public virtual Component
    {
    public:
        static void checkConnection(
                Component const& input, Component const& output, std::size_t emitsChannels, bool acceptsChannels );

        static constexpr std::true_type multiInput {};

        template< typename = void >
        Output()
        {
        }

        template< typename MultiInput = std::false_type >
        Output( Manager& manager, PropertyNode const& inputsNode, MultiInput multiInput = {} )
        {
            initialize( manager, inputsNode, multiInput );
        }

        virtual bool acceptsChannels( std::size_t channels ) const = 0;

    protected:
        virtual void set( Input const& input, ChannelBuffer const& values ) = 0;

        std::vector< Input const* > const& inputs() const { return inputs_; }

    private:
        void initialize( Manager& manager, PropertyNode const& inputsNode, std::false_type = {} );
        void initialize( Manager& manager, PropertyNode const& inputsNode, std::true_type );
        void setup( Input& input );

        std::vector< Input const* > inputs_;
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
