#include <common/StdAfx.h>
#include <common/config.h>

#include <json/reader.h>


//--------------------------------------------------------------------------------------------------

config_t::config_t( )
{
}

//--------------------------------------------------------------------------------------------------

bool config_t::do_parse( )
{
    ASSERT( !m_json->isNull( ) );

    // TODO: add some custom parsing
    return true;
}

//--------------------------------------------------------------------------------------------------


const Json::Value& config_t::operator[]( const std::string& name ) const
{
    return ( *m_json )[ name ];
}

//--------------------------------------------------------------------------------------------------

