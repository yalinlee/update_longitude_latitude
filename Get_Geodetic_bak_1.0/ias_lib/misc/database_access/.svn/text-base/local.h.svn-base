#ifndef LOCAL_H
#define LOCAL_H

/*************************************************************************

NAME: local.h

PURPOSE: Header file defining supporting routines that are local to the
    database access library, not intended for public use.

Algorithm References: None

**************************************************************************/

#include <sql.h>
#include "ias_db.h"

/* 
 * Defines the maximum number of rows to operate on at once.  This may be the 
 * number of rows to insert or the number of calls to a stored procedure.  If 
 * the caller provides more rows than this, the underlying database operations 
 * will be broken up into multiple sets. 
 */
#define MAX_ROWS  20000

int ias_db_bind_char_by_index
(
    struct ias_db_query *query_handle,         /* I: Query to bind value for */
    int index,                                 /* I: The index of the bind 
                                                     variable in the SQL 
                                                     statement */
    IAS_DB_PARAMETER_MODE_TYPE parameter_mode, /* I: The input-output type of 
                                                     bind variable */
    char *val,                                 /* I: The value to bind to the 
                                                     bind variable */
    int buf_len,                               /* I: Length of each entry in 
                                                     val */
    IAS_DB_NULL_TYPE *null_flag                /* I: Flag indicating if the 
                                                     value is a NULL */
);

int ias_db_bind_float_by_index
(
    struct ias_db_query *query_handle,         /* I: query to bind value for */
    int index,                                 /* I: The index of the bind 
                                                     variable in the SQL 
                                                     statement */
    IAS_DB_PARAMETER_MODE_TYPE parameter_mode, /* I: The input-output type of 
                                                     bind variable */
    float *val,                                /* I: The value to bind to the 
                                                      bind variable */
    IAS_DB_NULL_TYPE *null_flag                /* I: Flag indicating if the 
                                                      value is a NULL */
);

int ias_db_bind_double_by_index
(
    struct ias_db_query *query_handle,          /* I: query to bind value for */
    int index,                                  /* I: The index of the bind 
                                                      variable in the SQL
                                                      statement */
    IAS_DB_PARAMETER_MODE_TYPE parameter_mode,  /* I: The input-output type of 
                                                      bind variable */
    double *val,                                /* I: The array of doubles to 
                                                      bind */
    IAS_DB_NULL_TYPE *null_flag                 /* I: Flag indicating if the 
                                                      value is a NULL */
);

int ias_db_bind_int_by_index
(
    struct ias_db_query *query_handle,          /* I: query to bind value for */
    int index,                                  /* I: The index of the bind 
                                                      variable in the SQL 
                                                      statement */
    IAS_DB_PARAMETER_MODE_TYPE parameter_mode,  /* I: The input-output type of 
                                                     bind variable */
    int *val,                                   /* I: The value to bind to the 
                                                      bind variable */
    IAS_DB_NULL_TYPE *null_flag                 /* I: Flag indicating if the 
                                                      value is a NULL */
);

int ias_db_bind_short_int_by_index
(
    struct ias_db_query *query_handle,          /* I: query to bind value for */
    int index,                                  /* I: The index of the bind 
                                                      variable in the SQL 
                                                      statement */
    IAS_DB_PARAMETER_MODE_TYPE parameter_mode,  /* I: The input-output type of 
                                                     bind variable */
    short int *val,                             /* I: The value to bind to the 
                                                      bind variable */
    IAS_DB_NULL_TYPE *null_flag                 /* I: Flag indicating if the 
                                                      value is a NULL */
);

int ias_db_bind_date_by_index
(
    struct ias_db_query *query_handle,          /* I: query to bind value for */
    int index,                                  /* I: The index of the bind 
                                                      variable in the SQL 
                                                      statement */
    IAS_DB_PARAMETER_MODE_TYPE parameter_mode,  /* I: The input-output type of 
                                                      bind variable */
    SQL_DATE_STRUCT *date,                      /* I: array of dates to bind */
    IAS_DB_NULL_TYPE *null_flag                 /* I: Flag indicating if the 
                                                      value is a NULL */
);
int ias_db_bind_timestamp_by_index
(
    struct ias_db_query *query_handle,          /* I: query to bind value for */
    int index,                                  /* I: The index of the bind 
                                                      variable in the SQL 
                                                      statement */
    IAS_DB_PARAMETER_MODE_TYPE parameter_mode,  /* I: The input-output type of 
                                                      bind variable */
    SQL_TIMESTAMP_STRUCT *timestamps,           /* I: array of timestamps to 
                                                      bind */
    IAS_DB_NULL_TYPE *null_flag                 /* I: Flag indicating if the 
                                                      value is a NULL */
);

int ias_db_parse_yyyy_mm_dd_time
(
    const char *in_date,           /* I: input date in yyyy-mm-dd format with an
                                         optional "Thh:mm:ss" */
    SQL_TIMESTAMP_STRUCT *out_date /* O: pointer to SQL timestamp structure */
);

int ias_db_parse_yyyy_ddd
(
    const char *in_date,      /* I: input date in yyyy-ddd format */
    int *year,                /* O: year */
    int *month,               /* O: month */
    int *day                  /* O: day */
);

int ias_db_parse_yyyy_ddd_hms
(
    const char *in_date,      /* I: input date in format
                                    yyyy[-|:]ddd[T| |:]HH:MM:SS.sssssssss */
    int *year,                /* O: year */
    int *month,               /* O: month */
    int *day,                 /* O: day */
    int *hours,               /* O: hours */
    int *min,                 /* O: minutes */
    double *seconds           /* O: seconds, including fractional seconds */
);

int ias_db_parse_yyyy_ddd_time
(
    const char *in_date,      /* I: input date in
                                    yyyy[-|:]ddd[T| |:]SSSSS.sssssssss format */
    int *year,                /* O: year */
    int *month,               /* O: month */
    int *day,                 /* O: day */
    double *seconds           /* O: seconds, including fractional seconds */
);

SQLSMALLINT ias_db_get_input_output_type
(
    IAS_DB_PARAMETER_MODE_TYPE parameter_mode /* I: parameter mode */
);

#endif
