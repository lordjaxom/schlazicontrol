#include <utility>

#include "input_vdcd.hpp"
#include "manager.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

	struct VdcdInputLimits
	{
		static constexpr double minimum = 0.0;
		static constexpr double maximum = 100.0;
	};

	static PropertyKey const vdcdProperty( "vdcd" );
	static PropertyKey const dsuidProperty( "dsuid" );
	static PropertyKey const groupProperty( "group" );
	static PropertyKey const outputTypeProperty( "outputType", "" );
	static PropertyKey const dimmableProperty( "dimmable", false );

	VdcdInput::VdcdInput( string&& id, Manager& manager, PropertyNode const& properties )
		: Component( move( id ) )
		, device_( manager.get< Vdcd >( *this, properties[ vdcdProperty ] ), this->id(),
                   properties[ dsuidProperty ].as< string >(), properties[ groupProperty ].as< int >(),
                   properties[ outputTypeProperty ].as< string >(), properties[ dimmableProperty ].as< bool >() )
	{
		device_.setEvent().subscribe( [this]( double value ) { set( value ); } );
	}

	void VdcdInput::set( double value )
	{
        if ( value != value_ ) {
            value_ = value;
            inputChangeEvent_( ChannelBuffer { ranged< VdcdInputLimits >( value ) } );
        }
	}

	void VdcdInput::doStatistics( ostream& os ) const
	{
		os << ", value: " << value_;
	}

	static InputRegistry< VdcdInput > registry( "vdcd" );

} // namespace sc
