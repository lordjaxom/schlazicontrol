#ifndef SCHLAZICONTROL_PROPERTIES_HPP
#define SCHLAZICONTROL_PROPERTIES_HPP

#include <chrono>
#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <string>

#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/lexical_cast.hpp>
#include <json/value.h>

#include "typeinfo.hpp"
#include "utility_graphics.hpp"
#include "utility_string.hpp"

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

    namespace detail {
        template< typename Type, typename Enable = void > struct PropertyConverter;
        struct PropertyNodeIterator;
    } // namespace detail

	class PropertyNode
	{
        template< typename Type, typename Enable > friend struct detail::PropertyConverter;

    public:
        using const_iterator = detail::PropertyNodeIterator;

        PropertyNode();
        PropertyNode( std::string path, Json::Value const& value );

        explicit operator bool() const { return !value_->isNull(); }

        std::string const& path() const { return path_; }
        char const* typeName() const;

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

        bool has( std::string const& key ) const;

        PropertyNode operator[]( std::string const& key ) const;
        PropertyNode operator[]( PropertyKey const& key ) const;

    private:
        const_iterator iter( Json::Value::const_iterator it ) const;

        PropertyNode get( std::string const& key, Json::Value const& defaultValue = {} ) const;

        std::string path_;
        Json::Value const* value_;
	};

	namespace detail {

        struct PropertyNodeIterator
                : boost::iterator_adaptor<
                        PropertyNodeIterator,
                        Json::Value::const_iterator,
                        PropertyNode,
                        boost::use_default,
                        PropertyNode >
        {
            friend class boost::iterator_core_access;

            PropertyNodeIterator(
                    std::string const& path, Json::Value::const_iterator first, Json::Value::const_iterator it );

        private:
            reference dereference() const;

            std::string const* path_;
            Json::Value::const_iterator first_;
        };

	} // namespace detail

    /**
     * class PropertyList
     */

    namespace detail {
        template< typename Type > struct PropertyListIterator;
    } // namespace detail

    template< typename Type >
    class PropertyList
    {
    public:
        using const_iterator = detail::PropertyListIterator< Type >;

        explicit PropertyList( PropertyNode&& node )
            : node_( std::move( node ) )
        {
        }

        const_iterator cbegin() const { return const_iterator( node_.begin() ); }
        const_iterator cend() const { return const_iterator( node_.end() ); }
        const_iterator begin() const { return cbegin(); }
        const_iterator end() const { return cend(); }

    private:
        PropertyNode node_;
    };

    namespace detail {

        template< typename Type >
        struct PropertyListIterator
                : boost::iterator_adaptor<
                        PropertyListIterator< Type >,
                        PropertyNode::const_iterator,
                        Type,
                        boost::use_default,
                        Type >
        {
            friend class boost::iterator_core_access;

            explicit PropertyListIterator( PropertyNode::const_iterator it )
                    : PropertyListIterator::iterator_adaptor_( it )
            {
            }

        private:
            typename PropertyListIterator::reference dereference() const
            {
                return this->base()->template as< Type >();
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

    /**
     * property conversion
     */

    namespace detail {

        std::runtime_error invalidType( PropertyNode const& node, char const* expectedType );

        template< typename Type >
        bool propertyIsInRange( Json::LargestInt value, std::enable_if_t< IsSigned< Type >() >* = nullptr )
        {
            return value >= std::numeric_limits< Type >::min() && value <= std::numeric_limits< Type >::max();
        }

        template< typename Type >
        bool propertyIsInRange( Json::LargestUInt value, std::enable_if_t< IsSigned< Type >() >* = nullptr )
        {
            return value <= (Json::LargestUInt) std::numeric_limits< Type >::max();
        }

        template< typename Type >
        bool propertyIsInRange( Json::LargestInt value, std::enable_if_t< IsUnsigned< Type >() >* = nullptr )
        {
            return value >= 0;
        }

        template< typename Type >
        bool propertyIsInRange( Json::LargestUInt value, std::enable_if_t< IsUnsigned< Type >() >* = nullptr )
        {
            return value <= std::numeric_limits< Type >::max();
        }

        template< typename Type, typename Value >
        Type propertyCheckRange( PropertyNode const& node, Value value )
        {
            if ( !propertyIsInRange< Type >( value ) ) {
                throw std::runtime_error( str( "property at ", node.path(), " outside allowed range" ) );
            }
            return (Type) value;
        }

        template< typename Derived, typename Type >
        struct PropertyCheckingConverter
        {
            using BaseType = PropertyCheckingConverter< Derived, Type >;

            static Type convert( PropertyNode const& node )
            {
                if ( !Derived::is( node ) ) {
                    throw invalidType( node, typeName< Type >() );
                }
                return Derived::unckeckedConvert( node );
            }
        };

        template< typename Type, typename Enable >
        struct PropertyConverter
        {
            static bool is( PropertyNode const& node );
            static Type convert( PropertyNode const& node );
        };

        template<>
        struct PropertyConverter< std::string >
                : PropertyCheckingConverter< PropertyConverter< std::string >, std::string >
        {
            using BaseType::convert;
            static bool is( PropertyNode const& node );
            static std::string unckeckedConvert( PropertyNode const& node );
        };

        template<>
        struct PropertyConverter< bool >
                : PropertyCheckingConverter< PropertyConverter< bool >, bool >
        {
            using BaseType::convert;
            static bool is( PropertyNode const& node );
            static bool unckeckedConvert( PropertyNode const& node );
        };

        template< typename Type >
        struct PropertyConverter< Type, std::enable_if_t< IsIntegral< Type >() > >
        {
            static bool is( PropertyNode const& node )
            {
                Json::Value const& value = *node.value_;
                return ( value.type() == Json::intValue && propertyIsInRange< Type >( value.asLargestInt() ) ) ||
                       ( value.type() == Json::uintValue && propertyIsInRange< Type >( value.asLargestUInt() ) );
            }

            static Type convert( PropertyNode const& node )
            {
                Json::Value const& value = *node.value_;
                if ( value.type() == Json::intValue ) {
                    return propertyCheckRange< Type >( node, value.asLargestInt() );
                }
                if ( value.type() == Json::uintValue ) {
                    return propertyCheckRange< Type >( node, value.asLargestUInt() );
                }
                throw invalidType( node, typeName< Type >() );
            }
        };

        template< typename Type >
        struct PropertyConverter< Type, std::enable_if_t< IsFloatingPoint< Type >() > >
        {
            static bool is( PropertyNode const& node )
            {
                Json::Value const& value = *node.value_;
                return value.type() == Json::realValue;
            }

            static Type convert( PropertyNode const& node )
            {
                Json::Value const& value = *node.value_;
                if ( value.type() == Json::realValue ) {
                    return value.asDouble();
                }
                throw invalidType( node, typeName< Type >() );
            }
        };

        template<>
        struct PropertyConverter< std::chrono::nanoseconds >
        {
            static bool is( PropertyNode const& node );
            static std::chrono::nanoseconds convert( PropertyNode const& node );
        };

        template< typename Rep, typename Period >
        struct PropertyConverter< std::chrono::duration< Rep, Period > >
                : PropertyConverter< std::chrono::nanoseconds >
        {
            using BaseType = PropertyConverter< std::chrono::nanoseconds >;
            using BaseType::is;

            static std::chrono::duration< Rep, Period > convert( PropertyNode const& node )
            {
                return std::chrono::duration_cast< std::chrono::duration< Rep, Period > >( BaseType::convert( node ) );
            };
        };

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

        template< typename Type >
        struct PropertyConverter< Type[] >
        {
            static bool is( PropertyNode const& node )
            {
                return node.value_->isArray();
            }

            static PropertyList< Type > convert( PropertyNode node )
            {
                if ( !is( node ) ) {
                    throw invalidType( node, "array" );
                }
                return PropertyList< Type >( std::move( node ) );
            }
        };

    } // namespace detail

} // namespace sc

#endif // SCHLAZICONTROL_PROPERTIES_HPP
