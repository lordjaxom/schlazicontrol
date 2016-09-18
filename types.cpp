#include <algorithm>
#include <ostream>

#include "utility.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

    static constexpr size_t maxOutputCount = 3;

    /**
     * class ChannelValue
     */

	ChannelValue::ChannelValue()
		: value_()
	{
	}

	ChannelValue::ChannelValue( double value, double min, double max )
		: ChannelValue( sc::scale( value, min, max, minimum, maximum ) )
	{
	}

	ChannelValue::ChannelValue( bool value )
		: ChannelValue( value ? maximum : minimum )
	{
	}

    void ChannelValue::statistics( ostream& os ) const
    {
        os << value_;
    }

    /**
     * class ChannelBuffer
     */

	ChannelBuffer::ChannelBuffer() = default;

	ChannelBuffer::ChannelBuffer( initializer_list< ChannelValue > initializer )
		: values_( initializer.begin(), initializer.end() )
	{
        allocate< ChannelValue >( values_.size() );
	}

	ChannelBuffer::ChannelBuffer( size_t size )
		: values_( size )
	{
        allocate< ChannelValue >( values_.size() );
	}

	void ChannelBuffer::resize( size_t size )
	{
        deallocate< ChannelValue >( values_.size() );
		values_.resize( size );
        allocate< ChannelValue >( values_.size() );
	}

    void ChannelBuffer::shift( size_t offset )
    {
        values_.insert( values_.begin(), offset, {} );
        allocate< ChannelValue >( offset );
    }

    void ChannelBuffer::clear()
    {
        fill( values_.begin(), values_.end(), ChannelValue() );
    }

    void ChannelBuffer::statistics( ostream& os ) const
    {
        os << "{size: " << values_.size() << ", " << makeStatistics( trackable() ) << ", values: [";
        transform( values_.begin(), next( values_.begin(), min( values_.size(), maxOutputCount ) ),
                   OutputStreamIterator( os, ", " ), []( auto const& value ) { return makeStatistics( value ); } );
        if ( values_.size() > maxOutputCount ) {
            os << ", ...";
        }
        os << "]}";
    }

} // namespace sc
