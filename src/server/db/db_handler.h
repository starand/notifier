#pragma once

#include <common/protocol.h>
#include <common/enums.h>

#include <db/mysql_query.h>

#include <memory>


enum class CURRENCY
{
    USD = 0,
    EUR = 1
};

class config_t;

class db_handler_t
{
public:
    db_handler_t( const config_t& config );
    ~db_handler_t( ) = default;

public:
    bool init( );

    bool add_notification( const std::string& src,
                           const std::string& msg, NotificationType type ) const;

    bool get_new_notifications( uint id, std::vector< notification_t >& notifications ) const;

    bool add_currency( CURRENCY type, float rate, const std::string& time, uint count );
    const std::string get_last_currency_time( ) const;

private:
    const config_t& m_config;
    std::unique_ptr< mysql_query_t > m_query;

};
