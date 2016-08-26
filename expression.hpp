#ifndef SCHLAZICONTROL_EXPRESSION_HPP
#define SCHLAZICONTROL_EXPRESSION_HPP

#include <cstdint>
#include <chrono>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/variant.hpp>

#include "utility.hpp"

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

        template< typename Base, typename ...Factories >
        std::unique_ptr< Base > parseCall( std::string const& text, Factories&&... factories );

        std::chrono::microseconds parseDuration( std::string const& text );

        template< typename ...Args >
        std::runtime_error invalidArgument( std::string const& function, std::size_t index, Args&&... args )
        {
            return std::runtime_error( str(
                    "invalid argument ", index + 1, " in call to ", function, ": ", std::forward< Args >( args )... ) );
        }

        template< typename Type, typename Enable = void >
        struct ArgumentConverter;

        template<>
        struct ArgumentConverter< std::string >
        {
            static std::string const& parse( std::string const& function, std::size_t index, std::string const& value )
            {
                return value;
            }

            static std::string const& parse( std::string const& function, std::size_t index, std::intmax_t const& value )
            {
                throw invalidArgument( function, index, "expected string, got number" );
            }
        };

        template< typename Type >
        struct ArgumentConverter< Type, EnableIf< IsIntegral< Type >() > >
        {
            static Type parse( std::string const& function, std::size_t index, std::string const& value )
            {
                throw invalidArgument( function, index, "expected number, got string" );
            }

            static Type parse( std::string const& function, std::size_t index, std::intmax_t const& value )
            {
                return (Type) value;
            }
        };

        template<>
        struct ArgumentConverter< std::chrono::microseconds >
        {
            static std::chrono::microseconds parse(
                    std::string const& function, std::size_t index, std::string const& value )
            {
                return parseDuration( value );
            }

            static std::chrono::microseconds parse(
                    std::string const& function, std::size_t index, std::intmax_t const& value )
            {
                throw invalidArgument( function, index, "expected duration specification, got number" );
            }
        };

        namespace detail {

            using CallArgument = boost::variant< std::string, std::intmax_t >;

            struct Call
            {
                std::string function;
                std::vector< CallArgument > arguments;
            };

            template< typename Type >
            struct ArgumentVisitor : boost::static_visitor< Type >
            {
                ArgumentVisitor( std::string const& function, std::size_t index )
                        : function_( function )
                        , index_( index )
                {
                }

                Type operator()( Type const& value ) const { return value; }

                template< typename Other >
                Type operator()( Other const& value ) const
                {
                    return ArgumentConverter< Type >::parse( function_, index_, value );
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

            Call parseCall( std::string const& text );

        } // namespace detail

        template< typename Base, typename ...Factories >
        std::unique_ptr< Base > parseCall( std::string const& text, Factories&&... factories )
        {
            return detail::create< Base >( detail::parseCall( text ), std::forward< Factories >( factories )... );
        }

    } // namespace expression

} // namespace sc

#endif //SCHLAZICONTROL_EXPRESSION_HPP