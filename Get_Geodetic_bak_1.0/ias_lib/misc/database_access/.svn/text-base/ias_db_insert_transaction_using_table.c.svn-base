/******************************************************************************
NAME: ias_db_insert_transaction_using_table

PURPOSE:        
This function performs the entire transaction to remove old records from the 
database for the work order and inserts the new data using a table defined
by the calling routine.  It allows applications to be isolated from the actual 
mechanics of doing database inserts in case the method needs to change.

RETURNS: SUCCESS or ERROR

NOTES:
none

ALGORITHM REFERENCES:
none

******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_db_insert.h"
#include "ias_db_insert_transaction_using_table.h"

int ias_db_insert_transaction_using_table
(
    const char *database_name,     /* I: database name for inserts */
    const char *database_user,     /* I: database user name */
    const char *database_password, /* I: database password */
    const char *database_host,     /* I: database host */
    const char *database_table,    /* I: database table for the transaction */
    const IAS_DB_INSERT_TABLE* insert_table,/* I: table defining the data to be
                                                  inserted into the database */
    int insert_table_length,       /* I: number of entries in insert_table */
    int num_records,               /* I: The number of elements in any 
                                         members of the table struct
                                         that are declared as array types.
                                         If there are no arrays, then this
                                         argument must be 1 */
    const char *field_name,       /* I: field name to key on when deleting 
                                        previously existing rows */
    const char *field_value       /* I: value of the above field used to 
                                        identify rows to delete.  if a string,
                                        quotes should be included in the value*/
)
{
    int status;
    char buf[200];
    struct ias_db_connection *db = NULL;
    struct ias_db_query *query = NULL;

    db = ias_db_connect_with_transaction(database_name, database_user,
            database_password, database_host);

    if (db == NULL)
    {
        IAS_LOG_ERROR("Unable to connect to the database");
        return ERROR;
    }

    if (field_name != NULL && field_value != NULL)
    {
        snprintf(buf, sizeof(buf), "delete from %s where %s=%s", 
                database_table, field_name, field_value);
        query = ias_db_perform_query(db, buf);
        if (!ias_db_query_was_successful(query))
        {
            IAS_LOG_ERROR("Deleting previous records from %s where"
                    "%s=%s\n", database_table, field_name, field_value);
            ias_db_disconnect_with_transaction(db, IAS_DB_ROLLBACK);
            return ERROR;
        }
        ias_db_query_close(query);
        query = NULL;
    } 
    else if (field_name != NULL || field_value != NULL)
    {
        IAS_LOG_WARNING("Previous records were not deleted -- "
            "both field name and field value must be provided");
    }

    /* perform the insert */
    status = ias_db_insert_using_table(db, database_table, insert_table, 
            insert_table_length, num_records);

    /* if the insert worked, commit the transaction.  Otherwise, rollback
       the transaction */
    if (status == SUCCESS)
        status = ias_db_disconnect_with_transaction(db, IAS_DB_COMMIT);
    else
        status = ias_db_disconnect_with_transaction(db, IAS_DB_ROLLBACK);

    if (status != SUCCESS)
        IAS_LOG_ERROR("Completing the insert transaction");

    return status;
}
