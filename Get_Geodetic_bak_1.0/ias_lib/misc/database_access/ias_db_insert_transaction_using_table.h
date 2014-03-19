#ifndef IAS_DB_INSERT_TRANSACTION_USING_TABLE_H
#define IAS_DB_INSERT_TRANSACTION_USING_TABLE_H

/*************************************************************************

NAME: ias_db_insert_transaction_using_table.h

PURPOSE: Header file defining a function for doing a database insert with a 
    transaction.

Algorithm References: None

**************************************************************************/

#include "ias_db_insert.h"

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
);

#endif
