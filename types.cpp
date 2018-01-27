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

    namespace detail {

        /**
         * class SmartChannelBuffer
         */

        constexpr ChannelValue SmartChannelBuffer::emptyValue_;

        SmartChannelBuffer::SmartChannelBuffer()
                : offset_()
                , repeat_()
        {
        }

        SmartChannelBuffer::SmartChannelBuffer( initializer_list< ChannelValue > initializer )
                : offset_()
                , repeat_( 1 )
                , values_( initializer.begin(), initializer.end() )
        {
            allocate< ChannelValue >( values_.size() );
        }

        SmartChannelBuffer::SmartChannelBuffer( ChannelValue const& value )
                : offset_()
                , repeat_( 1 )
                , values_ { value }
        {
            allocate< ChannelValue >( values_.size() );
        }

        SmartChannelBuffer::SmartChannelBuffer( size_t size )
                : offset_( size )
                , repeat_( 1 )
        {
            allocate< ChannelValue >( values_.size() );
        }

        SmartChannelBuffer::~SmartChannelBuffer()
        {
            deallocate< ChannelValue >( values_.size() );
        }

        void SmartChannelBuffer::shift( size_t offset )
        {
            offset_ += offset;
        }

        void SmartChannelBuffer::multiply( size_t repeat )
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

        void SmartChannelBuffer::fill( ChannelValue const& value )
        {
            std::fill( values_.begin(), values_.end(), value );
        }

        ChannelValue& SmartChannelBuffer::operator[]( size_t index )
        {
            if ( index < offset_ || repeat_ > 1 ) {
                expand( index );
            }
            return values_[ index - offset_ ];
        }

        ChannelValue const& SmartChannelBuffer::operator[]( size_t index ) const
        {
            if ( index < offset_ ) {
                return emptyValue_;
            }
            return values_[ ( index - offset_ ) % values_.size() ];
        }

        SmartChannelBuffer::iterator SmartChannelBuffer::begin()
        {
            expand();
            return values_.begin();
        }

        SmartChannelBuffer::iterator SmartChannelBuffer::end()
        {
            expand();
            return values_.end();
        }

        SmartChannelBuffer::const_iterator SmartChannelBuffer::cbegin() const
        {
            return SmartChannelBuffer::const_iterator( *this );
        }

        SmartChannelBuffer::const_iterator SmartChannelBuffer::cend() const
        {
            return SmartChannelBuffer::const_iterator( *this, SmartChannelBuffer::const_iterator::endIterator );
        }

        void SmartChannelBuffer::expand( size_t index )
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

        void SmartChannelBuffer::statistics( ostream& os ) const
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

        /**
         * class SmartChannelBufferIterator
         */

        SmartChannelBufferIterator::SmartChannelBufferIterator( SmartChannelBuffer const& values )
                : owner_( &values )
                , offsetIt_()
                , repeatIt_()
                , valuesIt_( owner_->values_.begin() )
                , valuesLast_( owner_->values_.end() )
        {
        }

        SmartChannelBufferIterator::SmartChannelBufferIterator( SmartChannelBuffer const& owner, SmartChannelBufferIterator::EndIteratorTag )
                : owner_( &owner )
                , offsetIt_( owner_->offset_ )
                , repeatIt_( owner_->repeat_ )
                , valuesIt_( owner_->values_.end() )
                , valuesLast_( owner_->values_.end() )
        {
        }

        void SmartChannelBufferIterator::increment()
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

        bool SmartChannelBufferIterator::equal( SmartChannelBufferIterator const& other ) const
        {
            return owner_ == other.owner_ &&
                    offsetIt_ == other.offsetIt_ &&
                    repeatIt_ == other.repeatIt_ &&
                    valuesIt_ == other.valuesIt_;
        }

        ChannelValue const& SmartChannelBufferIterator::dereference() const
        {
            return offsetIt_ != owner_->offset_ ? SmartChannelBuffer::emptyValue_ : *valuesIt_;
        }

        /**
         * class SimpleChannelBuffer
         */

        constexpr ChannelValue SimpleChannelBuffer::emptyValue_;

        SimpleChannelBuffer::SimpleChannelBuffer() = default;

        SimpleChannelBuffer::SimpleChannelBuffer( initializer_list< ChannelValue > initializer )
                : values_( initializer.begin(), initializer.end() )
        {
            allocate< ChannelValue >( values_.size() );
        }

        SimpleChannelBuffer::SimpleChannelBuffer( ChannelValue const& value )
                : values_ { value }
        {
            allocate< ChannelValue >( values_.size() );
        }

        SimpleChannelBuffer::SimpleChannelBuffer( size_t size )
                : values_( size )
        {
            allocate< ChannelValue >( values_.size() );
        }

        SimpleChannelBuffer::~SimpleChannelBuffer()
        {
            deallocate< ChannelValue >( values_.size() );
        }

        void SimpleChannelBuffer::shift( size_t offset )
        {
            values_.insert( values_.begin(), offset, emptyValue_ );
            allocate< ChannelValue >( offset );
        }

        void SimpleChannelBuffer::multiply( size_t repeat )
        {
            assert( repeat > 1 );

            auto s = values_.size();
            values_.reserve( s * repeat );
            while ( --repeat > 0 ) {
                copy_n( values_.begin(), s, back_inserter( values_ ) );
            }
            allocate< ChannelValue >( values_.size() - s );
        }

        void SimpleChannelBuffer::fill( ChannelValue const& value )
        {
            std::fill( values_.begin(), values_.end(), value );
        }

        void SimpleChannelBuffer::statistics( ostream& os ) const
        {
            os << "{size: " << size() << ", content: [";
            transform( values_.begin(), next( values_.begin(), min( values_.size(), maxOutputCount ) ),
                       OutputStreamIterator( os, ", " ), []( auto const& value ) { return value.get(); } );
            if ( values_.size() > maxOutputCount ) {
                os << ", ...";
            }
            os << "]}";
        }

    } // namespace detail

    namespace detail {

        /**
         * class ColorProxy
         */

        ColorProxy::ColorProxy( ChannelBuffer::iterator wrapped )
                : wrapped_( wrapped ) {}

        uint32_t ColorProxy::get() const
        {
            auto it = wrapped_;
            uint8_t r = ColorValue( *it++ ).get();
            uint8_t g = ColorValue( *it++ ).get();
            uint8_t b = ColorValue( *it++ ).get();
            return ( r << 16 ) | ( g << 8 ) | ( b << 0 );
        }

        void ColorProxy::set( std::uint32_t color )
        {
            auto it = wrapped_;
            *it++ = ColorValue( (std::uint8_t) ( ( color >> 16 ) & 0xff ) );
            *it++ = ColorValue( (std::uint8_t) ( ( color >> 8 ) & 0xff ) );
            *it++ = ColorValue( (std::uint8_t) ( ( color >> 0 ) & 0xff ) );
        }

    } // namespace detail

    /**
     * class ColorBuffer
     */

    ColorBuffer::ColorBuffer( ChannelBuffer& wrapped )
            : wrapped_( &wrapped )
    {
        assert( wrapped_->size() % 3 == 0 );
    }

    ColorBuffer::iterator ColorBuffer::begin()
    {
        return ColorBuffer::iterator( *this );
    }

    ColorBuffer::iterator ColorBuffer::end()
    {
        return ColorBuffer::iterator( *this, ColorBuffer::iterator::endIterator );
    }

    ColorBuffer::Proxy ColorBuffer::operator[]( std::size_t index )
    {
        assert( wrapped_->size() % 3 == 0 );
        return Proxy( next( wrapped_->begin(), index * 3 ) );
    }

    namespace detail {

        /**
         * class SmartChannelBufferIterator
         */

        ColorBufferIterator::ColorBufferIterator( ColorBuffer& owner )
                : owner_( &owner )
                , valuesIt_( owner_->wrapped_->begin() )
                , valuesLast_( owner_->wrapped_->end() )
        {
        }

        ColorBufferIterator::ColorBufferIterator( ColorBuffer& owner, ColorBufferIterator::EndIteratorTag )
                : owner_( &owner )
                , valuesIt_( owner_->wrapped_->end() )
                , valuesLast_( owner_->wrapped_->end() )
        {
        }

        void ColorBufferIterator::increment()
        {
            std::advance( valuesIt_, 3 );
        }

        bool ColorBufferIterator::equal( ColorBufferIterator const& other ) const
        {
            return owner_ == other.owner_ &&
                   valuesIt_ == other.valuesIt_;
        }

        ColorProxy ColorBufferIterator::dereference() const
        {
            return ColorProxy( valuesIt_ );
        }

    } // namespace detail

} // namespace sc
