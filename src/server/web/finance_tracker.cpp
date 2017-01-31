#include <common/StdAfx.h>

#include <db/db_handler.h>
#include <web/finance_tracker.h>
#include <web/web_page.h>

#include <ctime>


#define FINANCE_I_UA "http://finance.i.ua/market/lvov/usd/?type=2"

#define CHECK_TIMEOUT 60000
#define CONNECTION_TIMEOUT  300000

const std::string ROW_START_KEY = "<td><time class=\"a icon i_expanditem\">";
const std::string ROW_END_KEY = "</td></tr>";
const std::string CELL_START_KEY = "<td>";
const std::string CELL_END_KEY = "</";

const std::string EXPIRED_TAG = "class=\"-expired\"";

//--------------------------------------------------------------------------------------------------

namespace
{
//--------------------------------------------------------------------------------------------------

std::string get_date( )
{
    time_t rawtime;
    time( &rawtime );

    struct tm* timeinfo = localtime( &rawtime );
    
    char buffer[80];
    strftime( buffer, 80, "%Y-%m-%d", timeinfo );

    return std::string( buffer );
}
//--------------------------------------------------------------------------------------------------

bool is_time_greater( const std::string& last_time, const std::string& curr_time )
{
    std::string currency_time = get_date( ) + " " + curr_time;
    
    auto check_time_part = [=]( uint start, uint end )
    {
        uint par_a = atoi( currency_time.substr( start, end ).c_str( ) );
        uint par_b = atoi( last_time.substr( start, end ).c_str( ) );
        
        if ( par_a > par_b ) return 1;
        if ( par_a == par_b ) return 0;
        return -1;
    };

    // last time format - 2017-01-30 12:35:00
    int check = check_time_part( 0, 4 ); // year
    if ( check > 0 ) return true; else if ( check < 0 ) return false;
    check = check_time_part( 5, 2 ); // month
    if ( check > 0 ) return true; else if ( check < 0 ) return false;
    check = check_time_part( 8, 2 ) ; // day
    if ( check > 0 ) return true; else if ( check < 0 ) return false;
    check = check_time_part( 11, 2 ); // hour
    if ( check > 0 ) return true; else if ( check < 0 ) return false;
    check = check_time_part( 14, 2 ); // minute
    if ( check > 0 ) return true; else if ( check < 0 ) return false;
    return false;
}

//--------------------------------------------------------------------------------------------------
} // namespace anonymous

//--------------------------------------------------------------------------------------------------

finance_tracker_t::finance_tracker_t( db_handler_t& db )
    : m_db( db )
    , m_web_page( new web_page_t( FINANCE_I_UA ) )
{
}

//--------------------------------------------------------------------------------------------------

finance_tracker_t::~finance_tracker_t( )
{
}

//--------------------------------------------------------------------------------------------------

void finance_tracker_t::do_run( )
{
    while ( !is_stopping( ) )
    {
        LOG_TRACE( "." );
        bool updated = update_data( );
        sleep( updated ? CHECK_TIMEOUT : CONNECTION_TIMEOUT );
    }
}

//--------------------------------------------------------------------------------------------------

void finance_tracker_t::do_stop( )
{
}

//--------------------------------------------------------------------------------------------------

bool finance_tracker_t::update_data( )
{
    if ( !m_web_page->load() )
    {
        return false;
    }

    parse_data( );

    return true;
}

//--------------------------------------------------------------------------------------------------

void finance_tracker_t::parse_data( )
{
    // delete everything after 'expired' tag appered
    auto pos = m_web_page->get_content( ).find( EXPIRED_TAG );
    std::string data = m_web_page->get_content( ).substr( 0, pos );

    const auto last_time = m_db.get_last_currency_time( );

    pos = 0;
    while ( std::string::npos != ( pos = data.find( ROW_START_KEY, pos ) ) )
    {
        pos += ROW_START_KEY.length( );
        auto endpos = data.find( ROW_END_KEY, pos );

        if ( endpos == std::string::npos )
        {
            LOG_ERROR( "Cannot find end of row" );
            break;
        }
        
        parse_row( data.substr( pos, endpos - pos ), last_time );

        pos = endpos + ROW_END_KEY.length( );
    }
}

//--------------------------------------------------------------------------------------------------

#define CHECK_ROW_RETURN( _VAL_ ) \
    if ( _VAL_ == std::string::npos ) { \
        LOG_ERROR( "Incorrect row: \'%s\'", row.c_str( ) ); \
        return; \
    }

//--------------------------------------------------------------------------------------------------

void finance_tracker_t::parse_row( const std::string& row, const std::string& last_time )
{
    auto pos = row.find( CELL_END_KEY );
    CHECK_ROW_RETURN( pos );

    auto time = row.substr( 0, pos );
    if ( !is_time_greater( last_time, time ) )
    {
        // skipping, time should already be in db
        //LOG_TRACE( "Skip: %s", time.c_str( ) );
        return;
    }

    pos = row.find( CELL_START_KEY, pos + CELL_END_KEY.length( ) );
    CHECK_ROW_RETURN( pos );
    pos += CELL_START_KEY.length( );
    auto endpos = row.find( CELL_END_KEY, pos );
    CHECK_ROW_RETURN( endpos );
    auto rate  = atof( row.substr( pos, endpos - pos ).c_str( ) );

    pos = row.find( CELL_START_KEY, endpos + CELL_END_KEY.length( ) );
    CHECK_ROW_RETURN( pos );
    pos += CELL_START_KEY.length( );
    endpos = row.find( CELL_END_KEY, pos );
    CHECK_ROW_RETURN( endpos );
    auto amount = atoi( row.substr( pos, endpos - pos - 2 ).c_str( ) );

    if ( m_db.add_currency( CURRENCY::USD, rate, time, amount ) )
    {
        LOG_TRACE( "Currency added: time %s, rate %f, amount %u", time.c_str( ), rate, amount );
    }
}

//--------------------------------------------------------------------------------------------------
