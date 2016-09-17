#ifndef SCHLAZICONTROL_INPUT_VDCD_HPP
#define SCHLAZICONTROL_INPUT_VDCD_HPP

#include <cstddef>
#include <string>

#include "input.hpp"
#include "vdcd.hpp"

namespace sc {

	class Manager;
	class PropertyNode;

	class VdcdInput final
		: public Input
	{
	public:
		VdcdInput( std::string&& id, Manager& manager, PropertyNode const& properties );

        virtual std::size_t emitsChannels() const override { return 1; }

		virtual void statistics( std::ostream& os ) const override;

	private:
		void set( double value );

		Manager& manager_;
		VdcdDevice device_;
        double value_ {};
	};

} // namespace sc

#endif // SCHLAZICONTROL_INPUT_VDCD_HPP
