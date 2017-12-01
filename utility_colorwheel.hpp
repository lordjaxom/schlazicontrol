#ifndef SCHLAZICONTROL_UTILITY_COLORWHEEL_HPP
#define SCHLAZICONTROL_UTILITY_COLORWHEEL_HPP

#include <cstddef>
#include <cstdint>
#include <cmath>

#include "utility_valuetable.hpp"

namespace sc {

    static constexpr double pi = 3.1415926;

	namespace detail {

		template< size_t N >
		struct ColorwheelFunction
		{
			static std::uint32_t constexpr apply( size_t input )
			{
				return ( r( input ) << 16 ) | ( g( input ) << 8 ) | b( input );
			}

		private:
			static std::size_t  constexpr i( std::size_t input ) { return input * 3 / N; }
			static double  constexpr f( std::size_t input ) { return pi * ( 3.0 * input - i( input ) * N ) / ( 2.0 * N ); }
			static int     constexpr s( std::size_t input ) { return 255.0 * std::sin( f( input ) ); }
			static int     constexpr c( std::size_t input ) { return 255.0 * std::cos( f( input ) ); }
			static std::uint8_t constexpr r( std::size_t input ) { return (std::uint8_t) ( ( i( input ) == 0 ? 1 : 0 ) * s( input ) + ( i( input ) == 1 ? 1 : 0 ) * c( input ) ); }
			static std::uint8_t constexpr g( std::size_t input ) { return (std::uint8_t) ( ( i( input ) == 1 ? 1 : 0 ) * s( input ) + ( i( input ) == 2 ? 1 : 0 ) * c( input ) ); }
			static std::uint8_t constexpr b( std::size_t input ) { return (std::uint8_t) ( ( i( input ) == 2 ? 1 : 0 ) * s( input ) + ( i( input ) == 0 ? 1 : 0 ) * c( input ) ); }
		};

	} // namespace detail

	template< size_t N >
	struct Colorwheel
	{
		static std::size_t const size = N;

		static std::uint32_t get( std::size_t index )
		{
			return color( ValueTable< detail::ColorwheelFunction< N >, 0, N >::get( index ) );
		}
	};

} // namespace sc

#endif // SCHLAZICONTROL_UTILITY_COLORWHEEL_HPP
