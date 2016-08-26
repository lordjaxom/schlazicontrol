#ifndef SCHLAZICONTROL_OUTPUT_HPP
#define SCHLAZICONTROL_OUTPUT_HPP

#include <cstddef>
#include <string>

#include "component.hpp"
#include "utility.hpp"

namespace sc {

    class ChannelBuffer;

	class Output
		: public Component
	{
	public:
		explicit Output( std::string const& id );

		virtual std::size_t channels() const final;

        virtual void set( ChannelBuffer const& values ) = 0;
	};

	template< typename Type >
	class OutputRegistry
		: public ComponentRegistry< Type >
	{
	public:
		OutputRegistry( char const* name ) : ComponentRegistry< Type >( str( "output.", name ) ) {}
	};

} // namespace sc

#endif // SCHLAZICONTROL_OUTPUT_HPP
