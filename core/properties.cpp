#include <fstream>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <system_error>

#include "expression.hpp"
#include "core/logging.hpp"
#include "properties.hpp"
#include "utility_string.hpp"

using namespace std;
using namespace nlohmann;

namespace sc {

	static Logger logger( "properties" );

    namespace detail {

        json readJsonFile( string const& fileName )
        {
            ifstream ifs( fileName, ios::in );
            if ( !ifs ) {
                throw system_error( errno, system_category(), "couldn't open " + fileName );
            }

            json props;
            ifs >> props;
            return move( props );
        }

    } // namespace detail

    /**
     * class PropertyKey
     */

	PropertyKey::PropertyKey( std::string name, nlohmann::json defaultValue )
		: name_( move( name ) )
		, defaultValue_( move( defaultValue ) ) {}

    /**
     * class PropertyNode
     */

    namespace detail {

        char const* to_string( json::value_t type )
        {
            switch ( type ) {
                case json::value_t::null: return "null";
                case json::value_t::number_integer:
                case json::value_t::number_unsigned: return "number";
                case json::value_t::number_float: return "decimal";
                case json::value_t::string: return "string";
                case json::value_t::boolean: return "boolean";
                case json::value_t::array: return "array";
                case json::value_t::object: return "object";
                case json::value_t::discarded: break;
            }
            throw invalid_argument( "invalid enum constant for json::value_t" );
        }

        inline void assertValueType( string const& path, json::value_t type, json::value_t expected )
        {
            if ( type != expected ) {
                throw runtime_error( str( "expected property \"", path, "\" to be of type ", to_string( expected ),
                                          " but was ", to_string( type ) ) );
            }
        }

        void propertyConversionError( string const& path, char const* typeName, json::exception const& e )
        {
            logger.error( "error converting property ", path, " to ", typeName, ": ", e.what() );
            throw std::runtime_error( str( "property at ", path, " could not be converted to ", typeName ) );
        }

    } // namespace detail

    PropertyNode::PropertyNode() = default;

    PropertyNode::PropertyNode( string path, nlohmann::json const& value )
            : path_( move( path ) )
            , value_( &value )
    {
    }

    char const* PropertyNode::typeName() const
    {
        return detail::to_string( value_->type() );
    }

    PropertyNode::const_iterator PropertyNode::begin() const
    {
        return iter( value_->begin() );
    }

    PropertyNode::const_iterator PropertyNode::end() const
    {
        return iter( value_->end() );
    }

    bool PropertyNode::has( string const& key ) const
    {
        return value_->find( key ) != value_->end();
    }

    PropertyNode PropertyNode::operator[]( string const& key ) const
    {
        return get( key, {} );
    }

    PropertyNode PropertyNode::operator[]( PropertyKey const& key ) const
    {
        return get( key.name(), key.defaultValue() );
    }

    PropertyNode::const_iterator PropertyNode::iter( json::const_iterator it ) const
    {
        detail::assertValueType( path_, value_->type(), json::value_t::array );
        return { path_, value_->begin(), it };
    }

    PropertyNode PropertyNode::get( string const& key, nlohmann::json const& defaultValue ) const
    {
        detail::assertValueType( path_, value_->type(), json::value_t::object );

        auto stored = value_->find( key );
        auto const& value = stored != value_->end() ? *stored : defaultValue;
        if ( value.is_null() ) {
            throw runtime_error( str( "required property \"", path_, "/", key, "\" not found" ) );
        }
        return PropertyNode( str( path_, "/", key ), value );
    }

    namespace detail {

        PropertyNodeIterator::PropertyNodeIterator(
                string const& path, json::const_iterator first, json::const_iterator it )
                : PropertyNodeIterator::iterator_adaptor_( it )
                , path_( &path )
                , first_( first )
        {
        }

        PropertyNodeIterator::reference PropertyNodeIterator::dereference() const
        {
            auto const& it = base_reference();
            return { str( *path_, '[', distance( first_, it ), ']' ), *it };
        }

    } // namespace detail

    /**
     * class PropertyList
     */

    Properties::Properties( string const& propertiesFile )
		: PropertyNode( "", value_ )
		, value_( detail::readJsonFile( propertiesFile ) )
	{
	}

    /**
     * property conversion
     */

    namespace detail {
/*
        bool PropertyConverter< Rgb >::is( PropertyNode const& node )
        {
            if ( !node.value_->is_string() ) {
                return false;
            }

            string digits = "0123456789abcdef";
            string const& value = *node.value_;
            return value.length() == 6 && value.find_first_not_of( "0123456789abcdef" ) == string::npos;
        }

        Rgb PropertyConverter< Rgb >::convert( PropertyNode const& node )
        {
            if ( !is( node ) ) {
                throw invalidType( node, "rgb color string" );
            }
            return Rgb( stoul( node.value_->get< string >(), 0, 16 ) );
        }
*/
    } // namespace detail

} // namespace sc

namespace nlohmann {

    void adl_serializer< chrono::nanoseconds >::from_json( json const& src, chrono::nanoseconds& dst )
    {
        dst = sc::expression::parseDuration( src );
    }

} // namespace nlohmann