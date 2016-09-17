#include <algorithm>
#include <iterator>
#include <ostream>

#include "utility.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

    static constexpr size_t statisticsOutputCount = 3;

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

	ChannelBuffer::ChannelBuffer() = default;

	ChannelBuffer::ChannelBuffer( initializer_list< ChannelValue > initializer )
		: values_( initializer.begin(), initializer.end() )
	{
	}

	ChannelBuffer::ChannelBuffer( size_t size )
		: values_( size )
	{
	}

	void ChannelBuffer::resize( size_t size )
	{
		values_.resize( size );
	}

    void StatisticsWriter< ChannelValue >::operator()( ostream& os, ChannelValue const& value )
    {
        os << value.get();
    }

    void StatisticsWriter< ChannelBuffer >::operator()( std::ostream& os, ChannelBuffer const& values )
    {
        os << "[";
        transform(
                values.begin(), next( values.begin(), min( values.size(), statisticsOutputCount ) ),
                ostream_iterator< Statistics< ChannelValue > >( os, ", " ),
                []( auto const& value ) { return makeStatistics( value ); } );
        if ( values.size() > statisticsOutputCount ) {
            os << ", ...";
        }
        os << "]";
    }

} // namespace sc
