#include <utility>

#include "manager.hpp"
#include "multiconnection.hpp"
#include "properties.hpp"

using namespace std;

namespace sc {

    static PropertyKey const inputsProperty( "inputs" );

    MultiConnection::MultiConnection( string&& id, Manager& manager, PropertyNode const& properties )
            : Component( move( id ) )
            , Output( manager, properties[ inputsProperty ], multiInput )
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
        for_each(
                buffers_.begin(), buffers_.end(),
                [this]( auto const& entry ) {
                    transform(
                            entry.second.begin(), entry.second.end(), values_.begin(), values_.begin(),
                            []( auto const& a, auto const& b ) { return ChannelValue( max( a.get(), b.get() ) ); } );
                } );
        inputChangeEvent_( values_ );
    }

    void MultiConnection::statistics( std::ostream& os ) const
    {
        os << "channels: " << channels_ << "; "
           << "buffers: " << buffers_.size() << "; "
           << "values: " << makeStatistics( values_ );
    }

    static ComponentRegistry< MultiConnection > registry( "multiconnection" );

} // namespace sc
