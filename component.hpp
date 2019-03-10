#ifndef SCHLAZICONTROL_COMPONENT_HPP
#define SCHLAZICONTROL_COMPONENT_HPP

#include <cassert>
#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include <boost/intrusive/set.hpp>

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

        using Factory = std::add_pointer_t< std::unique_ptr< Component >( std::string&&, Manager&, PropertyNode const& ) >;

        struct Entry
        {
            char const* category;
            char const* name;
            Factory factory;
            boost::intrusive::set_member_hook<> hook;
        };

        friend bool operator<( Entry const& lhs, Entry const& rhs ) noexcept;

        struct EntryCompare
        {
            bool operator()( char const* type, Entry const& entry ) const;
            bool operator()( Entry const& entry, char const* type ) const;
        };

        using EntryMember = boost::intrusive::member_hook< Entry, boost::intrusive::set_member_hook<>, &Entry::hook >;
        using EntrySet = boost::intrusive::set< Entry, EntryMember >;

    public:
        static ComponentFactory& instance() noexcept;

        ComponentFactory( ComponentFactory const& ) = delete;

		std::string generateId( std::string const& type );

        std::unique_ptr< Component > create(
                std::string const& type, std::string id, Manager& manager, PropertyNode const& properties );

	private:
		ComponentFactory() noexcept;

		void put( Entry& entry ) noexcept;

        EntrySet components_;
        std::size_t generatedId_;
	};


    /**
     * class ComponentRegistry
     */

    template< typename Type >
	class ComponentRegistry
	{
	public:
		ComponentRegistry( char const* category, char const* name ) noexcept
                : entry_ { category, name, [=]( auto&& id, auto& manager, auto const& properties ) {
                    std::unique_ptr< Component > result( new Type( std::move( id ), manager, properties ) );
                    result->category_ = category;
                    result->name_ = name;
                    return result;
                } }
        {
            assert( category != nullptr && name != nullptr );
            ComponentFactory::instance().put( entry_ );
        }

        explicit ComponentRegistry( char const* name ) noexcept
                : ComponentRegistry( "", name ) {}

        ComponentRegistry( ComponentRegistry const& ) = delete;

    private:
        ComponentFactory::Entry entry_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_COMPONENT_HPP
