#ifndef SCHLAZICONTROL_COMPONENT_HPP
#define SCHLAZICONTROL_COMPONENT_HPP

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "utility.hpp"

namespace sc {

	class Manager;
	class PropertyNode;

    /**
     * class Component
     */

	class Component
	{
        template< typename Type > friend class ComponentRegistry;

    public:
        Component();
		explicit Component( std::string&& id );
        Component( Component const& ) = delete;
        virtual ~Component();

        std::string const& id() const { return id_; }
        std::string const& category() const { return *category_; }
        std::string const& name() const { return *name_; }

    private:
        std::string id_;
        std::string const* category_;
        std::string const* name_;
	};

    /**
     * class ComponentFactory
     */

    class ComponentFactory
	{
        template< typename Type > friend class ComponentRegistry;

		using Factory = std::function< std::unique_ptr< Component > ( std::string&&, Manager&, PropertyNode const& ) >;

    public:
        static ComponentFactory& instance();

		std::string generateId( std::string const& type );

        std::unique_ptr< Component > create(
                std::string const& type, std::string id, Manager& manager, PropertyNode const& properties );

	private:
		ComponentFactory();
		ComponentFactory( ComponentFactory const& ) = delete;

		void put( std::string&& name, Factory factory );

		std::map< std::string, Factory > components_;
        std::size_t generatedId_;
	};

    /**
     * class ComponentRegistry
     */

    template< typename Type >
	class ComponentRegistry
	{
	public:
		ComponentRegistry( std::string const& category, std::string const& name )
                : ComponentRegistry( category, name, str( category, ".", name ) )
		{
		}

        ComponentRegistry( std::string const& name )
                : ComponentRegistry( name, name, name )
        {
        }

    private:
        ComponentRegistry( std::string const& category, std::string const& name, std::string type )
        {
            ComponentFactory::instance().put(
                    std::move( type ),
                    [category, name]( std::string&& id, Manager& manager, PropertyNode const& properties ) {
                        std::unique_ptr< Component > result( new Type( std::move( id ), manager, properties ) );
                        result->category_ = &category;
                        result->name_ = &name;
                        return result;
                    } );
        }
    };

} // namespace sc

#endif // SCHLAZICONTROL_COMPONENT_HPP
