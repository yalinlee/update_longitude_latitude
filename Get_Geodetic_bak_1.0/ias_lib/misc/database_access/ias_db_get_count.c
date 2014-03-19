/******************************************************************************
NAME:           ias_db_get_count.c

PURPOSE:
ias_db_get_count returns the record count from the named table 
  where the values in the provided column name equals the provided value

RETURN VALUE:
Type = int
Value    Description
-----    -----------
int >=0  Number of records matching the table name and key provided.
int -1   An error occurred with the query


******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ias_db.h"
#include "ias_db_insert.h"
#include "ias_logging.h"
#include "ias_const.h"

#include "ias_db_query.h"

#define SQL_QUERYLEN 256

int ias_db_get_count
(
    struct ias_db_connection *db,       /* db connection handle */
    const char *table_name,             /* query table */
    const char *column_name,            /* query column */
    const char *value                   /* where clause item */
)
{
    int status;                            /* function return value */
    int rec_count = 0;                     /* query return count */
    char sql[SQL_QUERYLEN];                /* query string buffer */
    char db_error_msg[IAS_SQL_RTN_MSGLEN]; /* query return msg buffer */

    struct ias_db_query *query = NULL;

    /* build query string */
    status = snprintf(sql, sizeof(sql), "SELECT count(*) FROM %s Where %s = %s",
                 table_name, column_name, value);
    if (status < 0 || status > sizeof(sql))
    {
        IAS_LOG_ERROR("Query is too long");
        return ERROR;
    }

    /* do the query */
    query = ias_db_perform_query(db, sql);
    if (!ias_db_query_was_successful(query))
    {
        ias_db_query_get_error_message(query, db_error_msg,
                                       sizeof(db_error_msg));
        IAS_LOG_ERROR("Get Record Count Failed For Query: %s, ERROR: %s",
                        sql, db_error_msg);
        ias_db_query_close(query);
        return -1;
    }

    /* get the first row returned to get the return count */
    if (ias_db_query_next(query))
    {
        /* a row was returned, so read the return value  */
         status = ias_db_query_int_value(query, &rec_count, 0);
         if (status != SUCCESS)
         {
            IAS_LOG_ERROR("Query int value failed to return count");
            ias_db_query_close(query);
            return -1;
         }
    }

    ias_db_query_close(query);
    query = NULL;

    return rec_count;
}
