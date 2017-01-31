#include <common/StdAfx.h>

#include <app/notifier_app.h>
#include <common/config.h>
#include <db/db_handler.h>
#include <net/client_handler.h>
#include <net/net_server.h>
#include <web/finance_tracker.h>

#include <files/fileutils.h>
#include <utils/utils.h>

#include <json/value.h>

#ifdef LINUX
#   include <signal.h>
#   include <unistd.h>
#endif


#define SLEEP_TIMEOUT   1000


static const char g_szLoggerConfigDef[] = "logger.cfg";
static const char g_szLoggerSourceName[] = "notifier";
static const char g_szConfigFileDef[] = "notifier.cfg";

bool notifier_app_t::m_stopping = false;


//--------------------------------------------------------------------------------------------------

notifier_app_t::notifier_app_t( )
    : m_logger( )
    , m_config( new config_t( ) )
    , m_db( new db_handler_t( *m_config ) )
    , m_client_handler( new client_handler_t( *m_config, *m_db ) )
    , m_net_server( new net_server_t( *m_config, *m_client_handler ) )
    , m_finance_tracker( new finance_tracker_t( *m_db ) )
{
    ASSERT( m_config.get( ) != nullptr );
    ASSERT( m_db.get( ) != nullptr );
    ASSERT( m_client_handler.get( ) != nullptr );
    ASSERT( m_net_server.get( ) != nullptr );
}

//--------------------------------------------------------------------------------------------------

int notifier_app_t::run( )
{
    if ( !init( ) )
    {
        return 1;
    }

    if ( !m_db->init( ) )
    {
        return 2;
    }

    if ( !m_net_server->start( ) )
    {
        return 3;
    }

#ifdef LINUX
    install_signal_handlers( );
#endif

    while ( !m_stopping )
    {
        utils::sleep_ms( SLEEP_TIMEOUT );
        // TODO: heartbeat ?
    }

    finalize( );

    return 0;
}

//--------------------------------------------------------------------------------------------------

bool notifier_app_t::init( )
{
    std::string binary_dir;
    if ( !FileUtils::GetBinaryDir( binary_dir ) )
    {
        LOG_FATAL( "Cannot get binary path" );
        return false;
    }

    if ( !start_finance_tracker( ) )
    {
        LOG_FATAL( "Cannot start finance tracker" );
        return false;
    }

    return init_logger( binary_dir ) && load_config( binary_dir );
}

//--------------------------------------------------------------------------------------------------

bool notifier_app_t::init_logger( const std::string& binary_dir )
{
    const string logger_config_filename = binary_dir + g_szLoggerConfigDef;

    m_logger.reset( new logger_initializer_t( "gold", logger_config_filename.c_str( ) ) );
    ASSERT( m_logger.get( ) != nullptr );

    return true;
}

//--------------------------------------------------------------------------------------------------

bool notifier_app_t::load_config( const std::string& binary_dir )
{
    const string config_file = binary_dir + g_szConfigFileDef;
    return m_config->read_config( config_file );
}

//--------------------------------------------------------------------------------------------------

bool notifier_app_t::start_finance_tracker( )
{
    ASSERT( m_finance_tracker.get( ) != nullptr );
    return m_finance_tracker->start( );
}

//--------------------------------------------------------------------------------------------------

void notifier_app_t::finalize()
{
    stop_finance_tracker( );
    stop_net_server( );
}

//--------------------------------------------------------------------------------------------------

void notifier_app_t::stop_net_server()
{
    m_net_server->stop( );
    m_net_server->wait( );
}

//--------------------------------------------------------------------------------------------------

void notifier_app_t::stop_finance_tracker()
{
    LOG_TRACE( "Stopping finance tracker" );

    m_finance_tracker->stop( );
    m_finance_tracker->wait( );
}

//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// SIGNAL HANDLERS
//--------------------------------------------------------------------------------------------------

#ifdef LINUX

static void handler( int sig )
{
    LOG_INFO( "Signal received (%i). Stopping ..", sig );
    notifier_app_t::m_stopping = true;
}

static void ignore_handler( int sig )
{
    LOG_INFO( "Signal ignored (%i)", sig );
}
//--------------------------------------------------------------------------------------------------

void notifier_app_t::install_signal_handlers( )
{
    struct sigaction sa;
    sigemptyset( &sa.sa_mask );
    sa.sa_flags = 0;
    sa.sa_handler = handler;

    int res = sigaction( SIGINT, &sa, NULL );
    ASSERT( res != -1 );
    res = sigaction( SIGTERM, &sa, NULL );
    ASSERT( res != -1 );

    sa.sa_handler = ignore_handler;
    res = sigaction( SIGHUP, &sa, NULL );
    ASSERT( res != -1 );
}

#endif // LINUX

//--------------------------------------------------------------------------------------------------
// MAIN FUNCTION
//--------------------------------------------------------------------------------------------------

int main( )
{
    return notifier_app_t( ).run( );
}

//--------------------------------------------------------------------------------------------------
