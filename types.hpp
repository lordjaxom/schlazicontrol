#ifndef SCHLAZICONTROL_TYPES_HPP
#define SCHLAZICONTROL_TYPES_HPP

#include <cstdint>
#include <initializer_list>
#include <iosfwd>
#include <vector>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/operators.hpp>

#include "statistics.hpp"
#include "trackable.hpp"
#include "utility_math.hpp"

namespace sc {

    /**
     * class ChannelValue
     */

	class ChannelValue
            : public boost::totally_ordered< ChannelValue >
	{
	public:
		static constexpr double minimum = 0.0;
		static constexpr double maximum = 100.0;
		static constexpr double epsilon = 0.01;

		static constexpr ChannelValue offValue() { return ChannelValue { minimum }; }
		static constexpr ChannelValue fullOnValue() { return ChannelValue { maximum }; }

		constexpr ChannelValue() : value_() {}
        constexpr explicit ChannelValue( double value ) : value_( value ) {}
		constexpr explicit ChannelValue( bool value ) : value_( value ? maximum : minimum ) {}

        template< typename Other >
        constexpr ChannelValue( Other value, Other min, Other max )
                : value_( sc::scale< double >( value, min, max, minimum, maximum ) ) {}

        operator bool() const;

        bool operator==( ChannelValue const& other ) const;
        bool operator<( ChannelValue const& other ) const;

        double get() const { return value_; }

        template< typename Other >
        Other scale( Other min, Other max ) const { return sc::scale< Other >( value_, minimum, maximum, min, max ); }

        bool off() const { return value_ == minimum; }
		bool on() const { return value_ != minimum; }
		bool fullOn() const { return value_ == maximum; }

	private:
		double value_;
	};

    /**
     * class ChannelBuffer
     */

    class ChannelBuffer;

    namespace detail {

        using ChannelBufferValues = std::vector< ChannelValue >;

        class ChannelBufferIterator
                : public boost::iterator_facade<
                        ChannelBufferIterator,
                        ChannelValue const,
                        boost::forward_traversal_tag
                        >
        {
            friend class boost::iterator_core_access;

            using Values = ChannelBufferValues;

            struct EndIteratorTag {};

        public:
            static constexpr EndIteratorTag endIterator {};

            explicit ChannelBufferIterator( ChannelBuffer const& owner );
            ChannelBufferIterator( ChannelBuffer const& owner, EndIteratorTag );

            void increment();
            bool equal( ChannelBufferIterator const& other ) const;
            ChannelValue const& dereference() const;

        private:
            ChannelBuffer const* owner_;
            std::size_t offsetIt_;
            std::size_t repeatIt_;
            Values::const_iterator valuesIt_;
            Values::const_iterator valuesLast_;
        };

    } // namespace detail

	class ChannelBuffer
            : Trackable< ChannelBuffer >
	{
        friend class detail::ChannelBufferIterator;

	public:
        using Values = std::vector< ChannelValue >;

        using iterator = Values::iterator;
        using const_iterator = detail::ChannelBufferIterator;

		ChannelBuffer();
        ChannelBuffer( ChannelValue const& value );
		explicit ChannelBuffer( std::initializer_list< ChannelValue > initializer );
        explicit ChannelBuffer( std::size_t size );

		bool empty() const { return offset_ == 0 && values_.size() == 0; }
		std::size_t size() const { return offset_ + values_.size() * repeat_; }

        void resize( std::size_t size );
        void fill( ChannelValue const& value = {} );

        void shift( std::size_t offset );
        void multiply( std::size_t repeat );

		ChannelValue& operator[]( std::size_t index );
		ChannelValue const& operator[]( std::size_t index ) const;

		iterator begin();
		iterator end();
        const_iterator cbegin() const;
        const_iterator cend() const;
		const_iterator begin() const { return cbegin(); }
		const_iterator end() const { return cend(); }

        using Trackable_::tracker;

        void statistics( std::ostream& os ) const;

	private:
        static constexpr ChannelValue emptyValue_ {};

        void expand( std::size_t index = 0 );

        std::size_t offset_;
        std::size_t repeat_;
		Values values_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_TYPES_HPP
