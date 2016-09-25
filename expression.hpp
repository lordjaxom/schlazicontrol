#ifndef SCHLAZICONTROL_EXPRESSION_HPP
#define SCHLAZICONTROL_EXPRESSION_HPP

#include <cstdint>
#include <algorithm>
#include <chrono>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <boost/fusion/container/set.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/variant.hpp>
#include <typestring.hh>

#include "typeinfo.hpp"
#include "typetraits.hpp"
#include "utility_stream.hpp"
#include "utility_string.hpp"

namespace sc {

    namespace expression {

        template< typename Derived, typename ...Args >
        struct Factory
        {
            Factory( std::string const& name )
                : name_( name ) {}

            std::string const& name() const { return name_; }

        private:
            std::string name_;
        };

        namespace diagnostics {

            template< typename ...Args >
            std::runtime_error invalidArgument( std::string const& function, std::size_t index, Args&&... args )
            {
                return std::runtime_error( str(
                        "invalid argument ", index + 1, " in call to ", function, ": ", std::forward< Args >( args )... ) );
            }

            namespace detail {

                template< typename Type >
                struct ValueHolder
                {
                    ValueHolder( Type const& value ) : value_( &value ) {}
                    Type const* value_;
                };

                template< typename Type >
                struct Value : ValueHolder< Type >
                {
                    using ValueHolder< Type >::ValueHolder;
                    friend std::ostream& operator<<( std::ostream& os, Value const& value )
                    {
                        return os << *value.value_;
                    }
                };

                template< typename Rep, typename Period >
                struct Value< std::chrono::duration< Rep, Period > >
                        : ValueHolder< std::chrono::duration< Rep, Period > >
                {
                    using ValueHolder< std::chrono::duration< Rep, Period > >::ValueHolder;
                    friend std::ostream& operator<<( std::ostream& os, Value const& value )
                    {
                        return os << std::chrono::duration_cast< std::chrono::nanoseconds >( *value.value_ ).count() << "ns";
                    }
                };

            } // namespace detail

            template< typename Type >
            detail::Value< Type > value( Type const& value )
            {
                return detail::Value< Type >( value );
            }

            namespace detail {

                template< typename ...Values >
                struct Strings
                {
                    Strings( char const* quote, char const* delim )
                            : quote_( quote ), delim_( delim ) {}

                    friend std::ostream& operator<<( std::ostream& os, Strings const& strings )
                    {
                        write( os, strings.quote_, strings.delim_, Values()... );
                        return os;
                    }

                private:
                    static void write( std::ostream& os, char const* quote, char const* delim )
                    {
                    }

                    template< typename Arg0, typename ...Args >
                    static void write(
                            std::ostream& os, char const* quote, char const* delim, Arg0 value0, Args... values )
                    {
                        os << quote;
                        std::copy( Arg0::cbegin(), Arg0::cend(), OutputStreamIterator( os ) );
                        os << quote;
                        if ( sizeof...( Args ) > 0 ) {
                            os << delim;
                        }
                        write( os, quote, delim, values... );
                    }

                    char const* quote_;
                    char const* delim_;
                };

            } // namespace detail

            template< typename ...Values >
            detail::Strings< Values... > strings( char const* quote, char const* delim )
            {
                return detail::Strings< Values... >( quote, delim );
            }

        } // namespace diagnostics

        template< typename Result, Result Minimum, Result Maximum >
        struct Range
        {
            using Type = Result;
            static void validate( std::string const& function, std::size_t index, Type const& value )
            {
                if ( value < Minimum || value > Maximum ) {
                    throw diagnostics::invalidArgument(
                            function, index, "expected numeric value between ", Minimum, " and ", Maximum,
                            " instead of ", value );
                }
            }
        };

        template< typename Result, typename ...Values >
        struct Enumeration
        {
            using Type = Result;

            template< typename ...Args >
            static void validate( std::string const& function, std::size_t index, Result const& value, Args... )
            {
                throw diagnostics::invalidArgument(
                        function, index, "expected one out of enumeration ",
                        diagnostics::strings< Args... >( "\"", ", " ), " instead of \"", value, "\"" );
            }
        };

        template< typename Result, typename Value0, typename ...Values >
        struct Enumeration< Result, Value0, Values... >
        {
            using Type = Result;

            static void validate( std::string const& function, std::size_t index, Result const& value )
            {
                validate( function, index, value, Value0(), Values()... );
            }

            template< typename ...Args >
            static void validate(
                    std::string const& function, std::size_t index, Result const& value, Args... args )
            {
                if ( value.size() != Value0::size() ||
                        !std::equal( Value0::cbegin(), Value0::cend(), value.cbegin() ) ) {
                    Enumeration< Result, Values... >::validate( function, index, value, args... );
                }
            }
        };

        namespace detail {

            template< typename Derived, typename Types >
            struct ArgParserInvoker
            {
                template< typename Other >
                static Derived parse( std::string const& function, std::size_t index, Other&& value )
                {
                    throw std::runtime_error( "TODO: no parser for this type" );
                }
            };

            template< typename Derived, typename Type0, typename ...Types >
            struct ArgParserInvoker< Derived, boost::fusion::set< Type0, Types... > >
                    : ArgParserInvoker< Derived, boost::fusion::set< Types... > >
            {
                using ArgParserInvoker< Derived, boost::fusion::set< Types... > >::parse;
                static Derived parse( std::string const& function, std::size_t index, Type0 const& value )
                {
                    Derived::validate( function, index, value );
                    return Derived( value );
                }
            };

            template< typename Derived, typename ...Types >
            struct ArgParserValidator
            {
                template< typename Other >
                static void validate( std::string const& function, std::size_t index, Other&& value ) {}
            };

            template< typename Derived, typename Type0, typename ...Types >
            struct ArgParserValidator< Derived, Type0, Types... >
                    : ArgParserValidator< Derived, Types... >
            {
                using ArgParserValidator< Derived, Types... >::validate;
                static void validate( std::string const& function, std::size_t index, typename Type0::Type const& value )
                {
                    Type0::validate( function, index, value );
                    ArgParserValidator< Derived, Types... >::validate( function, index, value );
                }
            };

            struct ArgParserTag {};

        } // namespace detail

        template< typename Derived, typename Result, typename ...Args >
        struct ArgParser : detail::ArgParserTag
        {
            using BaseType = ArgParser< Derived, Result, Args... >;

            template< typename Type >
            static Derived parse( std::string const& function, std::size_t index, Type&& value )
            {
                return detail::ArgParserInvoker< Derived, boost::fusion::set< typename Args::Type... > >::parse(
                        function, index, std::forward< Type >( value ) );
            }

            template< typename Type >
            static void validate( std::string const& function, std::size_t index, Type&& value )
            {
                detail::ArgParserValidator< Derived, Args... >::validate(
                        function, index, std::forward< Type >( value ) );
            }

            ArgParser( Result&& result ) : result_( std::move( result ) ) {}

            operator Result&&() { return std::move( result_ ); }

        private:
            Result result_;
        };

        template< typename Base, typename ...Factories >
        std::unique_ptr< Base > parseExpression( std::string const& text, Factories&& ... factories );

        namespace detail {

            using CallArgument = boost::variant< std::string, std::intmax_t, std::chrono::nanoseconds >;

            struct Call
            {
                std::string function;
                std::vector< CallArgument > arguments;
            };

            template< typename Type >
            struct ArgumentVisitorFallback
            {
                template< typename Other >
                static Type invoke( std::string const& function, std::size_t index, Other&& value )
                {
                    throw diagnostics::invalidArgument(
                            function, index, "got ", typeName< Other >(), " \"",
                            diagnostics::value( std::forward< Other >( value ) ), "\" instead of ",
                            typeName< Type >() );
                }
            };

            template< typename Type, typename Enable = void >
            struct ArgumentVisitorImpl;

            template<>
            struct ArgumentVisitorImpl< std::string >
                    : ArgumentVisitorFallback< std::string >
            {
                static std::string invoke( std::string const& function, std::size_t index, std::string const& value )
                {
                    return value;
                }

                using ArgumentVisitorFallback< std::string >::invoke;
            };

            template< typename Type >
            struct ArgumentVisitorImpl< Type, std::enable_if_t< IsIntegral< Type >() > >
                    : ArgumentVisitorFallback< Type >
            {
                static Type invoke( std::string const& function, std::size_t index, std::intmax_t value )
                {
                    return value;
                }

                using ArgumentVisitorFallback< Type >::invoke;
            };

            template< typename Rep, typename Period >
            struct ArgumentVisitorImpl< std::chrono::duration< Rep, Period > >
                    : ArgumentVisitorFallback< std::chrono::duration< Rep, Period > >
            {
                static std::chrono::duration< Rep, Period > invoke(
                        std::string const& function, std::size_t index, std::chrono::nanoseconds value )
                {
                    return std::chrono::duration_cast< std::chrono::duration< Rep, Period > >( value );
                }

                using ArgumentVisitorFallback< std::chrono::duration< Rep, Period > >::invoke;
            };

            template< typename Type >
            struct ArgumentVisitorImpl< Type, std::enable_if_t< IsBaseOf< ArgParserTag, Type >() > >
            {
                template< typename Other >
                static Type invoke( std::string const& function, std::size_t index, Other&& value )
                {
                    return Type::parse( function, index, std::forward< Other >( value ) );
                }
            };

            template< typename Type >
            struct ArgumentVisitor : boost::static_visitor< Type >
            {
                ArgumentVisitor( std::string const& function, std::size_t index )
                        : function_( function )
                        , index_( index )
                {
                }

                template< typename Other >
                Type operator()( Other&& value ) const
                {
                    return ArgumentVisitorImpl< Type >::invoke( function_, index_, std::forward< Other >( value ) );
                }

            private:
                std::string const& function_;
                std::size_t index_;
            };

            template< typename Type >
            Type expand( Call const& call, size_t index )
            {
                return boost::apply_visitor( ArgumentVisitor< Type >( call.function, index ), call.arguments[ index ] );
            }

            template< typename Base >
            std::unique_ptr< Base > create( Call const& call )
            {
                throw std::runtime_error( str( "unknown function ", call.function, " in expression" ));
            }

            template< typename Base, typename Derived, typename ...Args, typename ...Factories >
            std::unique_ptr< Base > create(
                    Call const& call, Factory< Derived, Args... > const& factory, Factories&& ... factories )
            {
                if ( factory.name() != call.function ) {
                    return create< Base >( call, std::forward< Factories >( factories )... );
                }

                if ( sizeof...( Args ) != call.arguments.size()) {
                    throw std::runtime_error( str(
                            "invalid number of arguments to function ", factory.name(), " (expected ",
                            sizeof...( Args ), " but was ", call.arguments.size(), ")" ) );
                }
                size_t index = 0;
                return std::unique_ptr< Derived >(
                        new Derived( expand< Args >( call, index++ )... ) );
            }

            Call parseExpression( std::string const& text );

        } // namespace detail

        template< typename Base, typename ...Factories >
        std::unique_ptr< Base > parseExpression( std::string const& text, Factories&& ... factories )
        {
            return detail::create< Base >( detail::parseExpression( text ), std::forward< Factories >( factories )... );
        }

        std::chrono::nanoseconds parseDuration( std::string const& text );

    } // namespace expression

} // namespace sc

#endif //SCHLAZICONTROL_EXPRESSION_HPP