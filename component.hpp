#ifndef SCHLAZICONTROL_COMPONENT_HPP
#define SCHLAZICONTROL_COMPONENT_HPP

#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "statistics.hpp"
#include "utility_string.hpp"

namespace sc {

	class Manager;
	class PropertyNode;

    /**
     * class Component
     */

    class Component;

    namespace detail {

        struct ComponentDescription
        {
            std::string const& category;
            std::string const& nameOrType;
            std::string const& id;
            Component const* requester;
        };

        std::ostream& operator<<( std::ostream& os, ComponentDescription const& description );

    } // namespace detail

	class Component
	{
        template< typename Type > friend class ComponentRegistry;

        using Description = detail::ComponentDescription;

    public:
        static Description describe( std::string const& type, std::string const& id,
                                     Component const* requester = nullptr );

        Component();
		explicit Component( std::string&& id, bool statistics = true );
        Component( Component const& ) = delete;
        virtual ~Component();

        std::string const& id() const { return id_; }
        std::string const& category() const { return *category_; }
        std::string const& name() const { return *name_; }

        Description describe( Component const* requester = nullptr ) const;

        void statistics( std::ostream& os ) const;

        virtual std::function< void () > forkedProcess() const { return nullptr; }

    protected:
        virtual void doStatistics( std::ostream& os ) const = 0;

    private:
        std::string id_;
        bool statistics_;
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

		void put( std::string&& name, Factory&& factory );

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
		ComponentRegistry( std::string category, std::string name )
                //: ComponentRegistry( std::move( category ), std::move( name ), str( category, ".", name ) )
		{
            using namespace std::placeholders;
            auto type = category.empty() ? name : str( category, ".", name );
            auto wrapper = []( auto&& c, auto&& n, auto&& id, auto&& manager, auto&& properties ) {
                std::unique_ptr< Component > result( new Type( std::move( id ), manager, properties ));
                result->category_ = &c;
                result->name_ = &n;
                return result;
            };
            ComponentFactory::instance().put( std::move( type ), std::bind( std::move( wrapper ), std::move( category ), std::move( name ), _1, _2, _3 ) );
		}

        ComponentRegistry( std::string name )
                : ComponentRegistry( {}, std::move( name ) )
                //: ComponentRegistry( {}, name, name )
        {
        }
    };

} // namespace sc

#endif // SCHLAZICONTROL_COMPONENT_HPP
