#include <fstream>
#include <iostream>

#include "core/config.hpp"
#include "commandline.hpp"
#include "core/logging.hpp"
#include "manager.hpp"

#if SCHLAZICONTROL_FORK
#   include <sys/stat.h>
#   include <sys/types.h>
#   include <unistd.h>
#endif

using namespace std;
using namespace sc;

static void writePidToFile( string const& pidFile )
{
    ofstream( pidFile, ios::out | ios::trunc ) << ::getpid() << "\n";
}

static void daemonize()
{
#if SCHLAZICONTROL_FORK
    pid_t pid;

    pid = ::fork();
    if ( pid > 0 ) {
        exit( EXIT_SUCCESS );
    }
    if ( pid != -1 && ::setsid() != -1 ) {
        pid = fork();
        if ( pid > 0 ) {
            exit( EXIT_SUCCESS );
        }
        if ( pid != -1 && ::chdir( "/" ) != -1 ) {
            ::umask( 0 );
            for ( int fd = sysconf( _SC_OPEN_MAX ) ; fd > 0 ; --fd ) {
                ::close( fd );
            }
            return;
        }
    }
#endif

    cerr << "couldn't daemonize: " << strerror( errno ) << "\n";
    exit( EXIT_FAILURE );
}

static ManagerProcess runMainLoopOrForkProcess( int argc, char* argv[] )
{
    Logger logger( "schlazicontrol" );

    try {
        CommandLine commandLine( argv, argc );
        if ( commandLine.daemon() ) {
            daemonize();
        }
        if ( !commandLine.pidFile().empty() ) {
            writePidToFile( commandLine.pidFile() );
        }
        if ( !commandLine.logFile().empty()) {
            Logger::output( commandLine.logFile().c_str());
        }

        logger.info( "schlazicontrol starting" );
        Manager manager( commandLine );

        logger.debug( "collecting sub-processes" );
        if ( ManagerProcess process = manager.forkProcesses() ) {
            return process;
        }

        logger.info( "setup finished, running..." );
        manager.run();
        logger.info( "schlazicontrol exiting" );
    }
    catch ( exception const& e ) {
        logger.error( e.what() );
    }
    return {};
}

static void runProcess( ManagerProcess const& process )
{
    Logger logger( process.name() );

    try {
        logger.info( process.name(), " starting" );
        process();
        logger.info( process.name(), " exiting" );
    }
    catch ( exception const& e ) {
        logger.error( e.what() );
    }
}

int main( int argc, char* argv[] )
{
    LoggerScope loggerScope;
    if ( ManagerProcess process = runMainLoopOrForkProcess( argc, argv ) ) {
        runProcess( process );
    }
}
