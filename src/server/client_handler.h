#pragma once


class config_t;
class db_handler_t;
class socket_t;


class client_handler_t
{
public:
    client_handler_t( const config_t& config, const db_handler_t& db );
    ~client_handler_t( );

public:
    void process_client( socket_t& client );

private:
    bool make_handshake( socket_t& client );

private:
    const config_t& m_config;
    const db_handler_t& m_db;
    std::string m_end_point;
};

