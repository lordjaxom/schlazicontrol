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
        template< typename Type, typename Enable = void > struct PropertyConverter;
        struct PropertyNodeIterator;
    } // namespace detail

	class PropertyNode
	{
        template< typename Type, typename Enable > friend struct detail::PropertyConverter;

    public:
        using const_iterator = detail::PropertyNodeIterator;

        PropertyNode();
        PropertyNode( std::string path, nlohmann::json const& value );

        explicit operator bool() const { return !value_->is_null(); }

        std::string const& path() const { return path_; }
        string_view typeName() const;

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
		nlohmann::json value_;
	};

    /**
     * property conversion
     */

    namespace detail {

        std::runtime_error invalidType( PropertyNode const& node, char const* expectedType );

        template< typename Type >
        bool propertyIsInRange( std::intmax_t value, std::enable_if_t< std::is_signed< Type >::value >* = nullptr )
        {
            return value >= std::numeric_limits< Type >::min() && value <= std::numeric_limits< Type >::max();
        }

        template< typename Type >
        bool propertyIsInRange( std::uintmax_t value, std::enable_if_t< std::is_signed< Type >::value >* = nullptr )
        {
            return value <= static_cast< std::uintmax_t >( std::numeric_limits< Type >::max() );
        }

        template< typename Type >
        bool propertyIsInRange( std::intmax_t value, std::enable_if_t< std::is_unsigned< Type >::value >* = nullptr )
        {
            return value >= 0;
        }

        template< typename Type >
        bool propertyIsInRange( std::uintmax_t value, std::enable_if_t< std::is_unsigned< Type >::value >* = nullptr )
        {
            return value <= std::numeric_limits< Type >::max();
        }

        template< typename Type, typename Value >
        Type propertyCheckRange( PropertyNode const& node, Value value )
        {
            if ( !propertyIsInRange< Type >( value ) ) {
                throw std::runtime_error( str( "property at ", node.path(), " outside allowed range" ) );
            }
            return static_cast< Type >( value );
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
        struct PropertyConverter< Type, std::enable_if_t< std::is_integral< Type >::value > >
        {
            static bool is( PropertyNode const& node )
            {
                nlohmann::json const& value = *node.value_;
                return ( value.type() == nlohmann::json::value_t::number_integer && propertyIsInRange< Type >( value.get< std::intmax_t >() ) ) ||
                       ( value.type() == nlohmann::json::value_t::number_unsigned && propertyIsInRange< Type >( value.get< std::uintmax_t >() ) );
            }

            static Type convert( PropertyNode const& node )
            {
                nlohmann::json const& value = *node.value_;
                if ( value.type() == nlohmann::json::value_t::number_integer ) {
                    return propertyCheckRange< Type >( node, value.get< std::intmax_t >() );
                }
                if ( value.type() == nlohmann::json::value_t::number_unsigned ) {
                    return propertyCheckRange< Type >( node, value.get< std::uintmax_t >() );
                }
                throw invalidType( node, typeName< Type >() );
            }
        };

        template< typename Type >
        struct PropertyConverter< Type, std::enable_if_t< std::is_floating_point< Type >::value > >
        {
            static bool is( PropertyNode const& node )
            {
                nlohmann::json const& value = *node.value_;
                return value.is_number_float();
            }

            static Type convert( PropertyNode const& node )
            {
                nlohmann::json const& value = *node.value_;
                if ( value.is_number_float() ) {
                    return value.get< double >();
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
                return node.value_->is_array();
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
