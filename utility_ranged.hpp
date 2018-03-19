#ifndef SCHLAZICONTROL_UTILITY_RANGED_HPP
#define SCHLAZICONTROL_UTILITY_RANGED_HPP

#include <cassert>
#include <limits>
#include <type_traits>

#include <boost/operators.hpp>

#include "utility_math.hpp"

namespace sc {

    /**
     * class RangedValue
     */

    namespace detail {

        template< typename T >
        struct TypeLimits
        {
            static constexpr T minimum = std::numeric_limits< T >::min();
            static constexpr T maximum = std::numeric_limits< T >::max();
        };

        template< typename T >
        struct UnitLimits
        {
            static constexpr T minimum = 0;
            static constexpr T maximum = 1;
        };

        template< typename T >
        struct PercentLimits
        {
            static constexpr T minimum = 0;
            static constexpr T maximum = 100;
        };

    } // namespace detail

    template< typename T, typename Limits >
    class Ranged
            : public boost::totally_ordered< Ranged< T, Limits > >
            , public boost::totally_ordered< Ranged< T, Limits >, T >
    {
    public:
        using RangedValue_ = Ranged;

        static constexpr T minimum = Limits::minimum;
        static constexpr T maximum = Limits::maximum;

        constexpr Ranged( std::enable_if_t< T() >= minimum && T() <= maximum >* = nullptr ) : value_() {}
        constexpr explicit Ranged( T value ) : value_( requireValid( value ) ) {}
        constexpr explicit Ranged( bool value ) : value_( value ? maximum : minimum ) {}

        template< typename Other >
        constexpr Ranged( Other const& other )
                : value_( sc::scale( other.get(), Other::minimum, Other::maximum, minimum, maximum ) ) {}

        template< typename Other >
        constexpr Ranged& operator=( Other const& other ) { return *this = Ranged( other.get() ); }

        constexpr operator bool() const { return value_ != minimum; }

        constexpr bool operator==( T value ) const { return value_ == value; }
        constexpr bool operator<( T value ) const { return value_ < value; }

        constexpr bool operator==( Ranged const& other ) const { return value_ == other; }
        constexpr bool operator<( Ranged const& other ) const { return value_ < other; }

        constexpr T get() const { return value_; }

    private:
        static T invalid( T value )
        {
            assert( value >= minimum && value <= maximum );
            return value;
        }

        static constexpr T requireValid( T value )
        {
            return ( value >= minimum && value <= maximum )
                   ? value
                   : invalid( value ); // ( assert( value >= minimum && value <= maximum ), T() );
        }

        T value_;
    };


    template< typename T >
    using RangedType = Ranged< T, detail::TypeLimits< T > >;

    template< typename T >
    RangedType< T > rangedType( T value ) { return RangedType< T >( value ); }


    template< typename T >
    using RangedUnit = Ranged< T, detail::UnitLimits< T > >;

    template< typename T >
    RangedUnit< T > rangedUnit( T value ) { return RangedUnit< T >( value ); }


    template< typename T >
    using RangedPercent = Ranged< T, detail::PercentLimits< T > >;

    template< typename T >
    RangedPercent< T > rangedPercent( T value ) { return RangedPercent< T >( value ); }

} // namespace sc

#endif //SCHLAZICONTROL_UTILITY_RANGED_HPP
