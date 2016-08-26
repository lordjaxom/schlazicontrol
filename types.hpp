#ifndef SCHLAZICONTROL_TYPES_HPP
#define SCHLAZICONTROL_TYPES_HPP

#include <cstdint>
#include <initializer_list>
#include <vector>

#include <boost/operators.hpp>

#include "utility.hpp"

namespace sc {

	class ChannelValue : public boost::totally_ordered< ChannelValue >
	{
	public:
		static constexpr double minimum = 0.0;
		static constexpr double maximum = 100.0;

		static constexpr ChannelValue offValue() { return ChannelValue { minimum }; }
		static constexpr ChannelValue fullOnValue() { return ChannelValue { maximum }; }

		ChannelValue();
		constexpr ChannelValue( double value ) : value_( value ) {}
		ChannelValue( double value, double min, double max );
		ChannelValue( bool value );

        bool operator==( ChannelValue const& other ) const { return value_ == other.value_; }
        bool operator<( ChannelValue const& other ) const { return value_ < other.value_; }

		double get() const { return value_; }

		double scale( double min, double max ) const { return sc::scale( value_, minimum, maximum, min, max ); }

		bool off() const { return value_ == minimum; }
		bool on() const { return value_ != minimum; }
		bool fullOn() const { return value_ == maximum; }

	private:
		double value_;
	};

	class ChannelBuffer
	{
	public:
		using Values = std::vector< ChannelValue >;

		ChannelBuffer();
		explicit ChannelBuffer( std::initializer_list< ChannelValue > initializer );
		explicit ChannelBuffer( std::size_t size );

		std::size_t size() const { return values_.size(); }

		void resize( std::size_t size );

		ChannelValue& operator[]( std::size_t index ) { return values_[ index ]; }
		ChannelValue const& operator[]( std::size_t index ) const { return values_[ index ]; }

		Values::iterator begin() { return values_.begin(); }
		Values::iterator end() { return values_.end(); }

	private:
		Values values_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_TYPES_HPP
