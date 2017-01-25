#include <common/StdAfx.h>

#include <server/client_handler.h>
#include <server/db_handler.h>
#include <server/net_server.h>
#include <server/notifier_app.h>

#include <files/config.h>
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

    m_net_server->stop( );
    m_net_server->wait( );

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

//--------------------------------------------------------------------------------------------------
// SIGNAL HANDLERS
//--------------------------------------------------------------------------------------------------

#ifdef LINUX

static void handler( int sig )
{
    LOG_INFO( "Signal received. Stopping .." );
    notifier_app_t::m_stopping = true;
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
