#pragma once

#include <memory>
#include <string>


class socket_t;

class web_page_t
{
public:
    web_page_t( const std::string& url );
    ~web_page_t( );

public:
    bool load( );
    const std::string& get_content( ) const;

private:
    bool connect( );
    void disconnect( );
    bool load_content( );
    bool send_request( );
    bool load_response( );
    bool load_headers( );
    bool load_page_body( );

private:
    const std::string m_url;

    std::unique_ptr< socket_t > m_socket;

    std::string m_server;
    std::string m_page;

    std::string m_content;
    uint m_content_length;
};
