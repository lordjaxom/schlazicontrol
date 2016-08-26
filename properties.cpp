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

	PropertyKey::PropertyKey( std::string name, std::string defaultValue )
		: name_( move( name ) )
		, defaultValue_( move( defaultValue ) )
	{
	}

    namespace detail {

        PropertyNodeTransformer::PropertyNodeTransformer( std::string const& path )
            : path_( &path )
        {

        }

        PropertyNode PropertyNodeTransformer::operator()(
                boost::tuple< Json::Value const&, size_t const& > const& value ) const
        {
            return { str( *path_, '[', get< 1 >( value ), ']' ), get< 0 >( value ) };
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

    /**
     * class PropertyNode
     */

    PropertyNode::PropertyNode() = default;

    PropertyNode::PropertyNode( string path, Json::Value const& value )
		: path_( move( path ) )
        , value_( &value )
	{
	}

    PropertyNode PropertyNode::get( string const& key, std::string const& defaultValue ) const
    {
        auto const& value = ( *value_ )[ key ];
        string path = str( path_, "/", key );
        if ( defaultValue.empty() && value.isNull() ) {
            throw runtime_error( str( "property ", path, " not found" ) );
        }
        return PropertyNode( move( path ), value );
    }

    Properties::Properties( string const& propertiesFile )
		: PropertyNode( "", value_ )
		, value_( readJsonFile( propertiesFile ) )
	{
	}

} // namespace sc
