#ifndef SCHLAZICONTROL_UTILITY_GAMMA_HPP
#define SCHLAZICONTROL_UTILITY_GAMMA_HPP

#include <cmath>
#include <cstdint>
#include <limits>

#include "utility_valuetable.hpp"

namespace sc {

	namespace detail {

		template< typename Gamma >
		struct GammaFunction
		{
			static std::uint8_t constexpr const max = std::numeric_limits< std::uint8_t >::max();
            static double constexpr const value = (double) Gamma::num / Gamma::den;

            static std::uint8_t constexpr apply( uint8_t input )
			{
				return std::pow( (double) input / max, ( 1.0 / value ) ) * max;
			}
		};

	} // namespace detail

	template< typename Gamma >
	struct GammaTable
	{
		static std::uint8_t get( std::uint8_t input )
		{
			return ValueTable< detail::GammaFunction< Gamma > >::get( input );
		}
	};

} // namespace sc

#endif // SCHLAZICONTROL_UTILITY_GAMMA_HPP
