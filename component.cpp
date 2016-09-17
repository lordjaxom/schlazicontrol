#include <cassert>
#include <ostream>
#include <stdexcept>

#include "component.hpp"

using namespace std;

namespace sc {

    /**
     * class Component
     */

    Component::Component( std::string&& id )
            : id_( move( id ) )
    {
    }

	Component::~Component() = default;

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

    /**
     * struct StatisticsWriter specialization
     */

    void StatisticsWriter< Component >::operator()( ostream& os, Component const& component )
    {
        if ( component.statistics() ) {
            os << "\n\tcomponent {" << component.category() << "." << component.name() << ":" << component.id()
               << "} : ";
            component.statistics( os );
        }
    }

} // namespace sc
