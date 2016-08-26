#include "utility.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

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

} // namespace sc
