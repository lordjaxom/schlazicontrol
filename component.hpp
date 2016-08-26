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
		Component( std::string const& category, std::string const& id );
        Component( Component const& ) = delete;
        virtual ~Component();

        std::string const& category() const { return category_; }
        std::string const& id() const { return id_; }

		virtual std::size_t channels() const = 0;
		virtual bool acceptsChannels( std::size_t channels ) const = 0;

    private:
        std::string category_;
        std::string id_;
	};

    class Standalone : public Component
    {
    public:
        explicit Standalone( std::string const& id );

        virtual std::size_t channels() const final;
        virtual bool acceptsChannels( std::size_t channels ) const final;
    };

    template< typename Type >
	class ComponentRegistry;

	class ComponentFactory
	{
		friend class StaticInstance< ComponentFactory >;

		using FactoryFunction = std::unique_ptr< Component > (*)( Manager&, std::string, PropertyNode const& );

	public:
		static StaticInstance< ComponentFactory > instance;

		std::unique_ptr< Component > create(
				Manager& manager, std::string const& name, std::string id, PropertyNode const& properties );

        void put( std::string name, FactoryFunction factory );

	private:
		ComponentFactory() = default;
		ComponentFactory( ComponentFactory const& ) = delete;

		std::map< std::string, FactoryFunction > components_;
	};

	template< typename Type >
	class ComponentRegistry
	{
	public:
		ComponentRegistry( std::string name )
		{
			ComponentFactory::instance->put(
                    std::move( name ), []( Manager& manager, std::string id, PropertyNode const& properties ) {
                        return std::unique_ptr< Component >( new Type( manager, std::move( id ), properties ) );
                    } );
		}
	};

} // namespace sc

#endif // SCHLAZICONTROL_COMPONENT_HPP
