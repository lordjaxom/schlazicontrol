#ifndef SCHLAZICONTROL_LOGGING_HPP
#define SCHLAZICONTROL_LOGGING_HPP

#include <cstddef>
#include <memory>
#include <mutex>
#include <ostream>
#include <utility>

#include "utility/optional.hpp"
#include "utility/string_view.hpp"

namespace sc {

	namespace detail {

		std::ostream& logTimestamp( std::ostream& os );
		std::ostream& logPid( std::ostream& os );

		inline void logWrite( std::ostream &os )
		{
			os << std::endl;
		}

		template< typename Arg0, typename ...Args >
		void logWrite( std::ostream& os, Arg0&& arg0, Args&&... args )
		{
			os << std::forward< Arg0 >( arg0 );
			logWrite( os, std::forward<Args>( args )... );
		}

		template< typename ...Args >
		void logMessage( std::ostream& os, std::string const& tag, char const* level, Args&&... args )
		{
			logWrite( os, logTimestamp, " [", logPid, "] [", tag, "] [", level, "] ", std::forward< Args >( args )... );
		}

        std::shared_ptr< std::ostream >& logOutput();

	} // namespace detail

	class Logger
	{
        using Lock = std::lock_guard< std::mutex >;

		struct Level
		{
			char const* name;
			unsigned level;
		};

		static constexpr std::size_t tagLength = 15;

		static bool is( Level const& level );

        static std::shared_ptr< std::ostream >& output();

		static bool active_;
		static Level const* level_;
        static std::mutex mutex_;

	public:
		static Level const Debug;
		static Level const Info;
		static Level const Warning;
		static Level const Error;

		static void level( Level const& level );
		static void output( std::ostream& output );
		static void output( char const* output );

        explicit Logger( char const* tag ) noexcept;
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
        std::string const& tag() const;

		template< typename ...Args >
		void log( Level const& level, Args&&... args )
		{
			if ( is( level ) ) {
                Lock lock( mutex_ );
                detail::logMessage( *output(), tag(), level.name, std::forward< Args >( args )... );
			}
		}

		string_view rawTag_;
        mutable optional< std::string > tag_;
	};

} // namespace sc

#endif // SCHLAZICONTROL_LOGGING_HPP
