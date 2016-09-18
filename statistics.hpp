#ifndef SCHLAZICONTROL_STATISTICS_HPP
#define SCHLAZICONTROL_STATISTICS_HPP

#include <algorithm>
#include <iterator>
#include <memory>
#include <ostream>
#include <tuple>
#include <type_traits>
#include <utility>
#include "utility.hpp"

namespace sc {

    template< typename Type > class Statistics;
    template< typename Type > Statistics< Type > makeStatistics( Type const& value );

    template< typename Type, typename Enable = void >
    struct StatisticsWriter;

    template< typename Type >
    struct StatisticsWriter< std::unique_ptr< Type > >
    {
        void operator()( std::ostream& os, std::unique_ptr< Type > const& value )
        {
            os << makeStatistics( *value );
        }
    };

    template< typename Type >
    struct StatisticsWriter<
            Type,
            std::conditional_t<
                    false,
                    std::tuple<
                            typename Type::key_type,
                            typename Type::mapped_type,
                            typename Type::value_type,
                            typename Type::const_iterator,
                            decltype( std::declval< Type >().begin() ),
                            decltype( std::declval< Type >().end() )
                            >,
                    void
                    >
            >
    {
        void operator()( std::ostream& os, Type const& value )
        {
            std::transform( value.begin(), value.end(), OutputStreamIterator( os ),
                            []( auto const& entry ) { return makeStatistics( entry.second ); } );
        }
    };

    template< typename Type >
    struct StatisticsWriter<
            Type,
            std::conditional_t<
                    false,
                    decltype( std::declval< Type const >().statistics( std::declval< std::ostream& >() ) ),
                    void
                    >
            >
    {
        void operator()( std::ostream& os, Type const& value )
        {
            value.statistics( os );
        }
    };

    template< typename Type >
    class Statistics
    {
    public:
        Statistics( Type const& value )
                : value_( value )
        {
        }

        friend std::ostream& operator<<( std::ostream& os, Statistics const& statistics )
        {
            StatisticsWriter< Type > writer;
            writer( os, statistics.value_ );
            return os;
        }

    private:
        Type const& value_;
    };

    template< typename Type >
    Statistics< Type > makeStatistics( Type const& value )
    {
        return Statistics< Type >( value );
    }

} // namespace sc

#endif // SCHLAZICONTROL_STATISTICS_HPP
