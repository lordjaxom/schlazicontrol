#include <cassert>
#include <ostream>
#include <stdexcept>

#include "component.hpp"

using namespace std;

namespace sc {

    /**
     * class Component
     */

    Component::Component( std::string&& id, bool statistics )
            : id_( move( id ) )
            , statistics_( statistics )
    {
    }

    Component::~Component() = default;

    void Component::statistics( ostream& os ) const
    {
        if ( !statistics_ ) {
            return;
        }

        os << "\n\tComponent : category: " << *category_ << ", name: " << *name_ << ", id: " << id_;
        doStatistics( os );
    }

    /**
     * class ComponentFactory
     */

    ComponentFactory& ComponentFactory::instance()
    {
        static ComponentFactory instance;
        return instance;
    }

    ComponentFactory::ComponentFactory()
        : generatedId_( 924536 )
    {
    }

	string ComponentFactory::generateId( string const& name )
	{
        auto id = str( name, ".", generatedId_ );
        generatedId_ = ( generatedId_ - 100000 + 99991 ) % 900000 + 100000;
        return id;
	}

    unique_ptr< Component > ComponentFactory::create(
            string const& type, string id, Manager& manager, PropertyNode const& properties )
	{
		auto it = components_.find( type );
		if ( it == components_.end() ) {
			throw runtime_error( str(
					"unable to create component \"", id, "\": type \"", type, "\" is not registered" ) );
		}
		return it->second( move( id ), manager, properties );
	}

	void ComponentFactory::put( string&& name, Factory factory )
	{
		auto it = components_.emplace( move( name ), move( factory ) );
		if ( !it.second ) {
            throw runtime_error( str(
                    "unable to register component type \"", it.first->first, "\": type already registered" ) );
		}
	}

} // namespace sc
