#ifndef IAS_DB_STORED_PROC_H
#define IAS_DB_STORED_PROC_H

/*************************************************************************

NAME: ias_db_stored_proc.h

PURPOSE: Header file defining types and functions for calling database
    stored procedures.

Algorithm References: None

**************************************************************************/

#include "ias_db_table.h"

/* provide an alias to the field table structure */
typedef IAS_DB_TABLE_FIELD IAS_DB_STORED_PROC_TABLE;

/* prototype for the routine to stored_proc data into a database using a table.
   Returns SUCCESS or ERROR */
int ias_db_stored_proc_using_table
(
    struct ias_db_connection *db,       /* I: database connection */
    const char *procedure_name,         /* I: name of procedure to call */
    const IAS_DB_STORED_PROC_TABLE* stored_proc_table, /* I: table defining the
                                              data to be stored into the 
                                              database */
    int stored_proc_table_length,       /* I: number of entries in the
                                              stored_proc_table */
    int num_records                     /* I: The number of elements in any 
                                              members of the table struct
                                              that are declared as array types.
                                              If there are no arrays, then this
                                              argument must be 1 */
);

/* provide an alias to the field enum types */
typedef enum
{
    IAS_STORED_PROC_DOUBLE              = IAS_DB_FIELD_DOUBLE,
    IAS_STORED_PROC_INT                 = IAS_DB_FIELD_INT,
    IAS_STORED_PROC_STRING              = IAS_DB_FIELD_STRING,
    IAS_STORED_PROC_DATETIME            = IAS_DB_FIELD_DATETIME,
    IAS_STORED_PROC_DATETIME_DOY        = IAS_DB_FIELD_DATETIME_DOY,
    IAS_STORED_PROC_DATETIME_NS_DOY     = IAS_DB_FIELD_DATETIME_NS_DOY,
    IAS_STORED_PROC_DATETIME_NS_DOY_SOD = IAS_DB_FIELD_DATETIME_NS_DOY_SOD,
    IAS_STORED_PROC_DOUBLE_ARRAY        = IAS_DB_FIELD_DOUBLE_ARRAY,
    IAS_STORED_PROC_INT_ARRAY           = IAS_DB_FIELD_INT_ARRAY,
    IAS_STORED_PROC_STRING_ARRAY        = IAS_DB_FIELD_STRING_ARRAY,
    IAS_STORED_PROC_DATETIME_ARRAY      = IAS_DB_FIELD_DATETIME_ARRAY,
    IAS_STORED_PROC_DATETIME_NS_DOY_SOD_ARRAY 
                                       = IAS_DB_FIELD_DATETIME_NS_DOY_SOD_ARRAY,
    IAS_STORED_PROC_INT16               = IAS_DB_FIELD_INT16,
    IAS_STORED_PROC_INT16_ARRAY         = IAS_DB_FIELD_INT16_ARRAY,
    IAS_STORED_PROC_FLOAT               = IAS_DB_FIELD_FLOAT,
    IAS_STORED_PROC_FLOAT_ARRAY         = IAS_DB_FIELD_FLOAT_ARRAY,
    IAS_STORED_PROC_TIMESTAMP           = IAS_DB_FIELD_TIMESTAMP
} IAS_STORED_PROC_DATA_TYPE;

#endif
