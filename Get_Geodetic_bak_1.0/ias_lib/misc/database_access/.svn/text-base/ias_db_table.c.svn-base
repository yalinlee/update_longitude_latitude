/******************************************************************************
NAME: ias_db_table

PURPOSE:        
These functions provide the ability to interact with the database using a table 
structure defined by the calling routine.  The table structures have the 
information that allow the data to be inserted, or to be used as parameters 
to a stored procedure.  They allow applications to be isolated from the actual 
mechanics of doing database operations in case the method needs to change.  It
also allows optimizing the operations by using multiple rows with one call
to the underlying database.

NOTES on interacting with stored procedures:
1. If there is any chance a procedure's output parameter may be NULL, be sure
to provide a valid null_ptr in your IAS_DB_STORED_PROC_TABLE.  
Null indicators may also be provided for input parameters.  If a parameter
is null for every call being made, the user can pass the same pointer in for
the data_ptr and the null_ptr items of the table structure.

2. Calling a stored procedure with a default parameter value is not handled 
programmatically by this library.  The only way to call a stored proc and use a
parameter's default value is to omit it from the call statement (i.e., leave it
out of the IAS_DB_STORED_PROC_TABLE).  To do this, the parameter with the 
deafult must be at the end of procedure's list of arguments.  For example, if 
you have a procedure defined like this: 
    CREATE OR REPLACE PROCEDURE TEST_PROC
     (arg1 IN INTEGER DEFAULT 20,
      arg2 IN INTEGER,
      arg3 IN INTEGER DEFAULT 4)
    AS
    BEGIN
        ...
    END TEST_PROC;
You can call it, specifying a value for all args : {CALL test_proc(1,2,9)}
You can call it, using the default value for arg3: {Call test_proc(1,2)}
BUT you CANNOT call it, using the default value for arg1:{Call test_proc( ,2,9)}
 (since arg1 is not at the end of the argument list, you can't just leave it 
  out of the call statement)

3. Binding to Oracle DATE and TIMESTAMP parameters for output does not work
   (but input does).  If you have a stored procedure that has an OUT or IN OUT
   parameter of type DATE or TIMESTAMP, define it in your proc table as
   a string (IAS_DB_FIELD_STRING) and the driver will do the conversion
   to and from the correct type.  When doing this, be sure to format the
   string with the default date or timestamp format:
   NLS_DATE_FORMAT      DD-MON-RR                '20-NOV-07'
   NLS_TIMESTAMP_FORMAT DD-MON-RR HH.MI.SSXFF AM '20-NOV-07 08:00:00.55342 AM'

ALGORITHM REFERENCES:
none

******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_db_table.h"
#include "local.h"

/* ======================= Static Routines ===================================*/

/****************************************************************************
* Name: ias_db_bind_table
*
* Description: binds the values from the table into the placeholders of the 
*   statement.
*   For INPUT ONLY arguments, if there are multiple records and an array
*   was not supplied, we can replicate the inputs using intermediate buffers 
*   and bind to that.
*   Otherwise, for OUTPUT arguments, we bind directly to the pointers 
*   provided by the caller.  If num_records > 1 then the caller must 
*   supply one of the array types.
*
* Returns: SUCCESS or ERROR
****************************************************************************/
static int ias_db_bind_table
(
    struct ias_db_query *query,   /* I: prepared query statement*/
    const IAS_DB_TABLE_FIELD* table, /* I: table defining the data to be bound
                                           to statement placeholders */
    int table_length,             /* I: number of entries in the proc_table */
    int start_record,             /* I: The starting record number */
    int num_records,              /* I: Total number of records in the table.
                                        This is also the number of elements in
                                        any members of the table struct that
                                        are declared as array types.  If there
                                        are no arrays, then this argument must
                                        be 1 */
    void **buffers,               /* I/O: pointers to intermediate buffers */
    int *num_buffers,             /* I/O: number of buffers allocated */
    IAS_DB_NULL_TYPE *all_nulls   /* I/O: one null indicator per row */
)
{
    int i;
    int j;
    IAS_DB_NULL_TYPE *null_indicators = NULL;
                                /* pointer to the null indicators for a set of 
                                   records for one column */
    IAS_DB_NULL_TYPE *string_null_ind; 
                                /* special list of null indicators for strings*/
    IAS_DB_NULL_TYPE *copy_of_null_ind;
                                /* a copy of string null indicators, since we 
                                   reset some of the values */
    int status = SUCCESS;

    /* Bind the table data pointers to the procedure arguments.
     * For INPUT ONLY arguments, if there are multiple records and an array
     * was not supplied, we can replicate the inputs and bind to that.
     * Otherwise, for OUTPUT arguments, we bind directly to the pointers 
     * provided by the caller.  If num_records > 1 then the caller must 
     * supply one of the array types.
     */
    for(i = 0, j = 0; i < table_length; i++, j++)
    {
        const IAS_DB_TABLE_FIELD *field = &table[i];

        /* Set up the null indicator for this column's set of rows */
        if (field->null_ptr == NULL)
            /*   none of the values in this column are null */
            null_indicators = NULL;
        else if (field->null_ptr == field->data_ptr) 
            /*   setting the null_ptr to the data_ptr is a way to indicate 
             *   that this field is null in every row */
            null_indicators = all_nulls;
        else
            /*   the user provided a list of null indicators */
            null_indicators = &field->null_ptr[start_record];

        switch (field->data_type)
        {
            case IAS_DB_FIELD_FLOAT:
                {
                    if (num_records == 1)
                    {
                        /* Bind directly to data_ptr */
                        status = ias_db_bind_float_by_index(query, j,
                            field->parameter_mode,
                            (float *)(field->data_ptr),
                            null_indicators);
                    }
                    else
                    {
                        if (field->parameter_mode == 
                            IAS_DB_PARAMETER_MODE_INPUT)
                        {
                            float *buf;
                            if (start_record == 0)
                            {
                                buf = malloc(sizeof(float) * num_records);
                                if (buf)
                                {
                                    buffers[(*num_buffers)++] = buf;
                                    int k;
                                    for (k = 0; k < num_records; k++)
                                        buf[k] = 
                                            *(float *)(field->data_ptr);
                                }
                                else
                                    status = ERROR;
                            }
                            else
                            {
                                buf = buffers[(*num_buffers)++];
                            }
                            if (status != ERROR)
                            {
                                status = ias_db_bind_float_by_index(query, j, 
                                    field->parameter_mode,
                                    &buf[start_record], null_indicators);
                            }
                        }
                        else
                        {
                            /* Output values need an array to be placed in */
                            IAS_LOG_ERROR("Array not provided for %d output"
                                " parameters for argument %d", 
                                num_records, i);
                            status = ERROR;
                        }
                    }
                    break;
                }
            case IAS_DB_FIELD_DOUBLE:
                {
                    if (num_records == 1)
                    {
                        /* Bind directly to data_ptr */
                        status = ias_db_bind_double_by_index(query, j,
                            field->parameter_mode,
                            (double *)(field->data_ptr),
                            null_indicators);
                    }
                    else
                    {
                        if (field->parameter_mode == 
                            IAS_DB_PARAMETER_MODE_INPUT)
                        {
                            double *buf;
                            if (start_record == 0)
                            {
                                buf = malloc(sizeof(double) * num_records);
                                if (buf)
                                {
                                    buffers[(*num_buffers)++] = buf;
                                    int k;
                                    for (k = 0; k < num_records; k++)
                                        buf[k] = 
                                            *(double *)(field->data_ptr);
                                }
                                else
                                    status = ERROR;
                            }
                            else
                            {
                                buf = buffers[(*num_buffers)++];
                            }
                            if (status != ERROR)
                            {
                                status = ias_db_bind_double_by_index(query, j,
                                    field->parameter_mode,
                                    &buf[start_record], null_indicators);
                            }
                        }
                        else
                        {
                            /* Output values need an array to be placed in */
                            IAS_LOG_ERROR("Array not provided for %d output"
                                " parameters for argument %d", 
                                num_records, i);
                            status = ERROR;
                        }
                    }
                    break;
                }
            case IAS_DB_FIELD_FLOAT_ARRAY:
                status = ias_db_bind_float_by_index (query, j,
                    field->parameter_mode,
                    &((float*)(field->data_ptr))[start_record],
                    null_indicators);
                break;
            case IAS_DB_FIELD_DOUBLE_ARRAY:
                status = ias_db_bind_double_by_index (query, j,
                    field->parameter_mode,
                    &((double*)(field->data_ptr))[start_record], 
                    null_indicators);
                break;
            case IAS_DB_FIELD_INT:
                {
                    if (num_records == 1)
                    {
                        /* Bind directly to data_ptr */
                        status = ias_db_bind_int_by_index(query, j,
                            field->parameter_mode,
                            (int *)(field->data_ptr),
                            null_indicators);
                    }
                    else
                    {
                        if (field->parameter_mode == 
                            IAS_DB_PARAMETER_MODE_INPUT)
                        {
                            /* Replicate input values */
                            int *buf;
                            if (start_record == 0)
                            {
                                buf = malloc(sizeof(int) * num_records);
                                if (buf)
                                {
                                    buffers[(*num_buffers)++] = buf;
                                    int k;
                                    for (k = 0; k < num_records; k++)
                                        buf[k] = *(int *)(field->data_ptr);
                                }
                                else
                                    status = ERROR;
                            }
                            else
                            {
                                buf = buffers[(*num_buffers)++];
                            }
                            if (status != ERROR)
                            {
                                status = ias_db_bind_int_by_index(query, j,
                                    field->parameter_mode,
                                    &buf[start_record], null_indicators);
                            }
                        }
                        else
                        {
                            /* Output values need an array to be placed in */
                            IAS_LOG_ERROR("Array not provided for %d output"
                                " parameters for argument %d", 
                                num_records, i);
                            status = ERROR;
                        }
                    }

                    break;
                }

            case IAS_DB_FIELD_INT_ARRAY:
                status = ias_db_bind_int_by_index (query, j,
                    field->parameter_mode,
                    &((int*)(field->data_ptr))[start_record], 
                    null_indicators); 
                break;
            case IAS_DB_FIELD_INT16:
                {
                    if (num_records == 1)
                    {
                        /* Bind directly to data_ptr */
                        status = ias_db_bind_short_int_by_index(query, j,
                            field->parameter_mode,
                            (short int *)(field->data_ptr),
                            null_indicators);
                    }
                    else
                    {
                        if (field->parameter_mode == 
                            IAS_DB_PARAMETER_MODE_INPUT)
                        {
                            short int *buf;
                            if (start_record == 0)
                            {
                                buf = malloc(sizeof(short int) * num_records);
                                if (buf)
                                {
                                    buffers[(*num_buffers)++] = buf;
                                    int k;
                                    for (k = 0; k < num_records; k++)
                                        buf[k] = 
                                            *(short int *)(field->data_ptr);
                                }
                                else
                                    status = ERROR;
                            }
                            else
                            {
                                buf = buffers[(*num_buffers)++];
                            }
                            if (status != ERROR)
                            {
                                status = ias_db_bind_short_int_by_index(query, 
                                    j, field->parameter_mode,
                                    &buf[start_record], null_indicators);
                            }
                        }
                        else
                        {
                            /* Output values need an array to be placed in */
                            IAS_LOG_ERROR("Array not provided for %d output"
                                " parameters for argument %d", 
                                num_records, i);
                            status = ERROR;
                        }
                    }
                    break;
                }
            case IAS_DB_FIELD_INT16_ARRAY:
                status = ias_db_bind_short_int_by_index (query, j, 
                    field->parameter_mode,
                    &((short int*)(field->data_ptr))[start_record], 
                    null_indicators); 
                break;
            case IAS_DB_FIELD_STRING:
                {
                    string_null_ind = NULL;
                    if (null_indicators)
                    {
                        /* Make a local copy here of the null indicators for all
                         * the records, so non-null values can have the 
                         * indicator changed to SQL_NTS */
                        if (start_record == 0)
                        {
                            copy_of_null_ind = malloc(num_records 
                                               * sizeof(*copy_of_null_ind));
                            if (copy_of_null_ind) 
                            {
                                int k;
                                for (k = 0; k < num_records; k++)
                                    if (null_indicators[k] == SQL_NULL_DATA)
                                        copy_of_null_ind[k] = SQL_NULL_DATA;
                                    else
                                        copy_of_null_ind[k] = SQL_NTS;
                                buffers[(*num_buffers)++] = copy_of_null_ind;
                            }
                            else
                                status = ERROR;
                        }
                        else
                        {
                            copy_of_null_ind = buffers[(*num_buffers)++];
                        }
                        string_null_ind = &copy_of_null_ind[start_record];
                    }
                    if (num_records == 1)
                    {
                        /* Bind directly to data_ptr */
                        status = ias_db_bind_char_by_index(query, j,
                            field->parameter_mode,
                            (char *)(field->data_ptr), 
                            field->length,
                            string_null_ind);
                    }
                    else
                    {
                        if (field->parameter_mode 
                            == IAS_DB_PARAMETER_MODE_INPUT)
                        {
                            int len = strlen((char *)(field->data_ptr)) + 1;
                            char *buf;
                            if (start_record == 0)
                            {
                                buf = malloc(sizeof(char) * num_records * len);
                                if (buf)
                                {
                                    int k;
                                    buffers[(*num_buffers)++] = buf;
                                    for (k = 0; k < num_records; k++)
                                    {
                                        strcpy(&buf[k * len],
                                            (char *)(field->data_ptr));
                                    }
                                }
                                else
                                    status = ERROR;
                            }
                            else
                            {
                                buf = buffers[(*num_buffers)++];
                            }
                            if (status != ERROR)
                            {
                                status = ias_db_bind_char_by_index(query, j,
                                    field->parameter_mode,
                                    &buf[start_record * len], len, 
                                    string_null_ind);
                            }
                        }
                        else
                        {
                            /* Output values need an array to be placed in */
                            IAS_LOG_ERROR("Array not provided for %d output"
                                " parameters for argument %d", 
                                num_records, i);
                            status = ERROR;
                        }
                    }
                    break;
                }
            case IAS_DB_FIELD_STRING_ARRAY:
                string_null_ind = NULL;
                /* Make a copy of the null indicators, so the non-null
                 * values can be marked with SQL_NTS */
                if (null_indicators)
                {
                    if (start_record == 0)
                    {
                        copy_of_null_ind 
                            = malloc(num_records * sizeof(*copy_of_null_ind));
                        if (copy_of_null_ind) 
                        {
                            int k;
                            for (k = 0; k < num_records; k++)
                                if (null_indicators[k] == SQL_NULL_DATA)
                                    copy_of_null_ind[k]  = SQL_NULL_DATA;
                                else
                                    copy_of_null_ind[k] = SQL_NTS;
                            buffers[(*num_buffers)++] = copy_of_null_ind;
                        }
                        else
                            status = ERROR;
                    }
                    else
                    {
                        copy_of_null_ind = buffers[(*num_buffers)++];
                    }
                    string_null_ind = &copy_of_null_ind[start_record];
                }
                if (status != ERROR)
                {
                    status = ias_db_bind_char_by_index(query, j,
                        field->parameter_mode,
                        &((char*)(field->data_ptr))[start_record 
                        * field->length], field->length, 
                        string_null_ind); 
                }
                break;
            case IAS_DB_FIELD_DATETIME:
                {
                    /* date with no time value attached */
                    SQL_TIMESTAMP_STRUCT timestamp;
                    SQL_TIMESTAMP_STRUCT *timestamps;

                    if (num_records > 1 && 
                        field->parameter_mode 
                        != IAS_DB_PARAMETER_MODE_INPUT)
                    {
                        /* Output values need an array to be placed in */
                        IAS_LOG_ERROR("Array not provided for %d output"
                            " parameters for argument %d", 
                            num_records, i);
                        status = ERROR;
                        break;
                    }
                    if (start_record == 0)
                    {
                        /* convert the input date to the SQL structure */
                        status = ias_db_parse_yyyy_mm_dd_time(
                            (char*)(field->data_ptr),
                            &timestamp);
                        if (status != ERROR)
                        {
                            timestamps = malloc(sizeof(SQL_TIMESTAMP_STRUCT)
                                * num_records);
                            if (timestamps)
                            {
                                int k;
                                buffers[(*num_buffers)++] = timestamps;
                                for (k = 0; k < num_records; k++)
                                    timestamps[k] = timestamp;
                            }
                            else
                                status = ERROR;
                        }
                    }
                    else
                    {
                        timestamps = buffers[(*num_buffers)++];
                    }
                    if (status != ERROR)
                    {
                        status = ias_db_bind_timestamp_by_index(query, j,
                            field->parameter_mode,
                            &timestamps[start_record],
                            null_indicators);
                    }
                    break;
                }
            case IAS_DB_FIELD_TIMESTAMP:
                {
                    /* date with time including up to 9 fractional digits
                     * inputs are expected to be yyyy:ddd:hh:mm:ss.ssssss
                     * e.g., 2014:184:17:00:45.2345678 */
                    SQL_TIMESTAMP_STRUCT timestamp;
                    SQL_TIMESTAMP_STRUCT *timestamps;

                    if (num_records > 1 && 
                        field->parameter_mode 
                        != IAS_DB_PARAMETER_MODE_INPUT)
                    {
                        /* Output values need an array to be placed in */
                        IAS_LOG_ERROR("Array not provided for %d output"
                            " parameters for argument %d", 
                            num_records, i);
                        status = ERROR;
                        break;
                    }
                    if (start_record == 0)
                    {
                        /* convert the input date to the SQL structure */
                        int year;
                        int month;
                        int day;
                        int hours;
                        int minutes;
                        double seconds;
                        status = ias_db_parse_yyyy_ddd_hms(
                            (char *)(field->data_ptr),
                            &year, &month, &day, &hours, &minutes,
                            &seconds);
                        if (status != ERROR)
                        {
                            timestamp.year = year;
                            timestamp.month = month;
                            timestamp.day = day;
                            timestamp.hour = hours;
                            timestamp.minute = minutes;
                            timestamp.second = (int)seconds;
                            timestamp.fraction = (seconds-timestamp.second) *
                                1000000000;

                            timestamps = malloc(sizeof(SQL_TIMESTAMP_STRUCT)
                                * num_records);
                            if (timestamps)
                            {
                                int k;
                                buffers[(*num_buffers)++] = timestamps;
                                for (k = 0; k < num_records; k++)
                                    timestamps[k] = timestamp;
                            }
                            else
                                status = ERROR;
                        }
                    }
                    else
                    {
                        timestamps = buffers[(*num_buffers)++];
                    }
                    if (status != ERROR)
                    {
                        status = ias_db_bind_timestamp_by_index(query, j,
                            field->parameter_mode,
                            &timestamps[start_record],
                            null_indicators);
                    }
                    break;
                }
            case IAS_DB_FIELD_DATETIME_DOY:
                {
                    SQL_DATE_STRUCT *dates;

                    if (num_records > 1 && 
                        field->parameter_mode 
                        != IAS_DB_PARAMETER_MODE_INPUT)
                    {
                        /* Output values need an array to be placed in */
                        IAS_LOG_ERROR("Array not provided for %d output"
                            " parameters for argument %d", 
                            num_records, i);
                        status = ERROR;
                        break;
                    }
                    if (start_record == 0)
                    {
                        /* date with no time value attached */
                        int year;
                        int month;
                        int day;

                        /* convert the input date to the SQL structure */
                        status = ias_db_parse_yyyy_ddd(
                            (char *)(field->data_ptr),
                            &year, &month, &day);
                        if (status != ERROR)
                        {
                            SQL_DATE_STRUCT date;

                            date.year = year;
                            date.month = month;
                            date.day = day;

                            dates = malloc(sizeof(SQL_DATE_STRUCT)
                                * num_records);
                            if (dates)
                            {
                                int k;
                                buffers[(*num_buffers)++] = dates;
                                for (k = 0; k < num_records; k++)
                                    dates[k] = date;
                            }
                            else
                                status = ERROR;
                        }
                    }
                    else
                    {
                        dates = buffers[(*num_buffers)++];
                    }
                    if (status != ERROR)
                    {
                        status = ias_db_bind_date_by_index(query, j, 
                            field->parameter_mode,
                            &dates[start_record], null_indicators);
                    }
                    break;
                }
            case IAS_DB_FIELD_DATETIME_NS_DOY:
                {
                    SQL_DATE_STRUCT *dates;
                    double *times;

                    if (num_records > 1 && 
                        field->parameter_mode 
                        != IAS_DB_PARAMETER_MODE_INPUT)
                    {
                        /* Output values need an array to be placed in */
                        IAS_LOG_ERROR("Array not provided for %d output"
                            " parameters for argument %d", 
                            num_records, i);
                        status = ERROR;
                        break;
                    }
                    if (start_record == 0)
                    {
                        /* date with time value attached */
                        int year;
                        int month;
                        int day;
                        int hours;
                        int minutes;
                        double seconds;

                        /* convert the input date to the SQL structure */
                        status = ias_db_parse_yyyy_ddd_hms(
                            (char *)(field->data_ptr),
                            &year, &month, &day, &hours, &minutes,
                            &seconds);
                        if (status != ERROR)
                        {
                            SQL_DATE_STRUCT date;
                            double time;

                            date.year = year;
                            date.month = month;
                            date.day = day;

                            dates = malloc(sizeof(SQL_DATE_STRUCT)
                                * num_records);
                            if (dates)
                            {
                                int k;
                                buffers[(*num_buffers)++] = dates;
                                for (k = 0; k < num_records; k++)
                                    dates[k] = date;

                                times = malloc(sizeof(double) * num_records);
                                if (times)
                                {
                                    int k;
                                    time = hours * 3600 + minutes * 60
                                        + seconds;
                                    buffers[(*num_buffers)++] = times;
                                    for (k = 0; k < num_records; k++)
                                        times[k] = time;

                                }
                                else
                                    status = ERROR;
                            }
                            else
                                status = ERROR;
                        }
                    }
                    else
                    {
                        dates = buffers[(*num_buffers)++];
                        times = buffers[(*num_buffers)++];
                    }
                    if (status != ERROR)
                    {
                        status = ias_db_bind_date_by_index(query, j, 
                            field->parameter_mode,
                            &dates[start_record], null_indicators);
                        if (status == SUCCESS)
                        {
                            j++;
                            status = ias_db_bind_double_by_index(query, j,
                                field->parameter_mode,
                                &times[start_record], null_indicators);
                        }
                    }
                    break;
                }
            case IAS_DB_FIELD_DATETIME_NS_DOY_SOD:
                {
                    SQL_DATE_STRUCT *dates;
                    double *times;

                    if (num_records > 1 && 
                        field->parameter_mode 
                        != IAS_DB_PARAMETER_MODE_INPUT)
                    {
                        /* Output values need an array to be placed in */
                        IAS_LOG_ERROR("Array not provided for %d output"
                            " parameters for argument %d", 
                            num_records, i);
                        status = ERROR;
                        break;
                    }
                    if (start_record == 0)
                    {
                        /* date with time value attached */
                        int year;
                        int month;
                        int day;
                        double seconds;

                        /* convert the input date to the SQL structure */
                        status = ias_db_parse_yyyy_ddd_time(
                            (char *)(field->data_ptr),
                            &year, &month, &day, &seconds);
                        if (status != ERROR)
                        {
                            SQL_DATE_STRUCT date;

                            date.year = year;
                            date.month = month;
                            date.day = day;

                            dates = malloc(sizeof(SQL_DATE_STRUCT) 
                                * num_records);
                            if (dates)
                            {
                                int k;
                                buffers[(*num_buffers)++] = dates;
                                for (k = 0; k < num_records; k++)
                                    dates[k] = date;

                                times = malloc(sizeof(double) * num_records);
                                if (times)
                                {
                                    int k;
                                    buffers[(*num_buffers)++] = times;
                                    for (k = 0; k < num_records; k++)
                                        times[k] = seconds;

                                }
                                else
                                    status = ERROR;
                            }
                            else
                                status = ERROR;
                        }
                    }
                    else
                    {
                        dates = buffers[(*num_buffers)++];
                        times = buffers[(*num_buffers)++];
                    }
                    if (status != ERROR)
                    {
                        status = ias_db_bind_date_by_index(query, j,
                            field->parameter_mode,
                            &dates[start_record], null_indicators);
                        if (status == SUCCESS)
                        {
                            j++;
                            status = ias_db_bind_double_by_index(query, j,
                                field->parameter_mode,
                                &times[start_record], null_indicators);
                        }
                    }
                    break;
                }
            case IAS_DB_FIELD_DATETIME_ARRAY:
                {
                    /* date with no time value attached */
                    SQL_TIMESTAMP_STRUCT timestamp;
                    SQL_TIMESTAMP_STRUCT *timestamps;

                    if (start_record == 0)
                    {
                        /* allocate space for the converted array */
                        timestamps = malloc(sizeof(SQL_TIMESTAMP_STRUCT)
                            * num_records);
                        buffers[(*num_buffers)++] = timestamps;
                        if (timestamps)
                        {
                            int k;
                            for (k = 0; k < num_records; k++)
                            {
                                /* convert the input date to the SQL 
                                   structure */
                                status = ias_db_parse_yyyy_mm_dd_time(
                                    &(((char*)(field->data_ptr))
                                        [k * field->length]),
                                    &timestamp);
                                if (status == ERROR)
                                    break;
                                timestamps[k] = timestamp;
                            }
                        }
                        else
                            status = ERROR;
                    }
                    else
                    {
                        timestamps = buffers[(*num_buffers)++];
                    }
                    if (status != ERROR)
                    {
                        status = ias_db_bind_timestamp_by_index(query, j,
                            field->parameter_mode,
                            &timestamps[start_record], 
                            null_indicators);
                    }
                    break;
                }
            case IAS_DB_FIELD_DATETIME_NS_DOY_SOD_ARRAY:
                {
                    SQL_DATE_STRUCT *dates;
                    double *times;

                    if (start_record == 0)
                    {
                        /* date with time value attached */
                        int year;
                        int month;
                        int day;
                        double seconds;

                        /* allocate space for the converted arrays of data */
                        dates = malloc(sizeof(SQL_DATE_STRUCT) * num_records);
                        buffers[(*num_buffers)++] = dates;
                        times = malloc(sizeof(double) * num_records);
                        buffers[(*num_buffers)++] = times;
                        if (dates && times)
                        {
                            int k;

                            for (k = 0; k < num_records; k++)
                            {
                                /* convert the input date to the SQL
                                   structure */
                                status = ias_db_parse_yyyy_ddd_time(
                                    &(((char*)(field->data_ptr))
                                        [k * field->length]),
                                    &year, &month, &day, &seconds);
                                if (status == ERROR)
                                    break;
                                dates[k].year = year;
                                dates[k].month = month;
                                dates[k].day = day;
                                times[k] = seconds;
                            }
                        }
                        else
                            status = ERROR;
                    }
                    else
                    {
                        dates = buffers[(*num_buffers)++];
                        times = buffers[(*num_buffers)++];
                    }
                    if (status != ERROR)
                    {
                        status = ias_db_bind_date_by_index(query, j,
                            field->parameter_mode,
                            &dates[start_record], null_indicators);
                        if (status == SUCCESS)
                        {
                            j++;
                            status = ias_db_bind_double_by_index(
                                query, j, field->parameter_mode,
                                &times[start_record], null_indicators);
                        }
                    }
                    break;
                }
        }

        /* If we got an error binding this argument, bail out */
        if (status == ERROR )
        {
            IAS_LOG_ERROR("Binding value for argument %d", i);
            return ERROR;
        }
    }

    return SUCCESS;
}

/****************************************************************************
* Name: ias_db_execute_using_table
*
* Description: this routine prepares the statement,
*   binds variables from the table, and executes the statement (possibly
*   multiple times, depending on the number of records).
*
* Returns: SUCCESS or ERROR
****************************************************************************/
static int ias_db_execute_using_table
(
    struct ias_db_connection *db,               /* I: database connection */
    const char *sql,                            /* I: the SQL query to execute*/
    const char *sql_description,                /* I: description of SQL being
                                                      executed, for logging */
    const IAS_DB_TABLE_FIELD* table,            /* I: table defining the data to
                                                      be used as procedure 
                                                      arguments */
    int table_length,                           /* I: number of entries in the 
                                                      proc_table */
    int num_records                             /* I: The number of elements in
                                                      any members of the table 
                                                      struct that are declared 
                                                      as array types.  If there 
                                                      are no arrays, then this 
                                                      argument must be 1 */
)
{
    int i;
    int free_index;
    int status = SUCCESS;
    struct ias_db_query *query = NULL;
    char db_error[200];
    void **buffers = NULL;      /* pointers to intermediate buffers */
    int num_buffers = 0;        /* number of buffers allocated */
    int start_record;           /* record loop counter */
    IAS_DB_NULL_TYPE *all_nulls = NULL; 
                                /* indicator for a column where the value in
                                   every (num_records) row is NULL */

    /* Ensure we're processing at least one record */
    if (num_records <= 0)
    {
        IAS_LOG_ERROR("Calling %s 0 times", sql_description);
        return ERROR;
    }

    /* Prepare the statement for binding */
    query = ias_db_prepare_query( db, sql );
    if (!ias_db_query_was_successful(query))
    {
        ias_db_query_get_error_message(query,db_error,sizeof(db_error));
        IAS_LOG_ERROR("Creating and preparing the %s query string: %s",
                sql_description, db_error);
        ias_db_query_close(query);
        return ERROR;
    }

    /* allocate the array to hold the pointers to any buffers allocated. It 
       is multiplied by 3 since some types are converted into two arguments,
       plus there may be a buffer of null indicators. */
    /* N.B.: buffers are allocated when num_records is more than 1, but
     * an array was not provided for a given column.  The buffer is allocated
     * for all num_records values at once (even if the query is done in
     * batches). */
    buffers = malloc(3 * table_length * sizeof(void *));
    if (!buffers)
    {
        IAS_LOG_ERROR("Allocating memory for buffers");
        ias_db_query_close(query);
        return ERROR;
    }

    /* set up an array of null indicators to be used when we want to
     * specify that every value in the array is null */
    all_nulls = malloc(num_records * sizeof(*all_nulls));
    if (!all_nulls)
    {
        IAS_LOG_ERROR("Allocating memory for null indicators");
        ias_db_query_close(query);
        free(buffers);
        return ERROR;
    }
    for (i=0; i<num_records; i++)
        all_nulls[i] = SQL_NULL_DATA;

    /* loop over the data, executing a maximum of MAX_ROWS at once */
    for (start_record = 0; start_record < num_records;
         start_record += MAX_ROWS)
    {
        int num_calls = num_records - start_record;
        if (num_calls > MAX_ROWS)
            num_calls = MAX_ROWS;

        /* set the number of rows to call at once to the number of records
           passed in */
        if (ias_db_set_rows_to_insert(query, num_calls) != SUCCESS)
        {
            ias_db_query_get_error_message(query,db_error,sizeof(db_error));

            IAS_LOG_ERROR("Setting the number of %ss to execute at once: %s",
                    sql_description, db_error);
            ias_db_query_close(query);
            for (free_index = 0; free_index < num_buffers; free_index++)
                free(buffers[free_index]);
            free(buffers);
            free(all_nulls);
            return ERROR;
        }

        num_buffers = 0;

        /* Bind the table data pointers to the procedure arguments. */
        status = ias_db_bind_table(query, table, table_length,
            start_record, num_records, buffers, &num_buffers, all_nulls);
        if (status == ERROR )
        {
            IAS_LOG_ERROR("Binding table values to %s statement", 
                sql_description);
            ias_db_query_close(query);
            for (free_index = 0; free_index < num_buffers; free_index++)
                free(buffers[free_index]);
            free(buffers);
            free(all_nulls);
            return ERROR;
        }


        /* Execute the query */
        status = ias_db_exec_prepared_query(query);
        if ((status != SUCCESS) || !ias_db_query_was_successful(query))
        {
            ias_db_query_get_error_message(query,db_error,sizeof(db_error));
            IAS_LOG_ERROR("%s call: %s", sql_description, db_error);
            ias_db_query_close(query);
            for (free_index = 0; free_index < num_buffers; free_index++)
                free(buffers[free_index]);
            free(buffers);
            free(all_nulls);
            return ERROR;
        }

    }

    ias_db_query_close(query);

    /* free all the memory allocated */
    for (free_index = 0; free_index < num_buffers; free_index++)
    {
        free(buffers[free_index]);
    }
    free(buffers);
    free(all_nulls);

    return SUCCESS;
}

/****************************************************************************
* Name: ias_db_stored_proc_get_call_statement 
*
* Description: this routine builds the procedure call
*   statement with bind variables using the table structure passed in.  
*   Returns a null pointer if it could not be constructed.
*   Otherwise, it returns a pointer to newly allocated memory containing the
*   call statement.  It is the invoker's responsibility to free the memory
*   when finished with it.
*
* Returns: char pointer to the SQL statement, or NULL on error 
****************************************************************************/
static char * ias_db_stored_proc_get_call_statement
(
    const char *procedure_name,                /* I: name of procedure (include 
                                                     package name, if 
                                                     applicable)*/
    const IAS_DB_TABLE_FIELD* proc_table,/* I: table defining the data to
                                                     be used as parameters to 
                                                     the procedure */
    int proc_table_length                      /* I: number of entries in the
                                                     proc_table */
)
{
    char *sql = NULL;    /* buffer pointer for building sql call statement */
    int num_return = 0;  /* number of return values from the procedure */
    int i;
    int nargs;
    int statement_length = 200 + strlen(procedure_name);

    /* estimate the amount of memory needed for the call statement */
    /* the statement will look something like this:
       {[?=]call procedure_name[(?[,?]...)]}  */
    for (i = 0; i < proc_table_length; i++)
    {
        const IAS_DB_TABLE_FIELD *field_ptr = &proc_table[i];
        /* Since this is using binds instead of actual values, all that is 
           needed to be known is the number of parameters.  Adding 6 takes into 
           account the question mark and any commas and spaces that may be 
           added.*/
        statement_length += 6;

        /* If the current data type is a date time that requires more than
           milliseconds of precision, then it gets split into two arguments
           to the procedure.  So add the length of another column to the
           statement length.  */
        switch (field_ptr->data_type)
        {
            case IAS_DB_FIELD_DATETIME_NS_DOY:
            case IAS_DB_FIELD_DATETIME_NS_DOY_SOD:
            case IAS_DB_FIELD_DATETIME_NS_DOY_SOD_ARRAY:
                statement_length += 6;
                break;
        }
        if (field_ptr->parameter_mode == IAS_DB_PARAMETER_MODE_RETURN)
        {
            /* Require the return parameter to be the first one in the table
             * so that variables are bound in the correct order */
            if (i > 0)
            {
                IAS_LOG_ERROR("Return values must be the first table entry");
                return NULL;
            }
            num_return++;
        }
    }
    if (num_return > 1)
    {
        /* there should be 0 or 1 return value */
        IAS_LOG_ERROR("Invalid number of return values: %d", num_return);
        return NULL;
    }

    /* allocate space for the call statement */
    sql = malloc(statement_length * sizeof(char));
    if (!sql)
    {
        /* error allocating memory */
        IAS_LOG_ERROR("Allocating memory");
        return NULL;
    }

    /* build the call statement */
    sprintf(sql, "{");
    if (num_return > 0)
        strcat(sql, "?=");
    strcat(sql, "call ");
    strcat(sql, procedure_name );
    strcat(sql, "(" );

    /* Add the bind variables */
    for (i = 0, nargs = 0; i < proc_table_length; i++)
    {
        /* a return value is accounted for above */
        if (proc_table[i].parameter_mode == IAS_DB_PARAMETER_MODE_RETURN)
            continue;

        if ( nargs == 0 )
            strcat(sql, "?"); /* first argument placeholder */
        else
            strcat(sql, ", ?"); /* subsequequent args separated by commas */

        nargs++;

        /* If this is a field for date time that requires two columns,
           then add the bind variable for the time column also */
        switch (proc_table[i].data_type)
        {
            case IAS_DB_FIELD_DATETIME_NS_DOY:
            case IAS_DB_FIELD_DATETIME_NS_DOY_SOD:
            case IAS_DB_FIELD_DATETIME_NS_DOY_SOD_ARRAY:
                strcat(sql, ", ?");
                nargs++;
                break;
        }

    }

    strcat(sql, ")}");
    IAS_LOG_DEBUG("Stored Procedure call statement: %s", sql);

    return sql;
}


/****************************************************************************
* Name: ias_db_insert_get_insert_statement 
*
* Description: this routine builds the insert statement 
*   with bind variables using the table structure passed in.  
*   Returns a null pointer if it could not be constructed.
*   Otherwise, it returns a pointer to newly allocated memory containing the
*   insert statement.  It is the invoker's responsibility to free the memory
*   when finished with it.
*
* Returns: char pointer to the SQL statement, or NULL on error 
****************************************************************************/
static char * ias_db_insert_get_insert_statement
(
    const char *database_table,             /* I: name of database table */
    const IAS_DB_TABLE_FIELD* insert_table,/* I: table defining the data to be
                                                  inserted into the database */
    int insert_table_length                 /* I: number of entries in the
                                                  insert_table */
)
{
    char *sql = NULL;   /* buffer pointer for building sql insert statement */
    int i;
    int statement_length = 200 + strlen(database_table);
    char *enable_dml_error_logging;

    /* estimate the amount of memory needed for the insert statement */
    for (i = 0; i < insert_table_length; i++)
    {
        const IAS_DB_TABLE_FIELD *field_ptr = &insert_table[i];
        /* Since this is using binds instead of actual values, all that is 
           needed to be known for the length is the column name.  The strlen
           function takes into account the column name in the insert portion
           and adding 6 takes into account the question mark in the values
           portion of the insert and any commas and spaces that may be added.*/
        statement_length += strlen(field_ptr->field_name) + 6; 

        /* If the current data type is a date time that requires more than
           milliseconds of precision, then it gets split into two columns
           in the database.  So add the length of another column to the
           statement length.  For these columns, the word DATE is replaced
           with "TIME" in the column name. */
        switch (field_ptr->data_type)
        {
            case IAS_DB_FIELD_DATETIME_NS_DOY:
            case IAS_DB_FIELD_DATETIME_NS_DOY_SOD:
            case IAS_DB_FIELD_DATETIME_NS_DOY_SOD_ARRAY:
                statement_length += strlen(field_ptr->field_name) + 6;
                break;
        }
    }

    /* See if the DML Error Logging is enabled */
    enable_dml_error_logging = getenv("ENABLE_DML_ERROR_LOGGING");
    if (enable_dml_error_logging)
    {
        /* Add the LOG ERRORS INTO <errlog_tablename> to the statement length 
           note: strlen( LOG ERRORS INTO ERR$_) = 22 */
        statement_length += strlen(database_table) + 22; 
    }

    /* allocate space for the insert statement */
    sql = malloc(statement_length * sizeof(char));
    if (!sql)
    {
        /* error allocating memory */
        IAS_LOG_ERROR("Allocating memory");
        return NULL;
    }

    /* build the insert statement for the field names */
    sprintf(sql, "insert into %s ( ", database_table );

    for (i = 0; i < insert_table_length; i++)
    {
        if ( i != 0 )
            strcat(sql, ", ");

        strcat(sql, insert_table[i].field_name);

        /* If the current data type is a date_time that requires
           the date to be in one column and the time in another,
           then add the time column to the insert statement.
           The method used will replace the word DATE in the column
           name that contains the date with the word TIME.  For example
           if the DATE column passed in is called DCE_START_DATE, the
           column containing the seconds from the beginning of the day
           will be called DCE_START_TIME. */
        char *time_ptr = NULL;
        char *tmp_ptr = NULL;

        switch ( insert_table[i].data_type )
        {
            case IAS_DB_FIELD_DATETIME_NS_DOY:
            case IAS_DB_FIELD_DATETIME_NS_DOY_SOD:
            case IAS_DB_FIELD_DATETIME_NS_DOY_SOD_ARRAY:
                /* replace the word DATE with the word TIME */
                time_ptr = strdup(insert_table[i].field_name);

                if (time_ptr == NULL)
                {
                    IAS_LOG_ERROR ("Allocating memory for a copy of the "
                            "date field name: %s",insert_table[i].field_name);
                    free(sql);
                    return NULL;
                }

                tmp_ptr = strstr(time_ptr, "_date");

                if (tmp_ptr == NULL)
                {
                    IAS_LOG_ERROR ("Creating the insert statement."
                            "  'date' not found in date field name: %s",
                            insert_table[i].field_name);
                    free(time_ptr);
                    free(sql);
                    return NULL;
                }

                /* Now that "_date" was found, replace it with "_time" */
                strncpy(tmp_ptr, "_time", 5);

                /* Add the additional column name to the insert statement */
                strcat(sql, ", ");
                strcat(sql, time_ptr);

                free(time_ptr);
                break;
        }
    }
    strcat(sql, " ) values ( ");

    /* Add the bind variables */
    for (i = 0; i < insert_table_length; i++)
    {
        if ( i != 0 )
            strcat(sql, ", ");

        strcat(sql, "?");

        /* If this is a field for date time that requires two columns,
           then add the bind variable for the time column */
        switch (insert_table[i].data_type)
        {
            case IAS_DB_FIELD_DATETIME_NS_DOY:
            case IAS_DB_FIELD_DATETIME_NS_DOY_SOD:
            case IAS_DB_FIELD_DATETIME_NS_DOY_SOD_ARRAY:
                strcat(sql, ", ?");
                break;
        }
    }

    strcat(sql, ")");

    /* See if the DML Error Logging is enabled */
    if (enable_dml_error_logging)
    {
        /* Build the errlog_tablename */
        char errlog_tablename[50];
        int return_value;

        /* The maximum length for any Oracle table name is 30 characters. So, 
           just take the first 25 characters of database_table (since 5
           characters are used up by the ERR$_ prefix) */
        return_value = snprintf(errlog_tablename, sizeof(errlog_tablename), 
                " LOG ERRORS INTO ERR$_%-.25s", database_table);
        if (return_value < 0 || return_value >= sizeof(errlog_tablename))
        {
            IAS_LOG_ERROR("Building errlog_tablename");
            free(sql);
            return NULL;
        }

        /* Invoking DML Error Logging */
        strcat(sql, errlog_tablename);
    }

    return sql;
}

/* ======================= Public Routines ===================================*/

/****************************************************************************
* Name: ias_db_insert_using_table
*
* Description: this routine builds the SQL string for an insert, 
*   then calls the routine to execute the SQL for the input table of data.
*
* Returns: SUCCESS or ERROR
****************************************************************************/
int ias_db_insert_using_table
(
    struct ias_db_connection *db,       /* I: database connection */
    const char *database_table,         /* I: name of database table */
    const IAS_DB_TABLE_FIELD* insert_table,/* I: table defining the data to be
                                                  inserted into the database */
    int insert_table_length,            /* I: number of entries in the
                                              insert_table */
    int num_records                     /* I: The number of elements in any 
                                              members of the table struct
                                              that are declared as array types.
                                              If there are no arrays, then this
                                              argument must be 1 */
)
{
    char *sql = NULL;   /* buffer pointer for building sql insert statement */
    int status = SUCCESS;

    sql = ias_db_insert_get_insert_statement(database_table, insert_table, 
            insert_table_length);
    if (sql == NULL)
    {
        IAS_LOG_ERROR("Unable to create insert statement");
        return ERROR;
    }

    status = ias_db_execute_using_table(db, sql, "INSERT", insert_table,
        insert_table_length, num_records);
    free(sql);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Unable to execute insert statement");
        return ERROR;
    }

    return SUCCESS;
}

/****************************************************************************
* Name: ias_db_get_null_data_flag
*
* Description: returns the API's flag value indicating a null value
*
* Returns: integer null indicator value
****************************************************************************/
IAS_DB_NULL_TYPE ias_db_get_null_data_flag()
{
    return SQL_NULL_DATA;
}

/****************************************************************************
* Name: ias_db_stored_proc_using_table
*
* Description: this routine builds the SQL string for a stored procedure call,
*   then calls the routine to execute the SQL using the input table of data.
*
* Returns: SUCCESS or ERROR
****************************************************************************/
int ias_db_stored_proc_using_table
(
    struct ias_db_connection *db,               /* I: database connection */
    const char *procedure_name,                 /* I: name of procedure 
                                                      (including package, if 
                                                      applicable */
    const IAS_DB_TABLE_FIELD* proc_table, /* I: table defining the data to
                                                      be used as procedure 
                                                      arguments */
    int proc_table_length,                      /* I: number of entries in the 
                                                      proc_table */
    int num_records                             /* I: The number of elements in
                                                      any members of the table 
                                                      struct that are declared 
                                                      as array types.  If there 
                                                      are no arrays, then this 
                                                      argument must be 1 */
)
{
    char *sql = NULL;           /* buffer pointer for sql statement */
    int status = SUCCESS;

    /* Build the statement to call the procedure */
    sql = ias_db_stored_proc_get_call_statement(procedure_name, proc_table,
            proc_table_length);
    if (sql == NULL)
    {
        IAS_LOG_ERROR("Unable to create stored procedure call statement");
        return ERROR;
    }

    status = ias_db_execute_using_table(db, sql, "STORED PROC", proc_table,
        proc_table_length, num_records);
    free(sql);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Unable to execute stored procedure");
        return ERROR;
    }

    return SUCCESS;
}

