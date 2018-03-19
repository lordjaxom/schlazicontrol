#ifndef SCHLAZICONTROL_TYPES_HPP
#define SCHLAZICONTROL_TYPES_HPP

#include <cassert>
#include <cstdint>
#include <array>
#include <initializer_list>
#include <iosfwd>
#include <limits>
#include <vector>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/operators.hpp>

#include "forward.hpp"
#include "statistics.hpp"
#include "trackable.hpp"
#include "utility_graphics.hpp"
#include "utility_ranged.hpp"

namespace sc {

    /**
     * class ChannelValue
     */

	class ChannelValue
            : public RangedPercent< double >
	{
	public:
		static constexpr double epsilon = 0.01;

		static constexpr ChannelValue offValue() { return ChannelValue { minimum }; }
		static constexpr ChannelValue fullOnValue() { return ChannelValue { maximum }; }

        using RangedValue_::Ranged;

        bool off() const { return get() == minimum; }
		bool on() const { return get() != minimum; }
		bool fullOn() const { return get() == maximum; }
	};

    namespace detail {

        using ChannelBufferValues = std::vector< ChannelValue >;

        class SmartChannelBuffer;

        /**
         * class SmartChannelBufferIterator
         */

        class SmartChannelBufferIterator
                : public boost::iterator_facade<
                        SmartChannelBufferIterator,
                        ChannelValue const,
                        boost::forward_traversal_tag
                        >
        {
            friend class boost::iterator_core_access;

            using Values = ChannelBufferValues;

            struct EndIteratorTag {};

        public:
            static constexpr EndIteratorTag endIterator {};

            explicit SmartChannelBufferIterator( SmartChannelBuffer const& owner );
            SmartChannelBufferIterator( SmartChannelBuffer const& owner, EndIteratorTag );

            void increment();
            bool equal( SmartChannelBufferIterator const& other ) const;
            ChannelValue const& dereference() const;

        private:
            SmartChannelBuffer const* owner_;
            std::size_t offsetIt_;
            std::size_t repeatIt_;
            Values::const_iterator valuesIt_;
            Values::const_iterator valuesLast_;
        };

        /**
         * class SmartChannelBuffer
         */

		class SmartChannelBuffer
				: Trackable< SmartChannelBuffer >
		{
			friend class detail::SmartChannelBufferIterator;

		public:
			using Values = std::vector< ChannelValue >;

			using iterator = Values::iterator;
			using const_iterator = detail::SmartChannelBufferIterator;

			SmartChannelBuffer();
			SmartChannelBuffer( ChannelValue const& value );
			explicit SmartChannelBuffer( std::initializer_list< ChannelValue > initializer );
			explicit SmartChannelBuffer( std::size_t size );
            ~SmartChannelBuffer();

			bool empty() const { return offset_ == 0 && values_.size() == 0; }
			std::size_t size() const { return offset_ + values_.size() * repeat_; }

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

        /**
         * class SimpleChannelBuffer
         */

        class SimpleChannelBuffer
                : Trackable< SimpleChannelBuffer >
        {
		public:
			using Values = std::vector< ChannelValue >;

			using iterator = Values::iterator;
			using const_iterator = Values::const_iterator;

            SimpleChannelBuffer();
            SimpleChannelBuffer( ChannelValue const& value );
			explicit SimpleChannelBuffer( std::initializer_list< ChannelValue > initializer );
			explicit SimpleChannelBuffer( std::size_t size );
            ~SimpleChannelBuffer();

			bool empty() const { return values_.empty(); }
			std::size_t size() const { return values_.size(); }

			void fill( ChannelValue const& value = {} );

			void shift( std::size_t offset );
			void multiply( std::size_t repeat );

			ChannelValue& operator[]( std::size_t index ) { return values_[ index ]; }
			ChannelValue const& operator[]( std::size_t index ) const { return values_[ index ]; }

			iterator begin() { return values_.begin(); }
			iterator end() { return values_.end(); }
			const_iterator cbegin() const { return values_.cbegin(); }
			const_iterator cend() const { return values_.cend(); }
			const_iterator begin() const { return cbegin(); }
			const_iterator end() const { return cend(); }

            using Trackable_::tracker;

			void statistics( std::ostream& os ) const;

		private:
			static constexpr ChannelValue emptyValue_ {};

			Values values_;
		};

	} // namespace detail

    using ChannelBuffer =
#if defined( USE_SMART_CHANNEL_BUFFER )
            detail::SmartChannelBuffer
#else
            detail::SimpleChannelBuffer
#endif
            ;

    namespace detail {

        /**
         * class ColorProxy
         */

        class ColorProxy
        {
        public:
            explicit ColorProxy( ChannelBuffer::iterator wrapped );

            Rgb get() const;
            void set( Rgb const& color );

            operator Rgb() const { return get(); }
            ColorProxy& operator=( Rgb const& color ) { set( color ); return *this; }

        private:
            ChannelBuffer::iterator wrapped_;
        };

        /**
         * class ColorIterator
         */

        class ColorBufferIterator
                : public boost::iterator_facade<
                        ColorBufferIterator,
                        ColorProxy,
                        boost::forward_traversal_tag,
                        ColorProxy
                >
        {
            friend class boost::iterator_core_access;

            using Values = ChannelBuffer;

            struct EndIteratorTag {};

        public:
            static constexpr EndIteratorTag endIterator {};

            explicit ColorBufferIterator( ColorBuffer& owner );
            ColorBufferIterator( ColorBuffer& owner, EndIteratorTag );

            void increment();
            bool equal( ColorBufferIterator const& other ) const;
            ColorProxy dereference() const;

        private:
            ColorBuffer* owner_;

            Values::iterator valuesIt_;
            Values::iterator valuesLast_;
        };

    } // namespace detail

    /**
     * class ColorBuffer
     */

    class ColorBuffer
    {
        friend class detail::ColorBufferIterator;

    public:
        using Proxy = detail::ColorProxy;

        using iterator = detail::ColorBufferIterator;

        explicit ColorBuffer( ChannelBuffer& wrapped );

        bool empty() const { return wrapped_->empty(); }
        std::size_t size() const { return wrapped_->size() / 3; }

        iterator begin();
        iterator end();

        Proxy operator[]( std::size_t index );

    private:
        ChannelBuffer* wrapped_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_TYPES_HPP
