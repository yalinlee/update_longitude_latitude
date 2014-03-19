#ifndef IAS_DB_TABLE_H
#define IAS_DB_TABLE_H

/*************************************************************************

NAME: ias_db_table.h

PURPOSE: Header file defining types and functions for doing table-based 
        database operations (inserts and stored procedures).

Algorithm References: None

**************************************************************************/

#include "ias_db.h"

/* define the supported data types */
typedef enum
{
    IAS_DB_FIELD_DOUBLE = 0,
    IAS_DB_FIELD_INT = 1,
    IAS_DB_FIELD_STRING = 2,
    IAS_DB_FIELD_DATETIME = 3,            /* For date/times to seconds precision
                                           Expected format: yyyy-mm-ddThh:mm:ss
                                                            or yyyy-mm-dd     */
    IAS_DB_FIELD_DATETIME_DOY = 4,        /* For date with no time.
                                           Expected format: yyyy-ddd */
    IAS_DB_FIELD_DATETIME_NS_DOY = 5,     /* For date/time to fractional second, 
                                           in 2 fields. 
                                           Expected format:  
                                           yyyy-ddd[T| ]HH:MM:SS.sssssssss */
    IAS_DB_FIELD_DATETIME_NS_DOY_SOD = 6, /* For date/time to fractional second,
                                           in 2 fields.  
                                           Expected format: 
                                           yyyy-ddd[T| ]SSSSS.sssssssss */
    IAS_DB_FIELD_DOUBLE_ARRAY = 7, 
    IAS_DB_FIELD_INT_ARRAY = 8,
    IAS_DB_FIELD_STRING_ARRAY = 9,
    IAS_DB_FIELD_DATETIME_ARRAY = 10,
    IAS_DB_FIELD_DATETIME_NS_DOY_SOD_ARRAY = 11,
    IAS_DB_FIELD_INT16 = 12,
    IAS_DB_FIELD_INT16_ARRAY = 13,
    IAS_DB_FIELD_FLOAT = 14,
    IAS_DB_FIELD_FLOAT_ARRAY = 15,
    IAS_DB_FIELD_TIMESTAMP = 16           /* For date/time to nanoseconds 
                                           precision, using a single timestamp 
                                           database column.
                                           Expected format:  
                                           yyyy:ddd:hh:mm:ss.sss */
} IAS_DB_FIELD_DATA_TYPE;

/* a structure that defines information about each field in the table */
typedef struct
{
    const char *field_name;
    IAS_DB_PARAMETER_MODE_TYPE parameter_mode;
    int data_type;
    void *data_ptr;
    int length;
    IAS_DB_NULL_TYPE *null_ptr; /* optional pointer to null indicator(s)  --
                      you may provide your own array of indicators, or set this
                      equal to data_ptr to indicate that every row's value is 
                      null */
} IAS_DB_TABLE_FIELD;


/* define macros to make setting up table entries easy */
#define IAS_DB_TABLE_DOUBLE(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DOUBLE, \
        &(variable), 0, NULL}
#define IAS_DB_TABLE_FLOAT(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_FLOAT, \
        &(variable), 0, NULL}
#define IAS_DB_TABLE_INT(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT, \
        &(variable), 0, NULL}
#define IAS_DB_TABLE_INT16(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT16, \
        &(variable), 0, NULL}
#define IAS_DB_TABLE_STRING(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, \
        variable, 0, NULL}
#define IAS_DB_TABLE_DATETIME(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME, \
        variable, 0, NULL}
#define IAS_DB_TABLE_DATETIME_DOY(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME_DOY,\
        variable, 0, NULL}
#define IAS_DB_TABLE_DATETIME_NS_DOY(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME_NS_DOY, \
        variable, 0, NULL}
#define IAS_DB_TABLE_DATETIME_NS_DOY_SOD(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME_NS_DOY_SOD,\
        variable, 0, NULL}
#define IAS_DB_TABLE_DOUBLE_ARRAY(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DOUBLE_ARRAY, \
        variable, 0, NULL}
#define IAS_DB_TABLE_FLOAT_ARRAY(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_FLOAT_ARRAY, \
        variable, 0, NULL}
#define IAS_DB_TABLE_INT_ARRAY(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT_ARRAY, \
        variable, 0, NULL}
#define IAS_DB_TABLE_INT16_ARRAY(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT16_ARRAY, \
        variable, 0, NULL}
#define IAS_DB_TABLE_STRING_ARRAY(field_name, variable, data_length) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING_ARRAY, \
        variable, data_length, NULL}
#define IAS_DB_TABLE_DATETIME_ARRAY(field_name, variable, data_length) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME_ARRAY, \
        variable, data_length, NULL}
#define IAS_DB_TABLE_DATETIME_NS_DOY_SOD_ARRAY(field_name, variable, \
        data_length) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, \
        IAS_DB_FIELD_DATETIME_NS_DOY_SOD_ARRAY, variable, data_length, \
        NULL}
#define IAS_DB_TABLE_TIMESTAMP(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_TIMESTAMP, \
        variable, 0, NULL}

/* Use these macros if you need to insert some NULL values 
 * The null_flag must be an array of length num_records, indicating whether the 
 * field in each record is NULL */
#define IAS_DB_TABLE_DOUBLE_WITH_NULLS(field_name, variable, null_flag) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DOUBLE, \
        &(variable), 0, null_flag}
#define IAS_DB_TABLE_FLOAT_WITH_NULLS(field_name, variable, null_flag) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_FLOAT, \
        &(variable), 0, null_flag}
#define IAS_DB_TABLE_INT_WITH_NULLS(field_name, variable, null_flag) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT, \
        &(variable), 0, null_flag}
#define IAS_DB_TABLE_INT16_WITH_NULLS(field_name, variable, null_flag) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT16, \
        &(variable), 0, null_flag}
#define IAS_DB_TABLE_STRING_WITH_NULLS(field_name, variable, null_flag) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, \
        variable, 0, null_flag}
#define IAS_DB_TABLE_DATETIME_WITH_NULLS(field_name, variable, null_flag) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME, \
        variable, 0, null_flag}
#define IAS_DB_TABLE_DATETIME_DOY_WITH_NULLS(field_name, variable, null_flag) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME_DOY, \
        variable, 0, null_flag}
#define IAS_DB_TABLE_DATETIME_NS_DOY_WITH_NULLS(field_name, variable, \
        null_flag) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME_NS_DOY, \
        variable, 0, null_flag}
#define IAS_DB_TABLE_DATETIME_NS_DOY_SOD_WITH_NULLS(field_name, variable, \
        null_flag) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME_NS_DOY_SOD,\
        variable, 0, null_flag}
#define IAS_DB_TABLE_DOUBLE_ARRAY_WITH_NULLS(field_name, variable, null_flag) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DOUBLE_ARRAY, \
        variable, 0, null_flag}
#define IAS_DB_TABLE_FLOAT_ARRAY_WITH_NULLS(field_name, variable, null_flag) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_FLOAT_ARRAY, \
        variable, 0, null_flag}
#define IAS_DB_TABLE_INT_ARRAY_WITH_NULLS(field_name, variable, null_flag) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT_ARRAY, \
        variable, 0, null_flag}
#define IAS_DB_TABLE_INT16_ARRAY_WITH_NULLS(field_name, variable, null_flag) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT16_ARRAY, \
        variable, 0, null_flag}
#define IAS_DB_TABLE_STRING_ARRAY_WITH_NULLS(field_name, variable, data_length,\
        null_flag) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING_ARRAY, \
        variable, data_length, null_flag}
#define IAS_DB_TABLE_DATETIME_ARRAY_WITH_NULLS(field_name, variable, \
        data_length, null_flag) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME_ARRAY, \
        variable, data_length, null_flag}
#define IAS_DB_TABLE_DATETIME_NS_DOY_SOD_ARRAY_WITH_NULLS(field_name, variable,\
        data_length, null_flag) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, \
        IAS_DB_FIELD_DATETIME_NS_DOY_SOD_ARRAY, variable, data_length, \
         null_flag}
#define IAS_DB_TABLE_TIMESTAMP_WITH_NULLS(field_name, variable, null_flag) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_TIMESTAMP, \
        &(variable), 0, null_flag}

/* Use these macros if you need to insert a single value or list of values 
 * where every value is NULL */
#define IAS_DB_TABLE_DOUBLE_NULL(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DOUBLE, \
        &(variable), 0, (void *)&(variable)}
#define IAS_DB_TABLE_FLOAT_NULL(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_FLOAT, \
        &(variable), 0, (void *)&(variable)}
#define IAS_DB_TABLE_INT_NULL(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT, \
        &(variable), 0, (void *)&(variable)}
#define IAS_DB_TABLE_INT16_NULL(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT16, \
        &(variable), 0, (void *)&(variable)}
#define IAS_DB_TABLE_STRING_NULL(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, \
        variable, 0, (void *)variable}
#define IAS_DB_TABLE_DATETIME_NULL(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME, \
        variable, 0, (void *)variable}
#define IAS_DB_TABLE_DATETIME_DOY_NULL(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME_DOY, \
        variable, 0, (void *)variable}
#define IAS_DB_TABLE_DATETIME_NS_DOY_NULL(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME_NS_DOY, \
        variable, 0, (void *)variable}
#define IAS_DB_TABLE_DATETIME_NS_DOY_SOD_NULL(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME_NS_DOY_SOD,\
        variable, 0, (void *)variable}
#define IAS_DB_TABLE_DOUBLE_ARRAY_NULL(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DOUBLE_ARRAY, \
        variable, 0, (void *)variable}
#define IAS_DB_TABLE_FLOAT_ARRAY_NULL(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_FLOAT_ARRAY, \
        variable, 0, (void *)variable}
#define IAS_DB_TABLE_INT_ARRAY_NULL(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT_ARRAY, \
        variable, 0, (void *)variable}
#define IAS_DB_TABLE_INT16_ARRAY_NULL(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT16_ARRAY, \
        variable, 0, (void *)variable}
#define IAS_DB_TABLE_STRING_ARRAY_NULL(field_name, variable, data_length) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING_ARRAY, \
        variable, data_length, (void *)variable}
#define IAS_DB_TABLE_DATETIME_ARRAY_NULL(field_name, variable, data_length) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME_ARRAY, \
        variable, data_length, (void *)variable}
#define IAS_DB_TABLE_DATETIME_NS_DOY_SOD_ARRAY_NULL(field_name, variable, \
        data_length) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, \
        IAS_DB_FIELD_DATETIME_NS_DOY_SOD_ARRAY, variable, data_length, \
        (void *)variable}
#define IAS_DB_TABLE_TIMESTAMP_NULL(field_name, variable) \
    {field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_TIMESTAMP, \
        &(variable), 0, (void *)&(variable)}


/* define macros to make it easy to set up table entries where the database 
   field name and structure field name are the same.  This uses the C
   preprocessor #define "stringify" # operator. */
#define IAS_DB_TABLE_STRUCT_DOUBLE(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DOUBLE, \
        &(structure)->field_name, 0, NULL}
#define IAS_DB_TABLE_STRUCT_FLOAT(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_FLOAT, \
        &(structure)->field_name, 0, NULL}
#define IAS_DB_TABLE_STRUCT_INT(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT, \
        &(structure)->field_name, 0, NULL}
#define IAS_DB_TABLE_STRUCT_INT16(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT16, \
        &(structure)->field_name, 0, NULL}
#define IAS_DB_TABLE_STRUCT_STRING(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, \
        (structure)->field_name, 0, NULL}
#define IAS_DB_TABLE_STRUCT_DATETIME(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME, \
        (structure)->field_name, 0, NULL}
#define IAS_DB_TABLE_STRUCT_DATETIME_DOY(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME_DOY, \
        (structure)->field_name, 0, NULL}
#define IAS_DB_TABLE_STRUCT_DATETIME_NS_DOY(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME_NS_DOY, \
        (structure)->field_name, 0, NULL}
#define IAS_DB_TABLE_STRUCT_DATETIME_NS_DOY_SOD(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, \
        IAS_DB_FIELD_DATETIME_NS_DOY_SOD, (structure)->field_name, 0, \
        NULL}
#define IAS_DB_TABLE_STRUCT_DOUBLE_ARRAY(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DOUBLE_ARRAY, \
        (structure)->field_name, 0, NULL}
#define IAS_DB_TABLE_STRUCT_FLOAT_ARRAY(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_FLOAT_ARRAY, \
        (structure)->field_name, 0, NULL}
#define IAS_DB_TABLE_STRUCT_INT_ARRAY(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT_ARRAY, \
        (structure)->field_name, 0, NULL}
#define IAS_DB_TABLE_STRUCT_STRING_ARRAY(structure, field_name, data_length) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING_ARRAY, \
        (structure)->field_name, data_length, NULL}
#define IAS_DB_TABLE_STRUCT_DATETIME_NS_DOY_SOD_ARRAY(structure,field_name, \
        data_length) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, \
        IAS_DB_FIELD_DATETIME_NS_DOY_SOD_ARRAY, \
        (structure)->field_name, data_length, NULL}
#define IAS_DB_TABLE_STRUCT_TIMESTAMP(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_TIMESTAMP, \
        &(structure)->field_name, 0, NULL}

/* Use these macros if you need to insert some NULL values 
 * The null_flag must be an array of length num_records, indicating whether the 
 * field in each record is NULL */
#define IAS_DB_TABLE_STRUCT_DOUBLE_WITH_NULLS(structure, field_name, \
        null_flag) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DOUBLE, \
        &(structure)->field_name, 0, null_flag}
#define IAS_DB_TABLE_STRUCT_FLOAT_WITH_NULLS(structure, field_name, \
        null_flag) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_FLOAT, \
        &(structure)->field_name, 0, null_flag}
#define IAS_DB_TABLE_STRUCT_INT_WITH_NULLS(structure, field_name, \
        null_flag) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT, \
        &(structure)->field_name, 0, null_flag}
#define IAS_DB_TABLE_STRUCT_INT16_WITH_NULLS(structure, field_name, \
        null_flag) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT16, \
        &(structure)->field_name, 0, null_flag}
#define IAS_DB_TABLE_STRUCT_STRING_WITH_NULLS(structure, field_name, \
        null_flag) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, \
        (structure)->field_name, 0, null_flag}
#define IAS_DB_TABLE_STRUCT_DATETIME_WITH_NULLS(structure, field_name, \
        null_flag) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME, \
        (structure)->field_name, 0, null_flag}
#define IAS_DB_TABLE_STRUCT_DATETIME_DOY_WITH_NULLS(structure, field_name, \
        null_flag) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME_DOY, \
        (structure)->field_name, 0, \
    null_flag}
#define IAS_DB_TABLE_STRUCT_DATETIME_NS_DOY_WITH_NULLS(structure, field_name, \
        null_flag) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME_NS_DOY, \
        (structure)->field_name, 0, null_flag}
#define IAS_DB_TABLE_STRUCT_DATETIME_NS_DOY_SOD_WITH_NULLS(structure, \
        field_name, null_flag) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, \
        IAS_DB_FIELD_DATETIME_NS_DOY_SOD, (structure)->field_name, 0,\
        null_flag}
#define IAS_DB_TABLE_STRUCT_DOUBLE_ARRAY_WITH_NULLS(structure, field_name, \
        null_flag) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DOUBLE_ARRAY, \
        (structure)->field_name, 0, null_flag}
#define IAS_DB_TABLE_STRUCT_FLOAT_ARRAY_WITH_NULLS(structure, field_name, \
        null_flag) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_FLOAT_ARRAY, \
        (structure)->field_name, 0, null_flag}
#define IAS_DB_TABLE_STRUCT_INT_ARRAY_WITH_NULLS(structure, field_name, \
        null_flag) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT_ARRAY, \
        (structure)->field_name, 0, null_flag}
#define IAS_DB_TABLE_STRUCT_STRING_ARRAY_WITH_NULLS(structure, field_name, \
        data_length, null_flag) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING_ARRAY, \
        (structure)->field_name, data_length, null_flag}
#define IAS_DB_TABLE_STRUCT_DATETIME_NS_DOY_SOD_ARRAY_WITH_NULLS(structure, \
        field_name, data_length, null_flag) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, \
        IAS_DB_FIELD_DATETIME_NS_DOY_SOD_ARRAY, \
        (structure)->field_name, data_length, null_flag}
#define IAS_DB_TABLE_STRUCT_TIMESTAMP_WITH_NULLS(structure, field_name, \
        null_flag) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_TIMESTAMP, \
        &(structure)->field_name, 0, null_flag}

/* Use these macros if you need to insert a single value or list of values 
 * where every value is NULL */
#define IAS_DB_TABLE_STRUCT_DOUBLE_NULL(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DOUBLE, \
        &(structure)->field_name, 0, (void *)&(structure)->field_name}
#define IAS_DB_TABLE_STRUCT_FLOAT_NULL(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_FLOAT, \
        &(structure)->field_name, 0, (void *)&(structure)->field_name}
#define IAS_DB_TABLE_STRUCT_INT_NULL(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT, \
        &(structure)->field_name, 0, (void *)&(structure)->field_name}
#define IAS_DB_TABLE_STRUCT_INT16_NULL(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT16, \
        &(structure)->field_name, 0, (void *)&(structure)->field_name}
#define IAS_DB_TABLE_STRUCT_STRING_NULL(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING, \
        (structure)->field_name, 0, (void *)(structure)->field_name}
#define IAS_DB_TABLE_STRUCT_DATETIME_NULL(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME, \
        (structure)->field_name, 0, (void *)(structure)->field_name}
#define IAS_DB_TABLE_STRUCT_DATETIME_DOY_NULL(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME_DOY, \
        (structure)->field_name, 0, (void *)(structure)->field_name}
#define IAS_DB_TABLE_STRUCT_DATETIME_NS_DOY_NULL(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DATETIME_NS_DOY, \
        (structure)->field_name, 0, (void *)(structure)->field_name}
#define IAS_DB_TABLE_STRUCT_DATETIME_NS_DOY_SOD_NULL(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, \
        IAS_DB_FIELD_DATETIME_NS_DOY_SOD, (structure)->field_name, 0, \
        (void *)(structure)->field_name}
#define IAS_DB_TABLE_STRUCT_DOUBLE_ARRAY_NULL(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_DOUBLE_ARRAY, \
        (structure)->field_name, 0, (void *)(structure)->field_name}
#define IAS_DB_TABLE_STRUCT_FLOAT_ARRAY_NULL(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_FLOAT_ARRAY, \
        (structure)->field_name, 0, (void *)(structure)->field_name}
#define IAS_DB_TABLE_STRUCT_INT_ARRAY_NULL(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_INT_ARRAY, \
        (structure)->field_name, 0, (void *)(structure)->field_name}
#define IAS_DB_TABLE_STRUCT_STRING_ARRAY_NULL(structure, field_name, \
        data_length) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_STRING_ARRAY, \
        (structure)->field_name, data_length, (void *)(structure)->field_name}
#define IAS_DB_TABLE_STRUCT_DATETIME_NS_DOY_SOD_ARRAY_NULL(structure,\
        field_name, data_length) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, \
        IAS_DB_FIELD_DATETIME_NS_DOY_SOD_ARRAY, \
        (structure)->field_name, data_length, (void *)(structure)->field_name}
#define IAS_DB_TABLE_STRUCT_TIMESTAMP_NULL(structure, field_name) \
    {#field_name, IAS_DB_PARAMETER_MODE_INPUT, IAS_DB_FIELD_TIMESTAMP, \
        &(structure)->field_name, 0, (void *)&(structure)->field_name}

IAS_DB_NULL_TYPE ias_db_get_null_data_flag();

#endif
