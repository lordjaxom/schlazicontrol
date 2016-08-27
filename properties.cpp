#include <fstream>
#include <iostream>
#include <stdexcept>
#include <system_error>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/find_iterator.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <json/reader.h>

#include "logging.hpp"
#include "properties.hpp"
#include "utility.hpp"

using namespace std;
using namespace boost::algorithm;

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

        string PropertyConverter< string >::convert( PropertyNode const& node )
        {
            return node.value_->asString();
        }

        bool PropertyConverter< bool >::convert( PropertyNode const& node )
        {
            return node.value_->asBool();
        }

    } // namespace detail

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

    static void assertValueType(
            string const& path, Json::ValueType type, initializer_list< Json::ValueType > allowedTypes )
    {
        if ( allowedTypes.begin() != allowedTypes.end() &&
                find( allowedTypes.begin(), allowedTypes.end(), type ) == allowedTypes.end() ) {
            throw runtime_error( str(
                    "expected property \"", path, "\" to be of type(s) ", join( ", ", allowedTypes ),
                    " but was ", valueType( type ) ) );
        }
    }

    PropertyNode::PropertyNode() = default;

    PropertyNode::PropertyNode( string path, Json::Value const& value )
		: path_( move( path ) )
        , value_( &value )
	{
	}

    PropertyNode::const_iterator PropertyNode::begin() const
    {
        return { path_, value_->begin(), value_->begin() };
    }

    PropertyNode::const_iterator PropertyNode::end() const
    {
        return { path_, value_->begin(), value_->end() };
    }

    PropertyNode PropertyNode::operator[]( string const& key ) const
    {
        assertValueType( path_, value_->type(), { Json::objectValue } );
        return get( key, {} );
    }

    PropertyNode PropertyNode::operator[]( PropertyKey const& key ) const
    {
        assertValueType( path_, value_->type(), { Json::objectValue } );
        return get( key.name(), key.defaultValue() );
    }

    PropertyNode PropertyNode::get(
            string const& key, Json::Value const& defaultValue, initializer_list< Json::ValueType > allowedTypes ) const
    {
        string path = str( path_, "/", key );
        auto const& value = ( *value_ )[ key ];
        if ( value.isNull() ) {
            if ( defaultValue.isNull()) {
                throw runtime_error( str( "mandatory property \"", path, "\" does not exist" ));
            }
            return PropertyNode( move( path ), defaultValue );
        }
        assertValueType( path, value.type(), allowedTypes );
        return PropertyNode( move( path ), value );
    }

    /**
     * class PropertyList
     */

    Properties::Properties( string const& propertiesFile )
		: PropertyNode( "", value_ )
		, value_( readJsonFile( propertiesFile ) )
	{
	}

} // namespace sc
