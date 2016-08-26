#include <stdexcept>
#include <utility>

#include "component.hpp"

using namespace std;

namespace sc {

	Component::Component( string const& category, string const& id )
		: category_( category )
        , id_( id )
	{
	}

    Component::~Component() = default;

    Standalone::Standalone( string const& id )
        : Component( "standalone", id )
    {
    }

    size_t Standalone::channels() const
    {
        throw invalid_argument( "standalone components don't send channels" );
    }

    bool Standalone::acceptsChannels( size_t channels ) const
    {
        throw invalid_argument( "standalone components don't accept channels" );
    }

    unique_ptr< Component > ComponentFactory::create(
			Manager& manager, string const& name, string id, PropertyNode const& properties )
	{
		auto it = components_.find( name );
		if ( it == components_.end() ) {
			throw runtime_error( str( "component type \"", name, "\" for component \"", id, "\" not found" ) );
		}
		return ( *it->second )( manager, move( id ), properties );
	}

	StaticInstance< ComponentFactory > ComponentFactory::instance;

	void ComponentFactory::put( string name, FactoryFunction factory )
	{
		auto it = components_.emplace( move( name ), factory );
		if ( !it.second ) {
			throw runtime_error( str( "duplicate component ", it.first->first ) );
		}
	}

} // namespace sc
