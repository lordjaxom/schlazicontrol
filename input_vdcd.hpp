#ifndef SCHLAZICONTROL_INPUT_VDCD_HPP
#define SCHLAZICONTROL_INPUT_VDCD_HPP

#include <cstddef>
#include <string>

#include "input.hpp"
#include "vdcd.hpp"

namespace sc {

	class Manager;
	class PropertyNode;

	class VdcdInput
		: public Input
	{
	public:
		VdcdInput( std::string&& id, Manager& manager, PropertyNode const& properties );

        virtual std::size_t emitsChannels() const override { return 1; }

	private:
		void set( double value );

		Manager& manager_;
		VdcdDevice device_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_INPUT_VDCD_HPP
