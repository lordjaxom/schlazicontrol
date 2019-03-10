#include <cstring>
#include <ostream>
#include <stdexcept>
#include <tuple>

#include "core/component.hpp"

using namespace std;

namespace sc {

    /**
     * class Component
     */

    namespace detail {

        ostream& operator<<( ostream& os, ComponentDescription const& description )
        {
            os << "\"" << description.id << "\" (of type \"" << description.category;
            if ( !description.nameOrType.empty() ) {
                os << "." << description.nameOrType;
            }
            os << "\")";
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
        return { category_, name_, id_ /*, requester*/ };
    }

    void Component::statistics( ostream& os ) const
    {
        if ( !statistics_ ) {
            return;
        }

        os << "\n\tComponent : category: " << category_ << ", name: " << name_ << ", id: " << id_;
        doStatistics( os );
    }


    /**
     * class ComponentFactory
     */

    namespace detail {

        bool operator<( ComponentEntry const& lhs, ComponentEntry const& rhs )
        {
            return forward_as_tuple( lhs.category, lhs.name ) < forward_as_tuple( rhs.category, rhs.name );
        }

        ComponentEntry::ComponentEntry( string_view category, string_view name, MakeComponent makeComponent )
                : category( category )
                , name( name )
                , makeComponent( makeComponent )
        {
            ComponentFactory::instance().components_.insert( *this );
        }

        auto splitComponentType( string_view type )
        {
            auto separator = type.find( ':' );
            return separator != string_view::npos
                    ? make_tuple( type.substr( 0, separator ), type.substr( separator + 1 ) )
                    : make_tuple( "", type );
        }

        struct ComponentEntryLookup
        {
            bool operator()( string_view type, ComponentEntry const& entry ) const
            {
                return splitComponentType( type ) < forward_as_tuple( entry.category, entry.name );
            }

            bool operator()( ComponentEntry const& entry, string_view type ) const
            {
                return forward_as_tuple( entry.category, entry.name ) < splitComponentType( type );
            }
        };

    } // namespace detail

    ComponentFactory& ComponentFactory::instance()
    {
        static ComponentFactory instance;
        return instance;
    }

    ComponentFactory::ComponentFactory()
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
		auto it = components_.find( type, detail::ComponentEntryLookup() );
		if ( it == components_.end() ) {
			throw runtime_error( str(
					"unable to create component \"", id, "\": type \"", type, "\" is not registered" ) );
		}
		auto component = it->makeComponent( move( id ), manager, properties );
        component->category_ = it->category;
        component->name_ = it->name;
        return component;
	}

} // namespace sc
