#include <iostream>
#include <stdexcept>

#include <getopt.h>

#include "core/config.hpp"
#include "commandline.hpp"
#include "utility_string.hpp"

using namespace std;

namespace sc {

	static char const* cmdLineMapShortToLong( int shortopt )
	{
		switch ( shortopt ) {
			case 'h': return "help";
            case 'c': return "config-file";
			case 'l': return "log-file";
            case 'p': return "pid-file";
#if defined( SCHLAZICONTROL_FORK )
			case 'd': return "daemonize";
#endif
			default: throw invalid_argument( str( "cmdLineMapShortToLong( ", shortopt, ")" ) );
		}
	}

	CommandLine::CommandLine( char* const *argv, int argc )
		: propertiesFile_( "/etc/schlazicontrol.json" )
		, daemon_()
	{
		struct option options[] = {
			{ nullptr, no_argument,       nullptr, 'h' },
			{ nullptr, required_argument, nullptr, 'c' },
            { nullptr, required_argument, nullptr, 'l' },
            { nullptr, required_argument, nullptr, 'p' },
#if defined( SCHLAZICONTROL_FORK )
			{ nullptr, no_argument,       nullptr, 'd' },
#endif
			{}
		};

		for ( auto& option : options ) {
			if ( option.val != 0 ) {
				option.name = cmdLineMapShortToLong( option.val );
			}
		}

		opterr = 0;

		int optchar;
		int optind;
		while ( ( optchar = getopt_long( argc, argv, ":hc:l:p:d", options, &optind ) ) != -1 ) {
			switch ( optchar ) {
				case ':':
					throw runtime_error( str( "missing argument to --", cmdLineMapShortToLong( optopt ) ) );

				case '?':
					throw runtime_error( str( "unknown option -", (char) optopt ) );

				case 'h':
					throw runtime_error( str( "Usage: ", argv[ 0 ], " [...]" ) );

				case 'c':
					propertiesFile_ = optarg;
					break;

				case 'l':
					logFile_ = optarg;
					break;

                case 'p':
                    pidFile_ = optarg;
                    break;

				case 'd':
					daemon_ = true;
					break;

				default:
					throw invalid_argument( str( "getopt_long( ... ) -> '", optchar, "'" ) );
			}
		}
	}

} // namespace sc
