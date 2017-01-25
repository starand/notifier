#pragma once

#include <common/enums.h>
#include <serialize/struct_serializer_ms.h>

#include <memory>

#ifdef WINDOWS
#   define sprintf sprintf_s
#endif


static const char g_szUnableRecvDataFromSocket[] = "Could not receive data from socket";
static const char g_szInvalidPacketType[] = "Invalid packet type %u";
static char g_szErrorMsgBuffer[ 1024 ] = { 0 };


#define RECV_PACKET_SWITCH(_SOCKET_) \
	socket_t *_socket = _SOCKET_; \
	char *_error = NULL; \
	EPACKETMAGIC packet_magic = EPM_ERROR; \
	if (_socket->recv(&packet_magic, sizeof(EPACKETMAGIC)) != sizeof(EPACKETMAGIC)) \
                    { _error = (char*)g_szUnableRecvDataFromSocket; packet_magic = EPM_ERROR; } \
        else { \
	switch(packet_magic) { {

#define RECV_PACKET_CASE(_PACKET_TYPE_, _PACKET_) \
	break; } \
	case _PACKET_TYPE_::m_packet_magic: { \
	std::unique_ptr< _PACKET_TYPE_ > _packet_ptr(new _PACKET_TYPE_()); \
	_PACKET_TYPE_ &_PACKET_ = *_packet_ptr; \
	struct_serializer_t<_PACKET_TYPE_>::serializer_t serializer; \
    (void)serializer; \
	if (!serializer.recv(*_socket, _PACKET_)) \
        { _error = (char*)g_szUnableRecvDataFromSocket; packet_magic = EPM_ERROR; } \
    else

#define RECV_PACKET_CASE_EXISTS(_PACKET_TYPE_, _PACKET_) \
	break; } \
	case _PACKET_TYPE_::m_packet_magic: { \
	typename struct_serializer_t<_PACKET_TYPE_>::serializer_t serializer; \
    (void)serializer; \
	if (!serializer.recv(*_socket, _PACKET_)) \
        { _error = (char*)g_szUnableRecvDataFromSocket; packet_magic = EPM_ERROR; } \
    else

#define RECV_PACKET_ERROR(_ERROR_) \
	break; } \
	default: \
	sprintf(g_szErrorMsgBuffer, g_szInvalidPacketType, packet_magic); \
	_error = (char*)g_szErrorMsgBuffer; packet_magic = EPM_INVALIDMAGIC; \
	} /*switch*/ \
	} /*if*/ \
	const char *_ERROR_ = _error; \
	if (_ERROR_)


/*
Usage example :

socket_t *socket ... ;

RECV_PACKET_SWITCH(socket)
RECV_PACKET_CASE(mobile_handshake_request_t, request)
{
// TODO : process request
break;
}
RECV_PACKET_CASE(mobile_handshake_request_t, request)
{
// TODO : process request
break;
}
RECV_PACKET_ERROR()
{
// TODO : process error
}

*/


//--------------------------------------------------------------------------------------------------

class packet_intf_t
{
public:
    virtual bool send( socket_t &socket ) = 0;
    virtual bool recv( socket_t &socket ) = 0;

    virtual ~packet_intf_t( )
    {
    }
};

//--------------------------------------------------------------------------------------------------

template <typename type_>
struct packet_base_t : public packet_intf_t
{
    virtual ~packet_base_t( )
    {
    }

    bool send( socket_t &socket )
    {
        type_ *packet = static_cast<type_ *>( this );
        ASSERT( packet != NULL );

        EPACKETMAGIC packet_magic = type_::m_packet_magic;
        if ( socket.send( &packet_magic, sizeof( EPACKETMAGIC ) ) != sizeof( EPACKETMAGIC ) )
        {
            return false;
        }

        typename struct_serializer_t<type_>::serializer_t serializer;
        (void)serializer;
        if ( !serializer.send( socket, *packet ) )
        {
            return false;
        }

        return true;
    }

    bool recv( socket_t &socket )
    {
        EPACKETMAGIC packet_magic;
        if ( socket.recv( &packet_magic, sizeof( packet_magic ) ) != sizeof( packet_magic ) )
        {
            return false;
        }

        if ( packet_magic != type_::m_packet_magic )
        {
            return false;
        }

        type_ *packet = static_cast<type_ *>( this );
        ASSERT( packet != NULL );

        typename struct_serializer_t<type_>::serializer_t serializer;
        (void)serializer;
        if ( !serializer.recv( socket, *packet ) )
        {
            return false;
        }

        return true;
    }
};


//--------------------------------------------------------------------------------------------------

struct handshake_request_t : public packet_base_t < handshake_request_t >
{
    static const EPACKETMAGIC m_packet_magic = EPM_HANDSHAKE_REQUEST;

    std::string password;

    handshake_request_t( ) : password( ) { }
    handshake_request_t( const std::string& _pass ) : password( _pass ) { }
};

STRUCT_SERIALIZER_BEGIN( handshake_request_t )
STRUCT_SERIALIZER_FIELD( std::string, password )
STRUCT_SERIALIZER_END( )

//--------------------------------------------------------------------------------------------------

struct handshake_response_t : public packet_base_t < handshake_response_t >
{
    static const EPACKETMAGIC m_packet_magic = EPM_HANDSHAKE_RESPONSE;

    EHANDSHAKE_STATE state;
};

STRUCT_SERIALIZER_BEGIN( handshake_response_t )
STRUCT_SERIALIZER_FIELD( EHANDSHAKE_STATE, state )
STRUCT_SERIALIZER_END( )

//--------------------------------------------------------------------------------------------------

struct disconnect_request_t : public packet_base_t < disconnect_request_t >
{
    static const EPACKETMAGIC m_packet_magic = EPM_DISCONNECT_REQUEST;
};

STRUCT_SERIALIZER_BEGIN( disconnect_request_t )
STRUCT_SERIALIZER_END( )

//--------------------------------------------------------------------------------------------------

struct error_msg_t : public packet_base_t < error_msg_t >
{
    static const EPACKETMAGIC m_packet_magic = EPM_ERROR_MSG;

    std::string msg;

    error_msg_t( ) : msg( ) { }
    error_msg_t( const char* _msg ) : msg( _msg ) { }
};

STRUCT_SERIALIZER_BEGIN( error_msg_t )
STRUCT_SERIALIZER_FIELD( std::string, msg )
STRUCT_SERIALIZER_END( )

//--------------------------------------------------------------------------------------------------

struct ok_t : public packet_base_t < ok_t >
{
    static const EPACKETMAGIC m_packet_magic = EPM_OK;
};

STRUCT_SERIALIZER_BEGIN( ok_t )
STRUCT_SERIALIZER_END( )

//--------------------------------------------------------------------------------------------------

struct add_notification_t : public packet_base_t < add_notification_t >
{
    static const EPACKETMAGIC m_packet_magic = EPM_ADD_NOTIFICATION;

    std::string src;
    std::string msg;
    NotificationType type;

    add_notification_t( ) : src( ), msg( ), type( ) { }
    add_notification_t( const std::string& _src, const std::string& _msg, NotificationType _type )
        : src( _src ), msg( _msg ), type( _type )
    {
    }
};

STRUCT_SERIALIZER_BEGIN( add_notification_t )
STRUCT_SERIALIZER_FIELD( std::string, src )
STRUCT_SERIALIZER_FIELD( std::string, msg )
STRUCT_SERIALIZER_FIELD( NotificationType, type )
STRUCT_SERIALIZER_END( )

//--------------------------------------------------------------------------------------------------

struct notification_t : public packet_base_t < notification_t >
{
    static const EPACKETMAGIC m_packet_magic = EPM_NOTIFICATION;

    uint id;
    std::string time;
    std::string src;
    std::string msg;
    NotificationType type;

    notification_t( ) : id( ), time( ), src( ), msg( ), type( ) { }
    notification_t( uint _id, const std::string& _time, const std::string& _src,
                           const std::string& _msg, NotificationType _type )
        : id( _id ), time( _time ), src( _src ), msg( _msg ), type( _type )
    {
    }
};

STRUCT_SERIALIZER_BEGIN( notification_t )
STRUCT_SERIALIZER_FIELD( uint, id )
STRUCT_SERIALIZER_FIELD( std::string, time )
STRUCT_SERIALIZER_FIELD( std::string, src )
STRUCT_SERIALIZER_FIELD( std::string, msg )
STRUCT_SERIALIZER_FIELD( NotificationType, type )
STRUCT_SERIALIZER_END( )

//--------------------------------------------------------------------------------------------------
