#include <cstring>
#include <ostream>
#include <stdexcept>

#include "component.hpp"

using namespace std;

namespace sc {

    /**
     * class Component
     */

    namespace detail {

        ostream& operator<<( ostream& os, ComponentDescription const& description )
        {
            os << "\"" << description.id << "\" (of type \"";
            if ( !description.category.empty() ) {
                os << description.category << ".";
            }
            os << description.nameOrType << "\")";
            if ( description.requester != nullptr ) {
                os << " defined by " << description.requester->describe();
            }
            return os;
        }

    } // namespace detail

    Component::Description Component::describe( string const& type, string const& id, Component const* requester )
    {
        static string category; // must stay alive
        return { category, type, id, requester };
    }

    Component::Component( std::string&& id, bool statistics )
            : id_( move( id ) )
            , statistics_( statistics )
    {
    }

    Component::~Component() = default;

    Component::Description Component::describe( Component const* requester ) const
    {
        return { *category_, *name_, id_ /*, requester*/ };
    }

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

    bool operator<( ComponentFactory::Entry const& lhs, ComponentFactory::Entry const& rhs ) noexcept
    {
        int r;
        return ( r = strcmp( lhs.category, rhs.category ) ) < 0 ||
                ( r == 0 && strcmp( lhs.name, rhs.name ) < 0 );
    }

    ComponentFactory& ComponentFactory::instance() noexcept
    {
        static ComponentFactory instance;
        return instance;
    }

    ComponentFactory::ComponentFactory() noexcept
        : generatedId_( 924536 ) {}

	string ComponentFactory::generateId( string const& name )
	{
        auto id = str( name, ".", generatedId_ );
        generatedId_ = ( generatedId_ - 100000 + 99991 ) % 900000 + 100000;
        return id;
	}

    unique_ptr< Component > ComponentFactory::create(
            string const& type, string id, Manager& manager, PropertyNode const& properties )
	{
		auto it = components_.find( type, EntryCompare() );
		if ( it == components_.end() ) {
			throw runtime_error( str(
					"unable to create component \"", id, "\": type \"", type, "\" is not registered" ) );
		}
		return it->factory( move( id ), manager, properties );
	}

	void ComponentFactory::put( Entry& entry ) noexcept
	{
	    auto it = components_.insert( entry );
	    assert( it.second );
	}

} // namespace sc
