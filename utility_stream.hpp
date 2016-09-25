#ifndef SCHLAZICONTROL_UTILITY_STREAM_HPP
#define SCHLAZICONTROL_UTILITY_STREAM_HPP

#include <iterator>
#include <iosfwd>
#include <tuple>

namespace sc {

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

} // namespace sc

#endif //SCHLAZICONTROL_UTILITY_STREAM_HPP
