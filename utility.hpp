#ifndef SCHLAZICONTROL_UTILITY_HPP
#define SCHLAZICONTROL_UTILITY_HPP

#include <algorithm>
#include <iterator>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>

namespace sc {

	template< bool Cond, typename Result = void > using EnableIf = typename std::enable_if< Cond, Result >::type;

	template< typename Type > using Decay = typename std::decay< Type >::type;
	template< typename Type > using RemoveReference = typename std::remove_reference< Type >::type;
	template< typename Type > using RemoveCv = typename std::remove_cv< Type >::type;

	template< typename Type > constexpr bool IsIntegral() { return std::is_integral< Type >::value; }
	template< typename Type > constexpr bool IsEnum() { return std::is_enum< Type >::value; }

    template< typename Type, typename Other > constexpr bool IsSame() { return std::is_same< Type, Other >::value; }
	template< typename Base, typename Derived > constexpr bool IsBaseOf() { return std::is_base_of< Base, Derived >::value; }

    template< typename Type >
    Type clip( Type value, Type min, Type max )
    {
        return value < min ? min : value > max ? max : value;
    }

    template< typename Type >
    Type scale( Type value, Type fromMin, Type fromMax, Type toMin, Type toMax )
    {
        return ( value - fromMin ) * ( toMax - toMin ) / ( fromMax - fromMin ) + toMin;
    }
	
	namespace detail {

		inline void strWrite( std::ostream& os ) {}

		template< typename Arg0, typename ...Args >
		void strWrite( std::ostream& os, Arg0&& arg0, Args&&... args )
		{
			os << std::forward< Arg0 >( arg0 );
			strWrite( os, std::forward< Args >( args )... );
		}

	} // namespace detail

	template< typename ...Args >
	std::string str( Args&&... args )
	{
		std::ostringstream os;
		detail::strWrite( os, std::forward< Args >( args )... );
		return os.str();
	}

    namespace detail {

        template< typename Delim, typename Iter >
        struct Joiner
        {
            Joiner( Delim&& delimiter, Iter first, Iter last )
                    : delimiter_( std::move( delimiter ) ), first_( first ), last_( last ) {}

            friend std::ostream& operator<<( std::ostream& os, Joiner const& joiner )
            {
                using ValueType = typename std::iterator_traits< Iter >::value_type;
                std::copy(
                        joiner.first_, joiner.last_,
                        std::ostream_iterator< ValueType >( os, joiner.delimiter_ ) );
                return os;
            }

        private:
            Delim delimiter_;
            Iter first_;
            Iter last_;
        };

    } // namespace detail

    template< typename Delim, typename Iter >
    detail::Joiner< Delim, Iter > join( Delim delimiter, Iter first, Iter last )
    {
        return detail::Joiner< Delim, Iter >( std::move( delimiter ), first, last );
    }

    template< typename Delim, typename Range >
    detail::Joiner< Delim, typename Range::const_iterator > join( Delim delimiter, Range const& range )
    {
        return detail::Joiner< Delim, typename Range::const_iterator >(
                std::move( delimiter ), range.begin(), range.end() );
    }

} // namespace sc

#if 0
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace sc {

	namespace detail {

		struct decimal_t {};
		struct hexadecimal_t {};
		struct octal_t {};


		static constexpr std::uint8_t digits_npos = std::numeric_limits< std::uint8_t >::max();


		template< typename CharT, std::size_t Offset, CharT... Digits >
		struct digit_set
		{
			template< std::size_t I = 0 >
			static constexpr std::uint8_t value_of( CharT )
			{
				return digits_npos;
			}
		};

		template< typename CharT, std::size_t Offset, CharT Digit0, CharT... Digits >
		struct digit_set< CharT, Offset, Digit0, Digits... >
		{
			static_assert( sizeof...( Digits ) < digits_npos - Offset, "digit limit exceeded" );

			template< std::size_t I = 0 >
			static constexpr std::uint8_t value_of( CharT digit )
			{
				return digit == Digit0
						? I + Offset
						: digit_set< CharT, Digits... >::template value_of< I + 1 >( digit );
			}
		};


		template< typename ...DigitSets >
		struct digits
		{
			template< typename CharT >
			std::uint8_t value_of( CharT digit )
			{
				throw std::range_error();
			}
		};
		
		template< typename DigitSet0, typename ...DigitSets >
		struct digits< DigitSet0, DigitSets... >
		{
			template< typename CharT >
			std::uint8_t value_of( CharT digit )
			{
				std::uint8_t result = DigitSet0::value_of( digit );
				return result != digits_npos
						? result
						: digits< CharT, DigitSets... >::value_of( digit );
			}
		};


		template< std::uint8_t Base, typename T, typename CharT, template< typename > class Digits >
		struct grabber
		{
			using digits_type = Digits< CharT >;

			grabber( T& result )
				: result_( &result )
			{
				result_ = T();
			}

			void operator()( CharT digit )
			{
				T next = result_ * digits_type::base;
				if ( next / digits_type::base != result_ ) {
					throw std::overflow_error();
				}
				result_ = next + digits_type::value_of( digit );
			}

		private:
			T* result_;
		};


		template< typename CharT >
		struct octal_digits;

		template<>
		struct octal_digits< char > : digits< digit_set< char, 0, '0', '1', '2', '3', '4', '5', '6', '7' > >
		{
		};


		template< typename CharT >
		struct decimal_digits;

		template<>
		struct decimal_digits< char > : digits< digit_set< char, 0, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' > >
		{
		};


		template< typename CharT >
		struct hexadecimal_digits;

		template<>
		struct hexadecimal_digits< char > : digits< 
				digit_set< char, 0, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' >,
				digit_set< char, 10, 'a', 'b', 'c', 'd', 'e', 'f' > >
		{
		};


		template< typename T, typename CharT, typename Policy >
		struct grabber_traits;

		template< typename T, typename CharT >
		struct grabber_traits< T, CharT, octal_t >
		{
			using type = grabber< 8, T, CharT, octal_digits >;
		};

		template< typename T, typename CharT >
		struct grabber_traits< T, CharT, decimal_t >
		{
			using type = grabber< 8, T, CharT, decimal_digits >;
		};

		template< typename T, typename CharT >
		struct grabber_traits< T, CharT, hexadecimal_t >
		{
			using type = grabber< 8, T, CharT, hexadecimal_digits >;
		};


		template< typename T, typename Policy >
		struct integral_converter
		{
			template< typename FwdIt >
			static T convert( FwdIt first, FwdIt last )
			{
				using char_type = decltype( *first );
				T result;
				std::for_each( first, last, typename grabber_traits< T, char_type, Policy >::type( result ) );
				return result;
			}
		};


		template< typename T, typename Policy, typename FwdIt, typename std::enable_if< std::is_integral< T >::value >::type* = nullptr >
		T convert( FwdIt first, FwdIt last )
		{
			return integral_converter< T, Policy >::convert( first, last );
		}

	} // namespace detail


	static constexpr detail::decimal_t decimal;
	static constexpr detail::hexadecimal_t hexadecimal;
	static constexpr detail::octal_t octal;


	template< typename T, typename FwdIt, typename Policy >
	T convert( FwdIt first, FwdIt last, Policy = decimal )
	{
		return detail::convert< T, Policy >( first, last );
	}

} // namespace sc
#endif // 0

#endif // SCHLAZICONTROL_UTILITY_HPP
