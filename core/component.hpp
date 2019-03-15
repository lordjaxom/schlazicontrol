#ifndef SCHLAZICONTROL_COMPONENT_HPP
#define SCHLAZICONTROL_COMPONENT_HPP

#include <cassert>
#include <functional>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>

#include <boost/intrusive/set.hpp>

#include "core/config.hpp"
#include "statistics.hpp"
#include "utility_string.hpp"
#include "utility/string_view.hpp"

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
            string_view category;
            string_view nameOrType;
            std::string const& id;
            Component const* requester;
        };

        std::ostream& operator<<( std::ostream& os, ComponentDescription const& description );

    } // namespace detail

	class Component
	{
        friend class ComponentFactory;

        using Description = detail::ComponentDescription;

    public:
        static Description describe( std::string const& type, std::string const& id,
                                     Component const* requester = nullptr );

        Component();
		explicit Component( std::string&& id, bool statistics = true );
        Component( Component const& ) = delete;
        virtual ~Component();

        std::string const& id() const { return id_; }
        string_view category() const { return category_; }
        string_view name() const { return name_; }

        Description describe( Component const* requester = nullptr ) const;

        void statistics( std::ostream& os ) const;

        virtual std::function< bool () > forkedProcess() const SCHLAZICONTROL_UNLESS( FORK, final ) { return nullptr; }

    protected:
        virtual void doStatistics( std::ostream& os ) const = 0;

    private:
        std::string id_;
        bool statistics_;
        string_view category_;
        string_view name_;
	};

    /**
     * class ComponentFactory
     */

    namespace detail {

        using MakeComponent = std::add_pointer_t<
				std::unique_ptr< Component >( std::string&& id, Manager& manager, PropertyNode const& properties ) >;

        using ComponentEntryHook = boost::intrusive::set_member_hook< boost::intrusive::link_mode< boost::intrusive::auto_unlink > >;

        struct ComponentEntry
        {
            ComponentEntry( string_view category, string_view name, MakeComponent makeComponent ) noexcept;

            string_view category;
            string_view name;
            MakeComponent makeComponent;
            ComponentEntryHook hook;
        };

        using ComponentEntryHookOption = boost::intrusive::member_hook< ComponentEntry, ComponentEntryHook, &ComponentEntry::hook >;

    } // namespace detail

    class ComponentFactory
	{
        friend class detail::ComponentEntry;

        using EntrySet = boost::intrusive::set<
				detail::ComponentEntry,
				detail::ComponentEntryHookOption,
				boost::intrusive::constant_time_size< false > >;

    public:
        static ComponentFactory& instance() noexcept;

        ComponentFactory( ComponentFactory const& ) = delete;

		std::string generateId( std::string const& type );

        std::unique_ptr< Component > create(
                std::string const& type, std::string id, Manager& manager, PropertyNode const& properties );

	private:
		ComponentFactory();

        EntrySet components_;
        std::size_t generatedId_;
	};


    /**
     * class ComponentRegistry
     */

    template< typename Type >
	class ComponentRegistry
	{
        static std::unique_ptr< Component > makeComponent( std::string&& id, Manager& manager, PropertyNode const& properties )
        {
            return std::make_unique< Type >( std::move( id ), manager, properties );
        }

    public:
		ComponentRegistry( string_view category, string_view name ) noexcept
                : entry_( category, name, &makeComponent ) {}

        explicit ComponentRegistry( string_view name ) noexcept
                : ComponentRegistry( "", name ) {}

        ComponentRegistry( ComponentRegistry const& ) = delete;

    private:
        detail::ComponentEntry entry_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_COMPONENT_HPP
