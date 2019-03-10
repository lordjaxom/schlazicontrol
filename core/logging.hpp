#ifndef SCHLAZICONTROL_LOGGING_HPP
#define SCHLAZICONTROL_LOGGING_HPP

#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#include "utility_string.hpp"

namespace sc {

	namespace detail {

		std::ostream& logTimestamp( std::ostream& os );
		std::ostream& logPid( std::ostream& os );

		template< typename ...Args >
		std::string logBuildMessage( std::string const& tag, char const* level, Args&&... args )
		{
            return str( logTimestamp, " [", logPid, "] [", tag, "] [", level, "] ", std::forward< Args >( args )...,
                        "\n" );
		}

	} // namespace detail

	class LoggerScope;

	class Logger
	{
		friend class LoggerScope;

		struct Level
		{
			char const* name;
			unsigned level;
		};

		static constexpr std::size_t tagLength = 15;

		static bool is( Level const& level );

		static bool active_;
		static Level const* level_;
		static std::shared_ptr< std::ostream > output_;

	public:
		static Level const Debug;
		static Level const Info;
		static Level const Warning;
		static Level const Error;

		static void level( Level const& level );
		static void output( std::ostream& output );
		static void output( char const* output );

        explicit Logger( std::string tag );
		Logger( Logger const& ) = delete;

		template< typename ...Args >
		void debug( Args&&... args )
		{
			log( Debug, std::forward< Args >( args )... );
		}

		template< typename ...Args >
		void info( Args&&... args )
		{
			log( Info, std::forward< Args >( args )... );
		}

		template< typename ...Args >
		void warning( Args&&... args )
		{
			log( Warning, std::forward< Args >( args )... );
		}

		template< typename ...Args >
		void error( Args&&... args )
		{
			log( Error, std::forward< Args >( args )... );
		}

	private:
		template< typename ...Args >
		void log( Level const& level, Args&&... args )
		{
			if ( is( level ) ) {
				std::string message =
						detail::logBuildMessage( tag_, level.name, std::forward< Args >( args )... );
				output_->write( message.c_str(), message.length() );
				output_->flush();
			}
		}

		std::string tag_;
	};

	class LoggerScope
	{
	public:
		LoggerScope();
		LoggerScope( LoggerScope const& ) = delete;
		LoggerScope( LoggerScope&& other );
		~LoggerScope();

	private:
		bool owned_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_LOGGING_HPP
