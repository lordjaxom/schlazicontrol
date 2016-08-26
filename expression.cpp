#include <boost/config/warning_disable.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/qi.hpp>

#include "expression.hpp"
#include "logging.hpp"

using CallArgument = boost::variant< std::string, std::intmax_t >;

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

                    argument_ %= identifier_ | number_;

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
                Rule< CallArgument() > argument_;
                Rule< std::vector< CallArgument >() > argumentList_;
                Rule< Call() > expression_;
            };

            Call parseCall( std::string const& text )
            {
                logger.debug( "parsing ", text );
                auto first = text.begin();
                auto last = text.end();
                Call result;
                if ( !qi::phrase_parse( first, last, CallGrammar(), ascii::space, result ) || first != last ) {
                    throw runtime_error( str( "unable to parse function expression \"", text, "\"" ) );
                }
                logger.debug( "parsing finished" );
                return result;
            }

            class DurationGrammar
                : public qi::grammar< string::const_iterator, chrono::microseconds(), ascii::space_type >
            {
                template< typename Signature > using Rule =
                qi::rule< string::const_iterator, Signature, ascii::space_type >;

            public:
                DurationGrammar()
                        : DurationGrammar::base_type( expression_ )
                {
                    using qi::int_;
                    using qi::lexeme;
                    using qi::lit;

                    using namespace qi::labels;

                    hours_ %= lexeme[ int_ >> lit( "h" ) ];
                    minutes_ %= lexeme[ int_ >> lit( "min" ) ];
                    seconds_ %= lexeme[ int_ >> lit( "s" ) ];
                    milliseconds_ %= lexeme[ int_ >> lit( "ms" ) ];
                    microseconds_ %= lexeme[ int_ >> lit( "us" ) ];

                    expression_ %= hours_ | minutes_ | seconds_ | milliseconds_ | microseconds_;
                }

            private:
                Rule< chrono::hours() > hours_;
                Rule< chrono::minutes() > minutes_;
                Rule< chrono::seconds() > seconds_;
                Rule< chrono::milliseconds() > milliseconds_;
                Rule< chrono::microseconds() > microseconds_;
                Rule< chrono::microseconds() > expression_;
            };

        } // namespace detail

        chrono::microseconds parseDuration( std::string const& text )
        {
            logger.debug( "parsing ", text );
            auto first = text.begin();
            auto last = text.end();
            chrono::microseconds result;
            if ( !qi::phrase_parse( first, last, detail::DurationGrammar(), ascii::space, result ) || first != last ) {
                throw runtime_error( str( "unable to parse duration \"", text, "\"" ) );
            }
            logger.debug( "parsing finished, got ", result.count(), " us" );
            return result;
        }

    } // namespace expression

} // namespace sc
