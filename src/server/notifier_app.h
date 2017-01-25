#pragma once

#include <memory>


class client_handler_t;
class config_t;
class db_handler_t;
class logger_initializer_t;
class net_server_t;

class notifier_app_t
{
public:
    notifier_app_t( );

    int run( );

private:
    bool init( );
    bool init_logger( const std::string& binary_dir );
    bool load_config( const std::string& binary_dir );

#ifdef LINUX
    void install_signal_handlers( );
#endif

public:
    static bool m_stopping;

private:
    std::unique_ptr< logger_initializer_t > m_logger;
    std::unique_ptr< config_t > m_config;
    std::unique_ptr< db_handler_t > m_db;

    std::unique_ptr< client_handler_t > m_client_handler;
    std::unique_ptr< net_server_t > m_net_server;
};
