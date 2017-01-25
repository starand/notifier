#pragma once

#include <threading/threading.h>

#include <memory>


class client_handler_t;
class config_t;
class socket_t;


class net_server_t : public thread_base_t
{
public:
    net_server_t( const config_t& config, client_handler_t& handler );
    ~net_server_t( );

public:
    virtual void do_run( );
    virtual void do_stop( );

public:
    bool init( );

private:
    std::unique_ptr< socket_t > m_socket;
    client_handler_t& m_handler;

    const config_t& m_config;
};
