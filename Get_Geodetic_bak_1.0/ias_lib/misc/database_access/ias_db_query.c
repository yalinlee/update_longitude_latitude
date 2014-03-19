/******************************************************************************
NAME: ias_db_query

PURPOSE:        
This function queries data from a database using a table defined by the 
calling routine. It allows applications to be isolated from the actual 
mechanics of doing database queries in case the method needs to change.

NOTES:
none

ALGORITHM REFERENCES:
none

******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include "ias_const.h"
#include "ias_math.h"
#include "ias_logging.h"
#include "ias_db_query.h"
#include "local.h"

/* The ias_db_query_get_query_statement routine builds the query 
   statement with bind variables using the table structure passed in.  
   Returns: a null pointer if the statement could not be constructed.
            a pointer to newly allocated memory containing the
              query statement.  It is the invoker's responsibility to 
              free the memory when finished with it. */
char * ias_db_query_get_query_statement
(
    const char *database_table,         /* I: name of database table */
    const IAS_DB_QUERY_TABLE* query_table,/* I: table defining the data to be
                                                  queried from the database */
    int query_table_length,             /* I: number of entries in the
                                              query_table */
    const char *where_clause            /* I: where clause for the query */
)
{
    char *sql = NULL;   /* buffer pointer for building sql query statement */
    int i;
    /* allow some slop for extra terms in the query such as "select" */
    int statement_length = 200 + strlen(database_table) + strlen(where_clause);

    /* estimate the amount of memory needed for the query statement */
    for (i = 0; i < query_table_length; i++)
    {
        const IAS_DB_QUERY_TABLE *table_ptr = &query_table[i];
        /* Since this is using binds instead of actual values, all that is 
           needed to be known for the length is the column name.  The strlen
           function takes into account the column name in the query portion
           and adding 6 takes into account the question mark in the values
           portion of the query and any commas and spaces that may be added.*/
        statement_length += strlen(table_ptr->field_name) + 6; 

        /* If the current data type is a date time that requires more than
           milliseconds of precision, then it gets split into two columns
           in the database.  So add the length of another column to the
           statement length.  For these columns, the word DATE is replaced
           with "TIME" in the column name. */
        switch (table_ptr->data_type)
        {
            case IAS_QUERY_DATETIME_NS_DOY:
            case IAS_QUERY_DATETIME_NS_DOY_SOD:
            case IAS_QUERY_DATETIME_NS_DOY_SOD_ARRAY:
                statement_length += strlen(table_ptr->field_name) + 6;
                break;
        }
    }

    /* allocate space for the query statement */
    sql = malloc(statement_length * sizeof(char));
    if (!sql)
    {
        /* error allocating memory */
        IAS_LOG_ERROR("Allocating memory");
        return NULL;
    }

    /* build the query statement for the field names */
    strcpy(sql,"select ");

    for (i = 0; i < query_table_length; i++)
    {
        if ( i != 0 )
            strcat(sql, ", ");

        strcat(sql, query_table[i].field_name);

        /* If the current data type is a date_time that requires
           the date to be in one column and the time in another,
           then add the time column to the query statement.
           The method used will replace the word DATE in the column
           name that contains the date with the word TIME.  For example
           if the DATE column passed in is called DCE_START_DATE, the
           column containing the seconds from the beginning of the day
           will be called DCE_START_TIME. */
        char *time_ptr = NULL;
        char *tmp_ptr = NULL;

        switch ( query_table[i].data_type )
        {
            case IAS_QUERY_DATETIME_NS_DOY:
            case IAS_QUERY_DATETIME_NS_DOY_SOD:
            case IAS_QUERY_DATETIME_NS_DOY_SOD_ARRAY:
                /* replace the word DATE with the word TIME */
                time_ptr = strdup(query_table[i].field_name);

                if (time_ptr == NULL)
                {
                    IAS_LOG_ERROR ("Allocating memory for a copy of the "
                            "date field name: %s",query_table[i].field_name);
                    free(sql);
                    return NULL;
                }

                tmp_ptr = strstr(time_ptr, "_date");

                if (tmp_ptr == NULL)
                {
                    IAS_LOG_ERROR ("Creating the query statement."
                            "  'date' not found in date field name: %s",
                            query_table[i].field_name);
                    free(time_ptr);
                    free(sql);
                    return NULL;
                }

                /* Now that "_date" was found, replace it with "_time" */
                strncpy(tmp_ptr, "_time", 5);

                /* Add the additional column name to the query statement */
                strcat(sql, ", ");
                strcat(sql, time_ptr);

                free(time_ptr);
                break;
        }
    }
    
    sprintf(sql, "%s from %s", sql, database_table);
    if (strlen(where_clause) != 0)
        sprintf(sql, "%s where %s", sql, where_clause);

    return sql;
}


/* ias_db_query_using_table performs a query on a database table using a 
 * C table (array of structures) to hold the data retrieved from the database 
 *
 * Returns: SUCCESS if there are no errors
 *          ERROR if the disconnect, or library
 *                close can not occur.
 */
int ias_db_query_using_table
(
    struct ias_db_connection *db,       /* I: database connection */
    const char *database_table,         /* I: name of database table */
    const IAS_DB_QUERY_TABLE* query_table, /* I: table defining the data to be
                                                 queryed into the database */
    int query_table_length,            /* I: number of entries in the
                                              query_table */
    const char *where_clause,          /* I: where clause for the query */
    int num_records                    /* I: The number of elements in any 
                                              members of the table struct
                                              that are declared as array types.
                                              If there are no arrays, then this
                                              argument must be 1 */
)
{
    char *sql = NULL;   /* buffer pointer for building sql query statement */
    int i,j,currRecord;
    int status = SUCCESS;
    struct ias_db_query *query = NULL;
    double temp_double; /* temporary double pointer  */   
    char temp_date[20]; /* temporary date string */
    int temp_int;       /* temporary integer pointer */
    int temp_doy;       /* temporary day of year, used in date conversions */
    SQL_TIMESTAMP_STRUCT temp_timestamp; /* temporary timestamp */

    sql = ias_db_query_get_query_statement(database_table, query_table, 
            query_table_length, where_clause);

    /* THe query string is built, now prepare the statement for binding */
    query = ias_db_perform_query(db, sql);

    /* after the prepare is done, the character string holding the 
       query is no longer needed */
    free(sql);

    if (!ias_db_query_was_successful(query))
    {
        char buffer[200];
        ias_db_query_get_error_message(query,buffer,200);

        IAS_LOG_ERROR ("Creating and preparing the table query: %s",
                buffer);
        ias_db_query_close(query);
        return ERROR;
    }

    /* Retrieve the results of the query */
    if(!ias_db_query_next(query))
    {
        IAS_LOG_ERROR("Retrieving results from DB query");
        return ERROR;
    }

    /* Bind the values that will not change on each query.  In other words,
       the non-array stuff */
    for(i = 0, j = 0; i < query_table_length; i++, j++)
    {
        const IAS_DB_QUERY_TABLE *table_ptr = &query_table[i];

        switch (table_ptr->data_type)
        {
            case IAS_QUERY_DOUBLE:
                status = ias_db_query_double_value(query, &temp_double, j); 
                if (status != SUCCESS)
                {
                    char buffer[200];
                    ias_db_query_get_error_message(query,buffer,200);
                    IAS_LOG_ERROR("IAS Database Query Double Value: %s", 
                                   buffer);
                }
                *(double *)table_ptr->data_ptr = temp_double;
                break;
            case IAS_QUERY_FLOAT:
                status = ias_db_query_double_value(query, &temp_double, j);
                if (status != SUCCESS)
                {
                    char buffer[200];
                    ias_db_query_get_error_message(query,buffer,200);
                    IAS_LOG_ERROR("IAS Database Query Float Value: %s",
                                   buffer);
                }
                *(float *)table_ptr->data_ptr = (float)temp_double;
                break;
            case IAS_QUERY_INT:
                status = ias_db_query_int_value(query, &temp_int, j);
                if (status != SUCCESS)
                {
                    char buffer[200];
                    ias_db_query_get_error_message(query,buffer,200);
                    IAS_LOG_ERROR("IAS Database Query Integer Value: %s",
                                   buffer);
                }
                *(int *)table_ptr->data_ptr = temp_int;
                break;
            case IAS_QUERY_INT16:
                status = ias_db_query_int_value(query, &temp_int, j);
                if (status != SUCCESS)
                {
                    char buffer[200];
                    ias_db_query_get_error_message(query,buffer,200);
                    IAS_LOG_ERROR("IAS Database Query Short Float Value: %s",
                                   buffer);
                }
                *(short int *)table_ptr->data_ptr = (short int)temp_int;
                break;
            case IAS_QUERY_STRING:
                ias_db_query_string_value(query, j,
                    (char *)(table_ptr->data_ptr), table_ptr->length);
                break;
            case IAS_QUERY_DATETIME:
                /* Just a single value since there are no fractional seconds */ 
                ias_db_query_datetime_value(query, j,
                        (char *)(table_ptr->data_ptr),
                        table_ptr->length);
                break;
            case IAS_QUERY_DATETIME_DOY:
                /* Just a single value since there are no fractional seconds 
                 * beyond milliseconds */ 
                ias_db_query_datetime_value(query, j, temp_date, 
                        sizeof(temp_date));
                /* Reformat the date as YYYY-DDD */
                status = ias_db_parse_yyyy_mm_dd_time(temp_date,
                        &temp_timestamp);
                ias_math_convert_month_day_to_doy(temp_timestamp.month, 
                        temp_timestamp.day, temp_timestamp.year, &temp_doy);
                snprintf((char*)(table_ptr->data_ptr), table_ptr->length,
                         "%04d-%03d", temp_timestamp.year, temp_doy);
                break;
            case IAS_QUERY_DATETIME_NS_DOY:
                /* Two columns since Qt and many databases
                   don't yet support precision beyond milliseconds */

                /* First get the date portion returned as 
                 * YYYY-MM-DDTHH:MM:SS.SSSSSS, put into a timestamp structure,
                 * and convert to DOY */
                ias_db_query_datetime_value(query, j, temp_date, 
                        sizeof(temp_date));
                status = ias_db_parse_yyyy_mm_dd_time(temp_date,
                        &temp_timestamp);
                ias_math_convert_month_day_to_doy(temp_timestamp.month, 
                        temp_timestamp.day, temp_timestamp.year, &temp_doy);

                /* Next get the double seconds and divide into parts */
                j++;
                status = ias_db_query_double_value(query, &temp_double, j); 
                if (status != SUCCESS)
                {
                    char buffer[200];
                    ias_db_query_get_error_message(query,buffer,200);
                    IAS_LOG_ERROR("IAS Database Query Double Value: %s", 
                                   buffer);
                }
                int hour = temp_double / 3600;
                temp_double -= hour * 3600;
                int minute = temp_double / 60;
                temp_double -= minute * 60;

                /* Reformat the date as yyyy-dddTHH:MM:SS.sssssssss, */
                /* replacing the time part of the timestamp with the time 
                 * from the 2nd column */
                snprintf((char*)(table_ptr->data_ptr), table_ptr->length,
                         "%04d-%03dT%02d:%02d:%012.9f", temp_timestamp.year, 
                         temp_doy, hour, minute, temp_double);

                break;
            case IAS_QUERY_DATETIME_NS_DOY_SOD:
                /* Two columns since Qt and many databases
                   don't yet support precision beyond milliseconds */

                /* First get the date portion returned as 
                 * YYYY-MM-DDTHH:MM:SS.SSSSSS  and convert to DOY */
                ias_db_query_datetime_value(query, j, temp_date, 
                        sizeof(temp_date));
                status = ias_db_parse_yyyy_mm_dd_time(temp_date,
                        &temp_timestamp);
                ias_math_convert_month_day_to_doy(temp_timestamp.month, 
                        temp_timestamp.day, temp_timestamp.year, &temp_doy);

                /* Next get the double seconds */
                j++;
                status = ias_db_query_double_value(query, &temp_double, j); 
                if (status != SUCCESS)
                {
                    char buffer[200];
                    ias_db_query_get_error_message(query,buffer,200);
                    IAS_LOG_ERROR("IAS Database Query Double Value: %s", 
                                   buffer);
                }

                /* Reformat the date as yyyy-dddTSSSSS.sssssssss, */
                /* replacing the time part of the timestamp with the time 
                 * from the 2nd column */
                snprintf((char*)(table_ptr->data_ptr), table_ptr->length,
                         "%04d-%03dT%015.9f", temp_timestamp.year, temp_doy,
                         temp_double);
                break;

            case IAS_QUERY_DATETIME_NS_DOY_SOD_ARRAY: 
                /* If it's any of the datetime arrays that require two columns,
                   then increment J so that it skips the next column */
                j++;
                break;
        }

        if (status == ERROR )
        {
            IAS_LOG_ERROR ("Query value for field: %s",
                    table_ptr->field_name);
            ias_db_query_close(query);
            return ERROR;
        }

    }

    /* Now loop through the query for each array element */
    for (currRecord = 0; currRecord < num_records; currRecord++)
    {
        for (i = 0, j = 0; i < query_table_length; i++, j++)
        {
            const IAS_DB_QUERY_TABLE *table_ptr = &query_table[i];

            switch (table_ptr->data_type)
            {
                case IAS_QUERY_DOUBLE_ARRAY:
                    status = ias_db_query_double_value(query, &temp_double, j);
                    if (status != SUCCESS)
                    {
                        char buffer[200];
                        ias_db_query_get_error_message(query,buffer,200);
                        IAS_LOG_ERROR("IAS Database Query Double Value: %s",
                                       buffer);
                    }
                    ((double *)(table_ptr->data_ptr))[currRecord] 
                                = temp_double;
                    break;
                case IAS_QUERY_FLOAT_ARRAY:
                    status = ias_db_query_double_value(query, &temp_double, j);
                    if (status != SUCCESS)
                    {
                        char buffer[200];
                        ias_db_query_get_error_message(query,buffer,200);
                        IAS_LOG_ERROR("IAS Database Query Float Value: %s",
                                       buffer);
                    }
                    ((float *)(table_ptr->data_ptr))[currRecord] 
                                = (float)temp_double;
                    break;
                case IAS_QUERY_INT_ARRAY:
                     status = ias_db_query_int_value(query, &temp_int, j);
                    if (status != SUCCESS)
                    {
                        char buffer[200];
                        ias_db_query_get_error_message(query,buffer,200);
                        IAS_LOG_ERROR("IAS Database Query Integer Value: %s",
                                       buffer);
                    }
                    ((int *)(table_ptr->data_ptr))[currRecord] = temp_int;
                    break;
                case IAS_QUERY_INT16_ARRAY:
                    status = ias_db_query_int_value(query, &temp_int, j);
                    if (status != SUCCESS)
                    {
                        char buffer[200];
                        ias_db_query_get_error_message(query,buffer,200);
                        IAS_LOG_ERROR("IAS Database Query Short Integer Value:" 
                                       "%s", buffer);
                    }
                    ((short int *)table_ptr->data_ptr)[currRecord] 
                                    = (short int)temp_int;
                    break;
                case IAS_QUERY_STRING_ARRAY:
                    ias_db_query_string_value(query, j,
                        &(((char*)(table_ptr->data_ptr))[currRecord
                                    * table_ptr->length]),
                        table_ptr->length);
                    break;
                case IAS_QUERY_DATETIME_NS_DOY_SOD_ARRAY:
                    /* Two columns to bind since Qt and many databases
                       don't yet support precision beyond milliseconds */

                    /* First get the date portion returned as 
                     * YYYY-MM-DDTHH:MM:SS.SSSSSS  and convert to DOY */
                    ias_db_query_datetime_value(query, j, temp_date, 
                            sizeof(temp_date));
                    status = ias_db_parse_yyyy_mm_dd_time(temp_date,
                            &temp_timestamp);
                    ias_math_convert_month_day_to_doy(temp_timestamp.month, 
                            temp_timestamp.day, temp_timestamp.year, &temp_doy);

                    /* Next get the double seconds */
                    j++;
                    status = ias_db_query_double_value(query, &temp_double, j); 
                    if (status != SUCCESS)
                    {
                        char buffer[200];
                        ias_db_query_get_error_message(query,buffer,200);
                        IAS_LOG_ERROR("IAS Database Query Double Value: %s", 
                                       buffer);
                    }

                    /* Reformat the date as yyyy-dddTSSSSS.sssssssss, */
                    /* replacing the time part of the timestamp with the time 
                     * from the 2nd column */
                    snprintf(&(((char*)(table_ptr->data_ptr))[currRecord
                            * table_ptr->length]),
                            table_ptr->length,
                             "%04d-%03dT%015.9f", temp_timestamp.year, 
                             temp_doy, temp_double);
                    break;
                case IAS_QUERY_DATETIME_NS_DOY:
                case IAS_QUERY_DATETIME_NS_DOY_SOD:
                    /* If it's any of the datetime's that aren't arrays, but
                       require two columns, then increment J so that it 
                       skips the next column in the list of bind variables. */
                    j++;
                    break;
            }

            if (status == ERROR )
            {
                IAS_LOG_ERROR ("Binding value for field: %s",
                        table_ptr->field_name);
                ias_db_query_close(query);
                return ERROR;
            }
        }

        if (currRecord + 1 < num_records)
        {
            /* Retrieve the next results of the query */
            if(!ias_db_query_next(query))
            {
                IAS_LOG_ERROR("Retrieving next results from DB query");
                return ERROR;
            }
        }

    }

    ias_db_query_close(query);

    return SUCCESS;
}

/* ias_db_query_connect opens a connection to a database
 *  The connection can be used for multiple queries.
 *
 * Returns: A pointer to the ias_db_connection structure on success, NULL
 *          on any failure, including if the database library can not be
 *          initialized, and connection can not be made.
 */

