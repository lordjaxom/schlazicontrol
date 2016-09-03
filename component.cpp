#include <stdexcept>

#include "component.hpp"

using namespace std;

namespace sc {

	Component::Component( string category, string id )
		: category_( move( category ) )
        , id_( move( id ) )
	{
	}

	Component::~Component() = default;

	StaticInstance< ComponentFactory > ComponentFactory::instance;

    unique_ptr< Component > ComponentFactory::create(
			Manager& manager, string const& name, string id, PropertyNode const& properties )
	{
		auto it = components_.find( name );
		if ( it == components_.end() ) {
			throw runtime_error( str(
					"unable to create component \"", id, "\": type \"", name, "\" is not registered" ) );
		}
		return unique_ptr< Component > { ( *it->second )( manager, move( id ), properties ) };
	}

	void ComponentFactory::put( string name, Factory* factory )
	{
		auto it = components_.emplace( move( name ), factory );
		if ( !it.second ) {
            throw invalid_argument( "multiple registration of same component type" );
		}
	}

} // namespace sc
