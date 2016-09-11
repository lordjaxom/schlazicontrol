#ifndef SCHLAZICONTROL_COMPONENT_HPP
#define SCHLAZICONTROL_COMPONENT_HPP

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
	public:
		Component( std::string category, std::string id );
        Component( Component const& ) = delete;
        virtual ~Component();

        std::string const& category() const { return category_; }
        std::string const& id() const { return id_; }

    private:
        std::string category_;
        std::string id_;
	};

    /**
     * class ComponentFactory
     */

    class ComponentFactory
	{
        template< typename Type > friend class ComponentRegistry;

		using Factory = Component* ( Manager&, std::string, PropertyNode const& );

    public:
        static ComponentFactory& instance();

		std::string generateId( std::string const& name );

		std::unique_ptr< Component > create(
				Manager& manager, std::string const& name, std::string id, PropertyNode const& properties );

	private:
		ComponentFactory() = default;
		ComponentFactory( ComponentFactory const& ) = delete;

		void put( std::string&& name, Factory* factory );

		std::map< std::string, Factory* > components_;
        std::size_t generatedId_;
	};

    /**
     * class ComponentRegistry
     */

    template< typename Type >
	class ComponentRegistry
	{
	public:
		ComponentRegistry( std::string name )
		{
			ComponentFactory::instance().put(
                    std::move( name ), []( Manager& manager, std::string id, PropertyNode const& properties ) {
                        return static_cast< Component* >( new Type( manager, std::move( id ), properties ) );
                    } );
		}
	};

} // namespace sc

#endif // SCHLAZICONTROL_COMPONENT_HPP
