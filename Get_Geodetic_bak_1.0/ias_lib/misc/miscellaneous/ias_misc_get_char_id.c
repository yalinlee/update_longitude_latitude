/*****************************************************************************
NAME:           ias_misc_get_char_id

PURPOSE: ias_misc_get_char_id returns the work order common ID (char ID)
         to use for detailed characterization information to the database. 
         The work order common table name and column name are provided as 
         input args.

RETURNS: SUCCESS or ERROR

NOTES:   The char ID is defined as NUMBER(20,0) in the database which is
         too large to be returned as an integer, so we return a string
         representation of it.  This conversion is handled automatically
         by the database library.  The buffer provided by the caller
         must provide room for at least 20 characters plus a null 
         terminator (IAS_CHAR_ID_SIZE).
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_db.h"
#include "ias_miscellaneous.h"

#define SQL_QUERYLEN 256

/*----------------------------------------------------------------------------
NAME:    get_char_id (local)

PURPOSE: A local helper function that obtains the radiometric or geometric
         characterization ID from the database table for the product passed
         in. If the ID is not in the table, pass back an invalid value to
         trigger the creation of a new one.

RETURNS: SUCCESS the existing char ID was found and returned
         WARNING no existing char ID was found
         ERROR   an error occurred trying to get the ID from the database
-----------------------------------------------------------------------------*/
static int get_char_id
(
    struct ias_db_connection *dbase, /* I: Database to use */
    const char *table_name,          /* I: Work order common table to query */
    const char *column_name,         /* I: WO common ID column name to query */
    const char *work_order_id,       /* I: Work order ID to char */
    char *char_id                    /* O: Characterization ID obtained */
)
{
    int status;                     /* Status of called functions */
    int return_status;              /* Status returned from this function */
    char sql[SQL_QUERYLEN];         /* Buffer for SQL commands */
    struct ias_db_query *query = NULL;

    /* Query the database for the latest char ID if the work order has 
       been run before */
    status = snprintf(sql, sizeof(sql),
            "SELECT %s FROM %s where work_order_id = '%s' ORDER BY %s DESC;",
            column_name, table_name, work_order_id, column_name);
    if (status < 0 || status >= sizeof(sql))
    {
        IAS_LOG_ERROR("Preparing SQL query");
        return ERROR;
    }

    query = ias_db_perform_query(dbase, sql);
    if (!ias_db_query_was_successful(query))
    {
        char query_error_message[IAS_SQL_RTN_MSGLEN];
                                    /* Buffer for SQL error messages */

        ias_db_query_get_error_message(query, query_error_message,
            sizeof(query_error_message));
        ias_db_query_close(query);
        IAS_LOG_ERROR("Performing query: %s\n"
            "DB error message:  %s", sql, query_error_message);
        return ERROR;
    }

    /* Get the first row returned to get the char ID */
    return_status = SUCCESS;
    if (ias_db_query_next(query))
    {
        /* A row was returned, so read the char ID */
        ias_db_query_string_value(query, 0, char_id, IAS_CHAR_ID_SIZE);
        if (strcmp(char_id, "NULL") == 0)
        {
            return_status = ERROR;
            IAS_LOG_ERROR("%s in existing row in %s table for work order %s "
                "is NULL", column_name, table_name, work_order_id);
        }
    } 
    else 
    {
        /* Warning indicates everything worked, we just didn't find one */
        return_status = WARNING;
    }

    ias_db_query_close(query);
    query = NULL;

    return return_status;
}

/*----------------------------------------------------------------------------
NAME:    ias_misc_get_char_id

RETURNS: SUCCESS or ERROR
-----------------------------------------------------------------------------*/
int ias_misc_get_char_id
(
    struct ias_db_connection *dbase,   /* I: Database to use */
    const char *table_name,            /* I: Work order common table to query */
    const char *column_name,           /* I: Column name of WO common ID */
    const char *work_order_id,         /* I: Work order ID to char */
    char *char_id                      /* O: Characterization ID, as a string */
)
{
    int status;                         /* Function return value */
    char sql[SQL_QUERYLEN];             /* Buffer for SQL commands */
    char buffer[200];                   /* Buffer for SQL return messages */
    struct ias_db_query *query = NULL;
    char table_name_lc[200];

    /* Attempt to get the char ID from the database table */
    status = get_char_id(dbase, table_name, column_name, work_order_id, 
        char_id);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Get Characterization ID failed");
        return ERROR;
    }

    /* If not able to retrieve the characterization ID for the given work
       order ID, one wasn't generated for this work order yet; get a new ID
       from the database. */
    if (status == WARNING)
    {
        strncpy(table_name_lc, table_name, 200);
        ias_misc_convert_to_lowercase(table_name_lc);

        /* Build the correct SQL string */
        if (strcmp(table_name_lc, "radiometric_work_order_common") == 0)
        {
            status = snprintf(sql, sizeof(sql),
                    "SELECT DBSEQ_RADIOMETRIC_WO_COMMON_PK.nextval FROM dual;");
            if (status < 0 || status >= sizeof(sql))
            {
                IAS_LOG_ERROR("Preparing SQL statement for new radiometric "
                        "characterization ID");
                return ERROR;
            }
        }
        else if (strcmp(table_name_lc, "geometric_work_order_common") == 0)
        {
            status = snprintf(sql, sizeof(sql),
                    "SELECT DBSEQ_GEOMETRIC_WO_COMMON_PK.nextval FROM dual;");
            if (status < 0 || status >= sizeof(sql))
            {
                IAS_LOG_ERROR("Preparing SQL statement for new geometric "
                        "characterization ID");
                return ERROR;
            }
        }
        else
        {
            IAS_LOG_ERROR("Invalid table %s", table_name);
            return ERROR;
        }

        /* Do the query */
        query = ias_db_perform_query(dbase, sql);
        if (!ias_db_query_was_successful(query))
        {
            ias_db_query_get_error_message(query, buffer, sizeof(buffer));
            IAS_LOG_ERROR("WO common ID seq call failed: %s", buffer);
            ias_db_query_close(query);
            return ERROR;
        }

        /* Get the first row returned to get the char ID */
        if (ias_db_query_next(query))
        {
            /* A row was returned, so read the char ID as a string */
            ias_db_query_string_value(query, 0, char_id, IAS_CHAR_ID_SIZE);
            /* If there is a problem getting the sequence number as a string,
             * we may get an empty string back */
            if (strcmp(char_id, "") == 0)
            {
                ias_db_query_close(query);
                IAS_LOG_ERROR("Retrieving ID for %s as a string", table_name);
                return ERROR;
            }
        }

        ias_db_query_close(query);
        query = NULL;

        status = snprintf(sql, sizeof(sql), "INSERT INTO %s "
                "(%s, work_order_id, date_processed) "
                "values (%s, '%s', SYSDATE)", table_name, column_name, char_id,
                work_order_id);
        if (status < 0 || status >= sizeof(sql))
        {
            IAS_LOG_ERROR("Preparing SQL insert statement");
            return ERROR;
        }

        query = ias_db_perform_query(dbase, sql);
        if (!ias_db_query_was_successful(query))
        {
            ias_db_query_get_error_message(query, buffer, sizeof(buffer));
            IAS_LOG_ERROR("Inserting into database: %s", buffer);
            ias_db_query_close(query);
            return ERROR;
        }

        ias_db_query_close(query);
        query = NULL;
    }

    return SUCCESS;
}

