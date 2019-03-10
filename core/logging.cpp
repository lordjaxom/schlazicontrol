#include <cstring>
#include <ctime>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>

#if !defined( WIN32 )
#   include <sys/types.h>
#   include <unistd.h>
#endif

#include "core/logging.hpp"

using namespace std;

namespace sc {

	namespace detail {

		ostream& logTimestamp( ostream& os )
		{
			auto timestamp = chrono::high_resolution_clock::now().time_since_epoch();
			auto seconds = chrono::duration_cast< chrono::seconds >( timestamp );
			auto micros = chrono::duration_cast< chrono::microseconds >( timestamp - seconds );
			time_t tt = seconds.count();
#if defined( WIN32 )
            tm* pTm = std::localtime( &tt ); tm& tm = *pTm;
#else
			tm tm; ::localtime_r( &tt, &tm );
#endif

			return os
					<< setw( 4 ) << setfill( '0' ) << ( tm.tm_year + 1900 ) << "/"
					<< setw( 2 ) << setfill( '0' ) << ( tm.tm_mon + 1 ) << "/"
					<< setw( 2 ) << setfill( '0' ) << tm.tm_mday << " "
					<< setw( 2 ) << setfill( '0' ) << tm.tm_hour << ":"
					<< setw( 2 ) << setfill( '0' ) << tm.tm_min << ":"
					<< setw( 2 ) << setfill( '0' ) << tm.tm_sec << "."
					<< setw( 6 ) << setfill( '0' ) << micros.count();
		}

		ostream& logPid( ostream& os )
		{
			return os << setw( 5 ) << setfill( ' ' ) << getpid();
		}

		template< size_t L >
		string logBuildTag( string_view rawTag )
		{
			if ( rawTag.length() == L ) {
				return rawTag.to_string();
			}

			string result;
			result.reserve( L );
			if ( rawTag.length() < L ) {
				result.append( ( L - rawTag.length() ) / 2, ' ' );
				result.append( rawTag.begin(), rawTag.end() );
				result.append( ( L - rawTag.length() ) - ( L - rawTag.length() ) / 2, ' ' );
			}
			else {
				result.append( "..." );
                auto abbrev = rawTag.substr( rawTag.length() - L + 3, L - 3 );
				result.append( abbrev.begin(), abbrev.end() );
			}
			return result;
		}

	} // namespace detail

	Logger::Level const Logger::Debug   { "DEBUG", 3 };
	Logger::Level const Logger::Info    { "INFO ", 2 };
	Logger::Level const Logger::Warning { "WARN ", 1 };
	Logger::Level const Logger::Error   { "ERROR", 0 };

	bool Logger::active_;
	Logger::Level const* Logger::level_ = &Logger::Debug;
    mutex Logger::mutex_;

	bool Logger::is( Logger::Level const& level )
	{
		return active_ && level_->level >= level.level;
	}

    shared_ptr< ostream >& Logger::output()
    {
        static shared_ptr< ostream > instance( &cerr, []( ostream const* ) {} );
        return instance;
    }

	void Logger::level( Logger::Level const& level )
	{
		level_ = &level;
	}

	void Logger::output( ostream& output )
	{
		Logger::output().reset( &output, []( ostream const* ) {} );
	}

	void Logger::output( char const* output )
	{
		Logger::output().reset( new ofstream( output, ios::out | ios::app ) );
	}

	Logger::Logger( char const* tag ) noexcept
		    : rawTag_( tag ) {}

    Logger::Logger( string tag )
            : tag_( detail::logBuildTag< tagLength >( tag ) ) {}

    string const& Logger::tag() const
    {
        if ( tag_ == nullopt ) {
            tag_ = detail::logBuildTag< tagLength >( rawTag_ );
        }
        return *tag_;
    }

} // namespace sc
