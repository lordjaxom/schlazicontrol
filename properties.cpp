#include <fstream>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <system_error>

#include <json/reader.h>

#include "expression.hpp"
#include "logging.hpp"
#include "properties.hpp"
#include "utility_string.hpp"

using namespace std;

namespace sc {

	static Logger logger( "properties" );

    static Json::Value readJsonFile( string const& fileName )
    {
        ifstream ifs( fileName, ios::in );
        if ( !ifs ) {
            throw system_error( errno, system_category(), str( "couldn't open ", fileName ) );
        }

        Json::CharReaderBuilder builder;
        builder[ "allowComments" ] = true;
        builder[ "collectComments" ] = false;
        builder[ "allowSingleQuotes" ] = true;

        Json::Value result;
        string errors;
        if ( !Json::parseFromStream( builder, ifs, &result, &errors ) ) {
            throw runtime_error( str( "couldn't parse ", fileName, ": ", errors ) );
        }
        return result;
    }

    /**
     * class PropertyKey
     */

	PropertyKey::PropertyKey( std::string name, Json::Value defaultValue )
		: name_( move( name ) )
		, defaultValue_( move( defaultValue ) )
	{
	}

    /**
     * class PropertyNode
     */

    static char const* valueType( Json::ValueType type )
    {
        switch ( type ) {
            case Json::nullValue: return "null";
            case Json::intValue:
            case Json::uintValue: return "number";
            case Json::realValue: return "decimal";
            case Json::stringValue: return "string";
            case Json::booleanValue: return "boolean";
            case Json::arrayValue: return "array";
            case Json::objectValue: return "object";
        }
        throw invalid_argument( "invalid enum constant for Json::ValueType" );
    }

    static void assertValueType( string const& path, Json::ValueType type, Json::ValueType expected )
    {
        if ( type != expected ) {
            throw runtime_error( str( "expected property \"", path, "\" to be of type ", valueType( expected ),
                                      " but was ", valueType( type ) ) );
        }
    }

    PropertyNode::PropertyNode() = default;

    PropertyNode::PropertyNode( string path, Json::Value const& value )
            : path_( move( path ) )
            , value_( &value )
    {
    }

    char const* PropertyNode::typeName() const
    {
        return valueType( value_->type() );
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
        return !( *value_ )[ key ].isNull();
    }

    PropertyNode PropertyNode::operator[]( string const& key ) const
    {
        return get( key, {} );
    }

    PropertyNode PropertyNode::operator[]( PropertyKey const& key ) const
    {
        return get( key.name(), key.defaultValue() );
    }

    PropertyNode::const_iterator PropertyNode::iter( Json::Value::const_iterator it ) const
    {
        assertValueType( path_, value_->type(), Json::arrayValue );
        return { path_, value_->begin(), it };
    }

    PropertyNode PropertyNode::get( string const& key, Json::Value const& defaultValue ) const
    {
        assertValueType( path_, value_->type(), Json::objectValue );

        string path = str( path_, "/", key );
        auto const& stored = ( *value_ )[ key ];
        auto const& value = !stored.isNull() ? stored : defaultValue;
        if ( value.isNull() ) {
            throw runtime_error( str( "required property \"", path, "\" not found" ) );
        }
        return PropertyNode( move( path ), value );
    }

    namespace detail {

        PropertyNodeIterator::PropertyNodeIterator(
                string const& path, Json::Value::const_iterator first, Json::Value::const_iterator it )
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
		, value_( readJsonFile( propertiesFile ) )
	{
	}

    /**
     * property conversion
     */

    namespace detail {

        runtime_error invalidType( PropertyNode const& node, char const* expectedType )
        {
            return std::runtime_error( str(
                    "property at ", node.path(), " expected to be of type ", expectedType, " but was ",
                    node.typeName() ) );
        }

        bool PropertyConverter< string >::is( PropertyNode const& node )
        {
            return node.value_->isString();
        }

        string PropertyConverter< string >::unckeckedConvert( PropertyNode const& node )
        {
            return node.value_->asString();
        }

        bool PropertyConverter< bool >::is( PropertyNode const& node )
        {
            return node.value_->isBool();
        }

        bool PropertyConverter< bool >::unckeckedConvert( PropertyNode const& node )
        {
            return node.value_->asBool();
        }

        bool PropertyConverter< chrono::nanoseconds >::is( PropertyNode const& node )
        {
            if ( !node.value_->isString() ) {
                return false;
            }

            try {
                convert( node );
                return true;
            }
            catch ( ... ) {
                return false;
            }
        }

        chrono::nanoseconds PropertyConverter< chrono::nanoseconds >::convert( PropertyNode const& node )
        {
            if ( !node.value_->isString() ) {
                throw invalidType( node, "duration string" );
            }
            return expression::parseDuration( node.value_->asString() );
        }

        bool PropertyConverter< Rgb >::is( PropertyNode const& node )
        {
            if ( !node.value_->isString() ) {
                return false;
            }

            string digits = "0123456789abcdef";
            string value = node.value_->asString();
            return value.length() == 6 && value.find_first_not_of( "0123456789abcdef" ) == string::npos;
        }

        Rgb PropertyConverter< Rgb >::convert( PropertyNode const& node )
        {
            if ( !is( node ) ) {
                throw invalidType( node, "rgb color string" );
            }
            return Rgb( stoul( node.value_->asString(), 0, 16 ) );
        }

    } // namespace detail

} // namespace sc
