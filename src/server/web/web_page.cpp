#include <common/StdAfx.h>

#include <web/web_page.h>

#include <net/xsocket.h>
#include <utils/strutils.h>

#include <iostream>


#define BUFFER_SIZE 1024

//--------------------------------------------------------------------------------------------------

namespace
{

//--------------------------------------------------------------------------------------------------

const std::string get_host_page( const std::string& url, std::string& page )
{
    auto pos = url.find( "://" );
    std::string result = ( pos == std::string::npos ) ? url : url.substr( pos + 3 );

    pos = result.find_first_of( "/&?#" );
    if ( pos != std::string::npos )
    {
        page = result.substr( pos );
        result = result.substr( 0, pos );
    }
    else
    {
        page.clear( );
    }

    return result;
}

//--------------------------------------------------------------------------------------------------

uint get_content_length( const std::string& headers )
{
    auto pos = headers.find( "Content-Length: " );
    if ( pos == std::string::npos )
    {
        return 0;
    }

    pos += 16;
    auto end_pos = headers.find( "\r\n", pos );
    if ( end_pos == std::string::npos )
    {
        return 0;
    }

    return (uint)atoi( headers.substr( pos, end_pos - pos ).c_str( ) );
}

//--------------------------------------------------------------------------------------------------

} // anonymous namespace

//--------------------------------------------------------------------------------------------------

web_page_t::web_page_t( const std::string& url )
    : m_url( url )
    , m_socket( new socket_t( ) )
    , m_server( )
    , m_page( )
{
}

//--------------------------------------------------------------------------------------------------

web_page_t::~web_page_t( )
{
}

//--------------------------------------------------------------------------------------------------

bool web_page_t::load( )
{
    if ( !connect( ) )
    {
        return false;
    }

    bool result = load_content( );

    disconnect( );

    return result;
}

//--------------------------------------------------------------------------------------------------

const std::string& web_page_t::get_content( ) const
{
    return m_content;
}

//--------------------------------------------------------------------------------------------------

bool web_page_t::connect( )
{
    m_socket.reset( new socket_t( ) );
    m_server = get_host_page( m_url, m_page );

    if ( !m_socket->connect( m_server, 80 ) )
    {
        LOG_ERROR( "Cannot connect \'%s:80\'", m_server.c_str( ) );
        return false;
    }

    //LOG_TRACE( "Connected to \'%s:80\'", m_server.c_str( ) );
    return true;
}

void web_page_t::disconnect( )
{
    if ( m_socket->get( ) )
    {
        m_socket->shutdown( );
        m_socket->close( );
    }
}

//--------------------------------------------------------------------------------------------------

bool web_page_t::load_content( )
{
    if ( !send_request( ) )
    {
        return false;
    }

    if ( !load_response( ) )
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------

bool web_page_t::send_request( )
{
    ASSERT( m_socket.get( ) != nullptr );

    std::string req = StrUtils::FormatString( "GET %s HTTP/1.1\r\nHost:%s\r\n\r\n",
                                              m_page.c_str( ), m_server.c_str( ) );
    //LOG_TRACE( "Sending request: %s", req.c_str( ) );

    if ( !m_socket->send( (void*)req.c_str( ), req.length( ) ) )
    {
        LOG_ERROR( "Cannot sent request to %s", m_server.c_str( ) );
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------

bool web_page_t::load_response( )
{
    return load_headers( ) && load_page_body( );
}

//--------------------------------------------------------------------------------------------------

bool web_page_t::load_headers( )
{
    char response_buffer[ BUFFER_SIZE ] = { 0 };
    if ( !m_socket->recv( response_buffer, BUFFER_SIZE - 1 ) )
    {
        LOG_ERROR( "Cannot recv response from %s", m_server.c_str( ) );
        return false;
    }

    std::string headers( response_buffer );
    m_content_length= get_content_length( headers );
    if ( m_content_length == 0 )
    {
        LOG_ERROR( "Cannot get content length" );
        return false;
    }

    auto pos = headers.find( "\r\n\r\n" );
    if ( pos == std::string::npos )
    {
        LOG_ERROR( "Cannot get end of headers" );
        return false;
    }

    pos += 4;
    m_content.append( headers.substr( pos ) );
    return true;
}

//--------------------------------------------------------------------------------------------------

 bool web_page_t::load_page_body( )
 {
     char response_buffer[ BUFFER_SIZE ] = { 0 };

     while ( m_content.length() < m_content_length )
     {
        auto left = m_content_length - m_content.length( );
        uint size = BUFFER_SIZE - 1 > left ? left : BUFFER_SIZE - 1;

        //LOG_TRACE( "Left: %u, Size: %u", left, size );
        if ( !m_socket->recv( response_buffer, size ) )
        {
            LOG_ERROR( "Cannot recv response from %s", m_server.c_str( ) );
            return false;
        }

        m_content.append( response_buffer );
     }

     return true;
 }

 //--------------------------------------------------------------------------------------------------
