#pragma once


//--------------------------------------------------------------------------------------------------

enum NotificationType
{
    NT__MIN,

    NT_INFO = NT__MIN,
    NT_WARNING,
    NT_ERROR,

    NT__MAX,
    NT_UNKNOWN = NT__MAX
};

//--------------------------------------------------------------------------------------------------

enum EPACKETMAGIC
{
    EPM__MIN,

    EPM_HANDSHAKE_REQUEST = EPM__MIN,
    EPM_HANDSHAKE_RESPONSE,
    EPM_DISCONNECT_REQUEST,

    EPM_ADD_NOTIFICATION,
    EPM_NOTIFICATION,

    EPM_OK,
    EPM_ERROR_MSG,

    EPM__MAX,
    EPM_ERROR = EPM__MAX,
    EPM_INVALIDMAGIC,
};

//--------------------------------------------------------------------------------------------------

enum EHANDSHAKE_STATE
{
    EHS_OK,

    EHS_INVALID_PASSWORD,
    EHS_SERVER_ERROR
};

//--------------------------------------------------------------------------------------------------
