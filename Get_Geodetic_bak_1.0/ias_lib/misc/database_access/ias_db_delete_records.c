/******************************************************************************
NAME:           ias_db_delete_records.c

PURPOSE: ias_db_delete_records deletes all records from the named table 
  where the provided column name equals the provided value (exact match is 
  required)

RETURN VALUE: SUCCESS or ERROR


******************************************************************************/
#include <stdio.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_db.h"
#include "ias_db_query.h"

#define SQL_QUERYLEN 256

int ias_db_delete_records
(
    struct ias_db_connection *db,       /* db connection handle */
    const char *table_name,             /* query table */
    const char *column_name,            /* query column */
    const char *value                   /* where clause item */
)
{
    char sql[SQL_QUERYLEN];                /* query string buffer */
    char db_error_msg[IAS_SQL_RTN_MSGLEN]; /* query return msg buffer */

    int status;
    struct ias_db_query *query = NULL;

    /* build query string */
    status = snprintf(sql, sizeof(sql), "DELETE FROM %s Where %s = %s",
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
        IAS_LOG_ERROR("Record delete from %s failed:  %s", 
                       table_name, db_error_msg);
        ias_db_query_close(query);
        return ERROR;
    }

    ias_db_query_close(query);
    query = NULL;

    return SUCCESS;
}
