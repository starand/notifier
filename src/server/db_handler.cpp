#include <common/StdAfx.h>

#include <common/config.h>
#include <server/db_handler.h>
#include <server/queries.inc.h>

#include <db/mysql_connection.h>
#include <utils/strutils.h>

#include <json/value.h>


#define NOTIFICTION_FIELDS_COUNT    5


//--------------------------------------------------------------------------------------------------

db_handler_t::db_handler_t( const config_t& config )
    : m_config( config )
    , m_query( new mysql_query_t( ) )
{
}

//--------------------------------------------------------------------------------------------------

bool db_handler_t::init( )
{
    auto host_node = m_config[ "db" ][ "host" ];
    auto user_node = m_config[ "db" ][ "user" ];
    auto password_node = m_config[ "db" ][ "password" ];
    auto db_node = m_config[ "db" ][ "name" ];

    std::string host = host_node.isNull( ) ? "localhost" : host_node.asString( );

    if ( user_node.isNull( ) || password_node.isNull( ) || db_node.isNull( ) )
    {
        LOG_ERROR( "User name, password or db name not set" );
        return false;
    }

    std::string user = user_node.asString( );
    std::string password = password_node.asString( );
    std::string db = db_node.asString( );
    if ( user.empty( ) || password.empty( ) || db.empty( ) )
    {
        LOG_ERROR( "User name, password and db name cannot be empty" );
        return false;
    }

    if ( !m_query->init( user, password, db, host ) )
    {
        return false;
    }

    if ( !m_query->execute( g_use_utf8 ) )
    {
        LOG_ERROR( "Unable to initialize db to use UTF8" );
        return false;
    }

    LOG_INFO( "DB connection to %s@%s [%s]", user.c_str( ), host.c_str( ), db.c_str( ) );
    return true;
}

//--------------------------------------------------------------------------------------------------

bool db_handler_t::add_notification( const std::string& src, const std::string& msg,
                                     NotificationType type ) const
{
    std::string sql;
    StrUtils::FormatString( sql, g_insert_sql, src.c_str( ), msg.c_str( ), type );

    return m_query->execute( sql );
}

//--------------------------------------------------------------------------------------------------

bool db_handler_t::get_new_notifications( uint id,
                                          std::vector< notification_t >& notifications ) const
{
    std::string sql;
    StrUtils::FormatString( sql, g_get_new_msgs, id );

    CMySQLResult ms_result;
    if ( !m_query->execute( sql, ms_result ) || ms_result.NumFields( ) != NOTIFICTION_FIELDS_COUNT )
    {
        return false;
    }

    size_t count = ms_result.GetRowCount( );
    notifications.reserve( count );

    for ( size_t idx = 0; idx < count; ++idx )
    {
        notifications.emplace_back(
            atoi( ms_result.GetCellValue( idx, 0 ) ),
            ms_result.GetCellValue( idx, 1 ),
            ms_result.GetCellValue( idx, 2 ),
            ms_result.GetCellValue( idx, 3 ),
            static_cast< NotificationType >( atoi( ms_result.GetCellValue( idx, 4 ) ) )
        );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
