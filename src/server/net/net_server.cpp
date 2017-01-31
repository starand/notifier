#include <common/StdAfx.h>

#include <common/config.h>
#include <net/client_handler.h>
#include <net/net_server.h>

#include <common/asserts.h>
#include <net/xsocket.h>

#include <json/value.h>


//--------------------------------------------------------------------------------------------------

net_server_t::net_server_t( const config_t& config, client_handler_t& handler )
    : m_socket( new socket_t( ) )
    , m_config( config )
    , m_handler( handler )
{
    ASSERT( m_socket.get( ) != nullptr );
}

//--------------------------------------------------------------------------------------------------

net_server_t::~net_server_t( )
{
}

//--------------------------------------------------------------------------------------------------

bool net_server_t::init( )
{
    auto password_node = m_config[ "net" ][ "password" ];
    if ( password_node.isNull( ) )
    {
        LOG_ERROR( "net::password node not set" );
        return false;
    }

    auto port_node = m_config[ "net" ][ "port" ];
    if ( port_node.isNull( ) )
    {
        LOG_ERROR( "net::port node not set" );
        return false;
    }

    ushort port = static_cast< ushort >( port_node.asUInt( ) );
    LOG_INFO( "Listen on %u port", port );

    return m_socket->listen( port );
}

//--------------------------------------------------------------------------------------------------

void net_server_t::do_run( )
{
    if ( !init( ) )
    {
        return;
    }

    while ( true )
    {
        std::unique_ptr< socket_t > client( m_socket->accept( ) );
        if ( is_stopping( ) )
        {
            break;
        }

        m_handler.process_client( *client );
    }
}

//--------------------------------------------------------------------------------------------------

void net_server_t::do_stop( )
{
    m_socket->shutdown( SD_BOTH );
    m_socket->close( );
}

//--------------------------------------------------------------------------------------------------

