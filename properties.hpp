#ifndef SCHLAZICONTROL_PROPERTIES_HPP
#define SCHLAZICONTROL_PROPERTIES_HPP

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <tuple>

#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/lexical_cast.hpp>
#include <json/value.h>

#include "utility.hpp"

namespace sc {

    /**
     * class PropertyKey
     */

	class PropertyKey
	{
	public:
		explicit PropertyKey( std::string name, Json::Value defaultValue = {} );

		std::string const& name() const { return name_; }
        Json::Value const& defaultValue() const { return defaultValue_; }

	private:
		std::string name_;
        Json::Value defaultValue_;
	};

    /**
     * class PropertyNode
     */

    class PropertyNode;

    namespace detail {

        template< typename Type, typename Enable = void >
        struct PropertyConverter
        {
            static bool is( PropertyNode const& node );
            static Type convert( PropertyNode const& node );
        };

        struct PropertyNodeTransformer;

    } // namespace detail

	class PropertyNode
	{
        template< typename Type, typename Enable >
        friend struct detail::PropertyConverter;

    public:
        using const_iterator = boost::transform_iterator<
                detail::PropertyNodeTransformer,
                boost::zip_iterator<
                        boost::tuple< Json::Value::const_iterator, boost::counting_iterator< std::size_t > > > >;

        PropertyNode();
        PropertyNode( std::string path, Json::Value const& value );

        explicit operator bool() const { return !value_->isNull(); }

		template< typename Type >
		bool is() const
		{
			return detail::PropertyConverter< Type >::is( *this );
		}

        template< typename Type >
        auto as() const -> decltype( detail::PropertyConverter< Type >::convert( std::declval< PropertyNode >() ) )
        {
            return detail::PropertyConverter< Type >::convert( *this );
        }

        const_iterator begin() const;
        const_iterator end() const;

        PropertyNode operator[]( std::string const& key ) const;
        PropertyNode operator[]( PropertyKey const& key ) const;

    private:
        PropertyNode get(
                std::string const& key, Json::Value const& defaultValue = {},
                std::initializer_list< Json::ValueType > allowedTypes = {}) const;

        const_iterator iter( Json::Value::const_iterator it, std::size_t index ) const;

        std::string path_;
        Json::Value const* value_;
	};

	namespace detail {

        struct PropertyNodeTransformer
        {
            explicit PropertyNodeTransformer( std::string const& path );

            PropertyNode operator()( boost::tuple< Json::Value const&, std::size_t const& > const& value ) const;

        private:
            std::string const* path_;
        };

        template<>
		struct PropertyConverter< std::string >
		{
			static std::string convert( PropertyNode const& node );
		};

		template<>
		struct PropertyConverter< bool >
		{
			static bool convert( PropertyNode const& node );
		};

        template< typename Type >
        struct PropertyConverter< Type, EnableIf< IsIntegral< Type >() > >
        {
            static Type convert( PropertyNode const& node ) { return (Type) node.value_->asInt64(); }
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

	} // namespace detail

    /**
     * class PropertyList
     */

    namespace detail {

        template< typename Type >
        struct PropertyListTransformer
        {
            Type operator()( PropertyNode const& node ) const { return node.as< Type >(); }
        };

    } // namespace detail

    template< typename Type >
    class PropertyList
    {
    public:
        using const_iterator = boost::transform_iterator<
                detail::PropertyListTransformer< Type >, PropertyNode::const_iterator, boost::use_default, boost::use_default >;

        explicit PropertyList( PropertyNode const& node )
            : node_( &node ) {}

        const_iterator begin() const { return iter( node_->begin() ); }
        const_iterator end() const { return iter( node_->end() ); }

    private:
        const_iterator iter( PropertyNode::const_iterator const& it ) const
        {
            return { it, detail::PropertyListTransformer< Type >() };
        }

        PropertyNode const* node_;
    };

    namespace detail {

        template< typename Type >
        struct PropertyConverter< Type[] >
        {
            static PropertyList< Type > convert( PropertyNode const& node )
            {
                return PropertyList< Type > { node };
            }
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
		Json::Value value_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_PROPERTIES_HPP
