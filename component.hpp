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

    template< typename Type >
	class ComponentRegistry;

	class ComponentFactory
	{
		friend class StaticInstance< ComponentFactory >;

		using Factory = Component* ( Manager&, std::string, PropertyNode const& );

	public:
		static StaticInstance< ComponentFactory > instance;

		std::unique_ptr< Component > create(
				Manager& manager, std::string const& name, std::string id, PropertyNode const& properties );

		template< typename Type >
		void put( std::string name )
		{
			put( std::move( name ), []( Manager& manager, std::string id, PropertyNode const& properties ) {
				return static_cast< Component* >( new Type( manager, std::move( id ), properties ) );
			} );
		}

	private:
		ComponentFactory() = default;
		ComponentFactory( ComponentFactory const& ) = delete;

		void put( std::string name, Factory* factory );

		std::map< std::string, Factory* > components_;
	};

	template< typename Type >
	class ComponentRegistry
	{
	public:
		ComponentRegistry( std::string name )
		{
			ComponentFactory::instance->put< Type >( std::move( name ) );
		}
	};

} // namespace sc

#endif // SCHLAZICONTROL_COMPONENT_HPP
