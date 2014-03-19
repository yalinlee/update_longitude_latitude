#ifndef IAS_DB_INSERT_H
#define IAS_DB_INSERT_H

/*************************************************************************

NAME: ias_db_insert.h

PURPOSE: Header file defining types and functions for doing a database insert.

Algorithm References: None

**************************************************************************/

#include "ias_db_table.h"

/* provide an alias to the field table structure */
typedef IAS_DB_TABLE_FIELD IAS_DB_INSERT_TABLE;

/* prototype for the routine to insert data into a database using a table.
   Returns SUCCESS on success, ERROR on failure. */
int ias_db_insert_using_table
(
    struct ias_db_connection *db,            /* I: database connection */
    const char *database_table,              /* I: name of database table */
    const IAS_DB_INSERT_TABLE* insert_table, /* I: table defining the data to be
                                                   inserted into the database */
    int insert_table_length,                 /* I: number of entries in the
                                                   insert_table */
    int num_records                          /* I: The number of elements in any
                                                    members of the table struct 
                                                    that are declared as array 
                                                    types.  If there are no 
                                                    arrays, then this argument 
                                                    must be 1 */
);

#endif
