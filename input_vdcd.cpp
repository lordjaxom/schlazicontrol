#include <functional>

#include "input_vdcd.hpp"
#include "manager.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

	static PropertyKey const vdcdProperty( "vdcd" );
	static PropertyKey const dsuidProperty( "dsuid" );
	static PropertyKey const groupProperty( "group" );
	static PropertyKey const outputTypeProperty( "outputType", "" );
	static PropertyKey const dimmableProperty( "dimmable", "false" );

	VdcdInput::VdcdInput( Manager& manager, string const& id, PropertyNode const& properties )
		: Input( id )
		, manager_( manager )
		, device_(
				manager, properties[ vdcdProperty].as< string >(), id, properties[ dsuidProperty ].as< string >(),
				properties[ groupProperty ].as< int >(), properties[ outputTypeProperty ].as< string >(),
				properties[ dimmableProperty ].as< bool >(), [this]( double value ) { set( value ); } )
	{
	}

	void VdcdInput::set( double value )
	{
		raiseInputChange( { value, 0.0, 100.0 } );
	}

	__attribute__(( unused )) static InputRegistry< VdcdInput > registry( "vdcd" );

} // namespace sc
