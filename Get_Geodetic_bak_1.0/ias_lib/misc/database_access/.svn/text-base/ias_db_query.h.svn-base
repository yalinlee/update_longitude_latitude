#ifndef IAS_DB_QUERY_H
#define IAS_DB_QUERY_H

/*************************************************************************

NAME: ias_db_query.h

PURPOSE: Header file defining types and functions for doing database
    queries.

Algorithm References: None

**************************************************************************/

#include "ias_db.h"

/* define the supported data types */
typedef enum
{
    IAS_QUERY_DOUBLE = 0,
    IAS_QUERY_INT = 1,
    IAS_QUERY_STRING = 2,
    IAS_QUERY_DATETIME = 3,
    IAS_QUERY_DATETIME_DOY = 4,
    IAS_QUERY_DATETIME_NS_DOY = 5,
    IAS_QUERY_DATETIME_NS_DOY_SOD = 6,
    IAS_QUERY_DOUBLE_ARRAY = 7,
    IAS_QUERY_INT_ARRAY = 8,
    IAS_QUERY_STRING_ARRAY = 9,
    IAS_QUERY_DATETIME_NS_DOY_SOD_ARRAY = 10,
    IAS_QUERY_INT16 = 11,
    IAS_QUERY_INT16_ARRAY = 12,
    IAS_QUERY_FLOAT = 13,
    IAS_QUERY_FLOAT_ARRAY = 14
} IAS_QUERY_DATA_TYPE;

/* define the structure to define the table of values to query */
typedef struct
{
    const char *field_name;
    void *data_ptr;
    int length;
    int data_type;
    int data_count;
} IAS_DB_QUERY_TABLE;

/* prototype for the routine to build the query statement with bind variables
   using a table structure passed in.  Returns a null pointer if it could not
   be constructed.
   Otherwise, it returns a pointer to newly allocated memory containing the
   insert statement.  It is the invoker's responsibility to free the memory
   when finished with it. */
char * ias_db_query_get_query_statement
(
    const char *database_table,            /* I: name of database table */
    const IAS_DB_QUERY_TABLE* query_table, /* I: table defining the data to be
                                                 queryed into the database */
    int query_table_length,                /* I: number of entries in the
                                                 query_table */
    const char *where_clause               /* I: where clause for the query */
);

/* prototype for the routine to query data into a database using a table.
   Returns SUCCESS on success, ERROR on failure. */
int ias_db_query_using_table
(
    struct ias_db_connection *db,          /* I: database connection */
    const char *database_table,            /* I: name of database table */
    const IAS_DB_QUERY_TABLE* query_table, /* I: table defining the data to be
                                                 queryed into the database */
    int query_table_length,                /* I: number of entries in the
                                                 query_table */
    const char *where_clause,              /* I: where clause for the query */
    int num_records                        /* I: The number of elements in any 
                                                 members of the table struct 
                                                 that are declared as array 
                                                 types.  If there are no arrays,
                                                 then this argument must be 1 */
);

int ias_db_get_count
(
    struct ias_db_connection *db,   /* I: db handle */
    const char *table_name,         /* I: table to query */
    const char *column_name,        /* I: table column name */
    const char *value               /* I: columnn value to look for */
);

int ias_db_delete_records
(
    struct ias_db_connection *db,   /* I: db handle */
    const char *table_name,         /* I: table to query */
    const char *column_name,        /* I: table column name */
    const char *value               /* I: work order common id to look for */
);

#endif
