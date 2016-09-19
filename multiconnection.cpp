#include <algorithm>
#include <utility>

#include "manager.hpp"
#include "multiconnection.hpp"
#include "properties.hpp"
#include "utility.hpp"

using namespace std;

namespace sc {

    static PropertyKey const inputsProperty( "inputs" );

    MultiConnection::MultiConnection( string&& id, Manager& manager, PropertyNode const& properties )
            : Component( move( id ) )
            , Output( manager, properties[ inputsProperty ], multipleInputs )
            , channels_(
                    ( *max_element(
                            inputs().begin(), inputs().end(),
                            []( auto const& a, auto const& b ) { return a->emitsChannels() < b->emitsChannels(); } )
                    )->emitsChannels() )
            , values_( channels_ )
    {
    }

    void MultiConnection::set( Input const& input, ChannelBuffer const& values )
    {
        buffers_[ input.id() ] = values;

        values_.clear();
        for_each( buffers_.cbegin(), buffers_.cend(),
                  [this]( auto const& entry ) {
                      transform( entry.second.cbegin(), entry.second.cend(), values_.begin(), values_.begin(),
                                 []( auto const& a, auto const& b ) {
                                     return ChannelValue( max( a.get(), b.get() ) );
                                 } );
                  } );
        inputChangeEvent_( values_ );
    }

    void MultiConnection::doStatistics( ostream& os ) const
    {
        os << ", channels: " << channels_ << ", buffers: " << buffers_.size()
           << "\n\t\tvalues: " << makeStatistics( values_ );
        transform( buffers_.begin(), buffers_.end(), OutputStreamIterator( os ),
                   []( auto const& entry ) {
                       return make_tuple( "\n\t\tbuffer[", ref( entry.first ), "]: ", makeStatistics( entry.second ) );
                   } );
    }

    static ComponentRegistry< MultiConnection > registry( "multiconnection" );

} // namespace sc
