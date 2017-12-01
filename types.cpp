#include <cassert>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <ostream>

#include "logging.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

    static Logger logger( "types" );

    static constexpr size_t maxOutputCount = 3;


    /**
     * class ChannelValue
     */

    ChannelValue::operator bool() const
    {
        return abs( value_ ) > 0.0;
    }

    bool ChannelValue::operator==( ChannelValue const& other ) const
    {
        return abs( value_ - other.value_ ) < epsilon;
    }

    bool ChannelValue::operator<( ChannelValue const& other ) const
    {
        return value_ - other.value_ < -epsilon;
    }

    /**
     * class ChannelBuffer
     */

    constexpr ChannelValue ChannelBuffer::emptyValue_;

	ChannelBuffer::ChannelBuffer()
            : offset_()
            , repeat_()
    {
    }

	ChannelBuffer::ChannelBuffer( initializer_list< ChannelValue > initializer )
            : offset_()
            , repeat_( 1 )
            , values_( initializer.begin(), initializer.end() )
	{
        allocate< ChannelValue >( values_.size() );
	}

    ChannelBuffer::ChannelBuffer( ChannelValue const& value )
            : offset_()
            , repeat_( 1 )
            , values_ { value }
    {
    }

	ChannelBuffer::ChannelBuffer( size_t size )
            : offset_( size )
            , repeat_( 1 )
	{
	}

    void ChannelBuffer::shift( size_t offset )
    {
        offset_ += offset;
    }

    void ChannelBuffer::multiply( size_t repeat )
    {
        assert( repeat > 1 );

        if ( values_.empty() ) {
            offset_ *= repeat;
        }
        else if ( offset_ == 0 ) {
            repeat_ *= repeat;
        }
        else {
            throw std::invalid_argument( "not implemented yet" );
        }
    }

    void ChannelBuffer::resize( size_t size )
    {
        offset_ = size;
        repeat_ = 1;
        vector< ChannelValue >().swap( values_ );
    }

    void ChannelBuffer::fill( ChannelValue const& value )
    {
        std::fill( values_.begin(), values_.end(), value );
    }

    ChannelValue& ChannelBuffer::operator[]( size_t index )
    {
        if ( index < offset_ || repeat_ > 1 ) {
            expand( index );
        }
        return values_[ index - offset_ ];
    }

    ChannelValue const& ChannelBuffer::operator[]( size_t index ) const
    {
        if ( index < offset_ ) {
            return emptyValue_;
        }
        return values_[ ( index - offset_ ) % values_.size() ];
    }

    ChannelBuffer::iterator ChannelBuffer::begin()
    {
        expand();
        return values_.begin();
    }

    ChannelBuffer::iterator ChannelBuffer::end()
    {
        expand();
        return values_.end();
    }

    ChannelBuffer::const_iterator ChannelBuffer::cbegin() const
    {
        return ChannelBuffer::const_iterator( *this );
    }

    ChannelBuffer::const_iterator ChannelBuffer::cend() const
    {
        return ChannelBuffer::const_iterator( *this, ChannelBuffer::const_iterator::endIterator );
    }

    void ChannelBuffer::expand( size_t index )
    {
        size_t r = repeat_, o = offset_, s = values_.size();
        bool d = repeat_ > 1 || offset_ > index;
        if ( repeat_ > 1 ) {
            size_t oldSize = values_.size();
            values_.reserve( oldSize * repeat_ );
            for ( size_t i = 1 ; i < repeat_ ; ++i ) {
                copy_n( values_.begin(), oldSize, back_inserter( values_ ) );
            }
            allocate< ChannelValue >( values_.size() - oldSize );
            repeat_ = 1;
        }
        if ( offset_ > index ) {
            size_t fillSize = offset_ - index;
            values_.insert( values_.begin(), fillSize, emptyValue_ );
            allocate< ChannelValue >( fillSize );
            offset_ = index;
        }
        if ( d ) {
            logger.debug( "channel buffer expanded: from {offset: ", o, ", repeat: ", r, ", size: ", s,
                          "} to {offset: ", offset_, ", repeat: ", repeat_, ", size: ", values_.size(), "}" );
        }
    }

    void ChannelBuffer::statistics( ostream& os ) const
    {
        os << "{size: " << size() << ", offset: " << offset_ << ", repeat: " << repeat_ << ", values: "
           << values_.size() << ", " << makeStatistics( trackable() ) << ", content: [";
        if ( offset_ > 0 ) {
            os << "..." << ( !values_.empty() ? ", " : "" );
        }
        transform( values_.begin(), next( values_.begin(), min( values_.size(), maxOutputCount ) ),
                   OutputStreamIterator( os, ", " ), []( auto const& value ) { return value.get(); } );
        if ( values_.size() > maxOutputCount ) {
            os << ", ...";
        }
        os << "]}";
    }

    namespace detail {

        ChannelBufferIterator::ChannelBufferIterator( ChannelBuffer const& values )
                : owner_( &values )
                , offsetIt_()
                , repeatIt_()
                , valuesIt_( owner_->values_.begin() )
                , valuesLast_( owner_->values_.end() )
        {
        }

        ChannelBufferIterator::ChannelBufferIterator( ChannelBuffer const& owner, ChannelBufferIterator::EndIteratorTag )
                : owner_( &owner )
                , offsetIt_( owner_->offset_ )
                , repeatIt_( owner_->repeat_ )
                , valuesIt_( owner_->values_.end() )
                , valuesLast_( owner_->values_.end() )
        {
        }

        void ChannelBufferIterator::increment()
        {
            if ( offsetIt_ != owner_->offset_ ) {
                ++offsetIt_;
            }
            else if ( valuesIt_ != valuesLast_ &&
                    ++valuesIt_ == valuesLast_ &&
                    ++repeatIt_ != owner_->repeat_ ) {
                valuesIt_ = owner_->values_.begin();
            }
        }

        bool ChannelBufferIterator::equal( ChannelBufferIterator const& other ) const
        {
            return owner_ == other.owner_ &&
                    offsetIt_ == other.offsetIt_ &&
                    repeatIt_ == other.repeatIt_ &&
                    valuesIt_ == other.valuesIt_;
        }

        ChannelValue const& ChannelBufferIterator::dereference() const
        {
            return offsetIt_ != owner_->offset_ ? ChannelBuffer::emptyValue_ : *valuesIt_;
        }

    } // namespace detail

} // namespace sc
