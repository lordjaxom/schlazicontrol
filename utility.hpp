#ifndef SCHLAZICONTROL_UTILITY_HPP
#define SCHLAZICONTROL_UTILITY_HPP

#include <algorithm>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>

namespace sc {

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

    /**
     * class OutputStreamIterator
     */

    class OutputStreamIterator
            : public std::iterator< std::output_iterator_tag, void, void, void, void >
    {
    public:
        OutputStreamIterator( std::ostream& os, char const* infix = "" )
                : os_( &os )
                , infix_( infix )
        {
        }

        template< typename Type >
        OutputStreamIterator& operator=( Type const& value )
        {
            if ( !first_ ) {
                *os_ << infix_;
            }
            else {
                first_ = false;
            }
            *os_ << value;
            return *this;
        }

        OutputStreamIterator& operator*() { return *this; }
        OutputStreamIterator& operator++() { return *this; }
        OutputStreamIterator& operator++( int ) { return *this; }

    private:
        std::ostream* os_;
        char const* infix_;
        bool first_ { true };
    };

    /**
     * output stream operator for tuples
     */

    namespace detail {

        template< typename Tuple, std::size_t I >
        struct TupleWriter
        {
            static void write( std::ostream& os, Tuple const& tuple )
            {
                TupleWriter< Tuple, I - 1 >::write( os, tuple );
                os << std::get< I - 1 >( tuple );
            }
        };

        template< typename Tuple >
        struct TupleWriter< Tuple, 0 >
        {
            static void write( std::ostream& os, Tuple const& tuple )
            {
            }
        };

    } // namespace detail

    template< typename ...Args >
    std::ostream& operator<<( std::ostream& os, std::tuple< Args... > const& tuple )
    {
        detail::TupleWriter< decltype( tuple ), sizeof...( Args ) >::write( os, tuple );
        return os;
    }

    /**
     * for_each algorithm for arbitrary number of iterators
     */

    template< typename Function, typename InputIt, typename ...InputIts >
    void forEach( Function function, InputIt first, InputIt last, InputIts... others )
    {
        for ( ; first != last ; ++first, void( ++others... ) ) {
            function( *first, *others... );
        }
    }

    /**
     * transform algorithm for arbitrary number of iterators
     */

    template< typename Function, typename InputIt, typename OutputIt, typename ...InputIts >
    OutputIt transform( Function function, InputIt first, InputIt last, OutputIt dest, InputIts... others )
    {
        while ( first != last ) {
            *dest++ = function( *first++, *others++... );
        }
        return dest;
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
