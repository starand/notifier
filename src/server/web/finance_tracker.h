#pragma once

#include <threading/threading.h>

#include <memory>


class db_handler_t;
class web_page_t;

class finance_tracker_t : public thread_base_t
{
public:
    finance_tracker_t( db_handler_t& db );
    ~finance_tracker_t( );

public: // thread_base_t
    virtual void do_run( );
    virtual void do_stop( );

private:
    bool update_data( );
    void parse_data( );
    void parse_row( const std::string& row, const std::string& last_time );

private:
    db_handler_t& m_db;
    std::unique_ptr< web_page_t > m_web_page;
};
