#pragma once

#include <files/config.h>


class config_t : public FileUtils::config_t
{
public:
    config_t( );
    ~config_t( ) = default;

    bool do_parse( );

public:
    const Json::Value& operator[]( const std::string& name ) const;

private:

};
