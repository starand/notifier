#pragma once


//--------------------------------------------------------------------------------------------------

static const std::string g_use_utf8 =
    "SET character_set_results = 'utf8', character_set_client = 'utf8', character_set_connection = "
    "'utf8', character_set_database = 'utf8', character_set_server = 'utf8'";
//--------------------------------------------------------------------------------------------------

static const char g_insert_sql[] = "INSERT INTO notifications VALUES(NULL, now(), '%s', '%s', 0 )";

//--------------------------------------------------------------------------------------------------

static const char g_get_new_msgs[] = "SELECT * FROM notifications WHERE n_id > %u";

//--------------------------------------------------------------------------------------------------
