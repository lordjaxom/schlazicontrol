#include <iostream>
#include <stdexcept>

#include <getopt.h>

#include "commandline.hpp"
#include "utility.hpp"

using namespace std;

namespace sc {

	static char const* cmdLineMapShortToLong( int shortopt )
	{
		switch ( shortopt ) {
			case 'h': return "help";
			case 'p': return "properties";
			case 'l': return "log-file";
			case 'd': return "daemonize";
			default: throw invalid_argument( str( "cmdLineMapShortToLong( ", shortopt, ")" ) );
		}
	}

	CommandLine::CommandLine( char* const *argv, int argc )
		: propertiesFile_( "/etc/schlazicontrol.json" )
		, daemon_()
	{
		struct option options[] = {
			{ nullptr, no_argument,       nullptr, 'h' },
			{ nullptr, required_argument, nullptr, 'p' },
			{ nullptr, required_argument, nullptr, 'l' },
			{ nullptr, no_argument,       nullptr, 'd' },
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
		while ( ( optchar = getopt_long( argc, argv, ":hp:l:d", options, &optind ) ) != -1 ) {
			switch ( optchar ) {
				case ':':
					throw runtime_error( str( "error: missing argument to --", cmdLineMapShortToLong( optopt ) ) );

				case '?':
					throw runtime_error( str( "error: unknown option -", optopt ) );

				case 'h':
					throw runtime_error( str( "Usage: ", argv[ 0 ], " [...]" ) );

				case 'p':
					propertiesFile_ = optarg;
					break;

				case 'l':
					logFile_ = optarg;
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
