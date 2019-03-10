#ifndef SCHLAZICONTROL_PROPERTIES_HPP
#define SCHLAZICONTROL_PROPERTIES_HPP

#include <chrono>
#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <string>

#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/lexical_cast.hpp>
#include <nlohmann/json.hpp>

#include "typeinfo.hpp"
#include "utility_graphics.hpp"
#include "utility_string.hpp"
#include "utility/string_view.hpp"

namespace sc {

    /**
     * class PropertyKey
     */

	class PropertyKey
	{
	public:
		explicit PropertyKey( std::string name, nlohmann::json defaultValue = {} );

		std::string const& name() const { return name_; }
        nlohmann::json const& defaultValue() const { return defaultValue_; }

	private:
		std::string name_;
        nlohmann::json defaultValue_;
	};

    /**
     * class PropertyNode
     */

    namespace detail {

        struct PropertyNodeIterator;

        [[noreturn]]
        void propertyConversionError( std::string const& path, char const* typeName, nlohmann::json::exception const& e );

    } // namespace detail

	class PropertyNode
	{
    public:
        using const_iterator = detail::PropertyNodeIterator;

        PropertyNode();
        PropertyNode( std::string path, nlohmann::json const& value );

        explicit operator bool() const { return !value_->is_null(); }

        std::string const& path() const { return path_; }
        char const* typeName() const;

		template< typename Type >
		bool is() const
		{
		    try {
		        value_->get< Type >();
		        return true;
		    } catch ( nlohmann::json::exception const& ) {
		        return false;
		    }
		}

        template< typename Type >
        auto as() const
        {
            try {
                return value_->get< Type >();
            } catch ( nlohmann::json::exception const& e ) {
                detail::propertyConversionError( path_, sc::typeName< Type >(), e );
            }
        }

        const_iterator begin() const;
        const_iterator end() const;

        bool has( std::string const& key ) const;

        PropertyNode operator[]( std::string const& key ) const;
        PropertyNode operator[]( PropertyKey const& key ) const;

    private:
        const_iterator iter( nlohmann::json::const_iterator it ) const;

        PropertyNode get( std::string const& key, nlohmann::json const& defaultValue = nullptr ) const;

        std::string path_;
        nlohmann::json const* value_;
	};

	namespace detail {

        struct PropertyNodeIterator
                : boost::iterator_adaptor<
                        PropertyNodeIterator,
                        nlohmann::json::const_iterator,
                        PropertyNode,
                        boost::use_default,
                        PropertyNode >
        {
            friend class boost::iterator_core_access;

            PropertyNodeIterator(
                    std::string const& path, nlohmann::json::const_iterator first, nlohmann::json::const_iterator it );

        private:
            reference dereference() const;

            std::string const* path_;
            nlohmann::json::const_iterator first_;
        };

	} // namespace detail


    /**
     * class Properties
     */

	class Properties : PropertyNode
	{
	public:
        explicit Properties( std::string const& propertiesFile );

		using PropertyNode::is;
		using PropertyNode::as;
        using PropertyNode::operator[];

	private:
		nlohmann::json value_;
	};

    /**
     * property conversion
     */

    namespace detail {
/*
        template<>
        struct PropertyConverter< Rgb >
        {
            static bool is( PropertyNode const& node );
            static Rgb convert( PropertyNode const& node );
        };

        template< typename Type, typename Enable >
        Type PropertyConverter< Type, Enable >::convert( PropertyNode const& node )
        {
            try {
                return boost::lexical_cast< Type >( PropertyConverter< std::string >::convert( node ) );
            }
            catch ( boost::bad_lexical_cast const& e ) {
                throw std::runtime_error( str( "couldn't convert property \"", node.path_, "\" to requested type" ) );
            }
        }
*/
    } // namespace detail

} // namespace sc

/**
 * Property conversion
 */

namespace nlohmann {

    template<>
    struct adl_serializer< std::chrono::nanoseconds >
    {
        static void from_json( json const& src, std::chrono::nanoseconds& dst );
    };

    template< typename Rep, typename Period >
    struct adl_serializer< std::chrono::duration< Rep, Period > >
    {
        static void from_json( json const& src, std::chrono::duration< Rep, Period >& dst )
        {
            return std::chrono::duration_cast< std::chrono::duration< Rep, Period > >(
                    src.get< std::chrono::nanoseconds >() );
        }
    };

} // namespace nlohmann

#endif // SCHLAZICONTROL_PROPERTIES_HPP
