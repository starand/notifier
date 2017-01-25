#include <common/StdAfx.h>

#include <common/protocol.h>
#include <server/client_handler.h>
#include <server/db_handler.h>

#include <files/config.h>
#include <utils/strutils.h>

#include <json/writer.h>
#include <json/value.h>


#define CHECK_AND_BREAK_IF_FAIL( __res__ ) \
    if ( !__res__ ) { \
        LOG_ERROR( "Cannot send packet" ); \
        error = true; break; \
    }


//--------------------------------------------------------------------------------------------------

client_handler_t::client_handler_t( const config_t& config, const db_handler_t& db )
    : m_config( config )
    , m_db( db )
    , m_end_point( )
{
}

//--------------------------------------------------------------------------------------------------

client_handler_t::~client_handler_t( )
{
}

//--------------------------------------------------------------------------------------------------

void client_handler_t::process_client( socket_t& client )
{
    if ( !make_handshake( client ) )
    {
        return;
    }

    bool error = false;
    while ( !error )
    {
        RECV_PACKET_SWITCH( &client );
        RECV_PACKET_CASE( add_notification_t, req )
        {
            bool res = m_db.add_notification( req.src, req.msg, req.type )
                ? ok_t( ).send( client )
                : error_msg_t( "Cannot add notification" ).send( client );

            CHECK_AND_BREAK_IF_FAIL( res );
        }
        RECV_PACKET_CASE( disconnect_request_t, request )
        {
            LOG_DEBUG( "Client %s disconnected", m_end_point.c_str( ) );
            error = true;
            break;
        }
        RECV_PACKET_ERROR( message )
        {
            LOG_INFO( "Client %s disconnected (%s)", m_end_point.c_str( ), message );
            error = true;
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------

bool client_handler_t::make_handshake( socket_t& client )
{
    StrUtils::FormatString( m_end_point, "%s:%u",
                            client.get_remote_address( ), client.get_remote_port( ) );

    handshake_request_t hs_req;
    if ( !hs_req.recv( client ) )
    {
        LOG_ERROR( "Cannot receive handshake request from %s", m_end_point.c_str( ) );
        return false;
    }

    auto password_node = m_config[ "net" ][ "password" ];
    ASSERT( !password_node.isNull( ) );
    std::string password = password_node.asString( );
    ASSERT( !password.empty( ) );

    handshake_response_t hs_res;
    hs_res.state = hs_req.password != password ? EHS_INVALID_PASSWORD : EHS_OK;

    if ( !hs_res.send( client ) )
    {
        LOG_ERROR( "Cannot send handshake response to  %s", m_end_point.c_str( ) );
        return false;
    }

    LOG_DEBUG( "Handshake result with %s : %u", m_end_point.c_str( ), hs_res.state );
    return hs_res.state == EHS_OK;
}

//--------------------------------------------------------------------------------------------------
