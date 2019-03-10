#include <boost/config/warning_disable.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/qi.hpp>

#include "expression.hpp"
#include "core/logging.hpp"

using CallArgument = sc::expression::detail::CallArgument;

BOOST_FUSION_ADAPT_STRUCT(
    sc::expression::detail::Call,
    ( std::string, function ),
    ( std::vector< CallArgument >, arguments )
)

using namespace std;

namespace fusion = boost::fusion;
namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

namespace sc {

    static Logger logger( "expression" );

    namespace expression {

        namespace detail {

            class DurationGrammar
                    : public qi::grammar< string::const_iterator, chrono::nanoseconds(), ascii::space_type >
            {
                template< typename Signature > using Rule =
                qi::rule< string::const_iterator, Signature, ascii::space_type >;

            public:
                DurationGrammar()
                        : DurationGrammar::base_type( duration_ )
                {
                    using qi::int_;
                    using qi::lexeme;
                    using qi::lit;
                    using qi::char_;
                    using ascii::alnum;
                    using ascii::alpha;

                    using namespace qi::labels;

                    hours_ %= lexeme[ int_ >> lit( "h" ) ];
                    minutes_ %= lexeme[ int_ >> lit( "min" ) ];
                    seconds_ %= lexeme[ int_ >> lit( "s" ) ];
                    milliseconds_ %= lexeme[ int_ >> lit( "ms" ) ];
                    microseconds_ %= lexeme[ int_ >> lit( "us" ) ];
                    nanoseconds_ %= lexeme[ int_ >> lit( "ns" ) ];
                    duration_ %= hours_ | minutes_ | seconds_ | milliseconds_ | microseconds_ | nanoseconds_;
                }

            private:
                Rule< chrono::hours() > hours_;
                Rule< chrono::minutes() > minutes_;
                Rule< chrono::seconds() > seconds_;
                Rule< chrono::milliseconds() > milliseconds_;
                Rule< chrono::microseconds() > microseconds_;
                Rule< chrono::nanoseconds() > nanoseconds_;
                Rule< chrono::nanoseconds() > duration_;
            };

            class CallGrammar
                    : public qi::grammar< string::const_iterator, Call(), ascii::space_type >
            {
                template< typename Signature > using Rule =
                qi::rule< string::const_iterator, Signature, ascii::space_type >;

            public:
                CallGrammar()
                        : CallGrammar::base_type( expression_ )
                {
                    using qi::int_;
                    using qi::lexeme;
                    using qi::lit;
                    using qi::char_;
                    using ascii::alnum;
                    using ascii::alpha;

                    using namespace qi::labels;

                    identifier_ %= lexeme[( alpha | char_( '_' ) ) >> *( alnum | char_( '_' ) )];

                    number_ %= int_;

                    argument_ %= identifier_ | duration_ | number_;

                    argumentList_ %= argument_ % lit( ',' );

                    expression_ %=
                            identifier_
                                    >> lit( '(' )
                                    >> -argumentList_
                                    >> lit( ')' );
                }

            private:
                Rule< string() > identifier_;
                Rule< intmax_t() > number_;
                DurationGrammar duration_;
                Rule< CallArgument() > argument_;
                Rule< vector< CallArgument >() > argumentList_;
                Rule< Call() > expression_;
            };

            Call parseExpression( string const& text )
            {
                auto first = text.begin();
                auto last = text.end();
                Call result;
                if ( !qi::phrase_parse( first, last, CallGrammar(), ascii::space, result ) || first != last ) {
                    throw runtime_error( str( "unable to parse function expression \"", text, "\"" ) );
                }
                return result;
            }

        } // namespace detail

        chrono::nanoseconds parseDuration( string const& text )
        {
            auto first = text.begin();
            auto last = text.end();
            chrono::nanoseconds result;
            if ( !qi::phrase_parse( first, last, detail::DurationGrammar(), ascii::space, result ) || first != last ) {
                throw runtime_error( str( "unable to parse duration expression \"", text, "\"" ) );
            }
            return result;
        }

    } // namespace expression

} // namespace sc
