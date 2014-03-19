#ifndef IAS_DB_H
#define IAS_DB_H

/*************************************************************************

NAME: ias_db.h

PURPOSE: Header file defining database access types and functions.  See 
    the comments in ias_db.c for more information.

Algorithm References: None

**************************************************************************/

#include <limits.h>     /* include this so __WORDSIZE is visible */

#define IAS_SQL_RTN_MSGLEN 200 /* length of sql error return message */

#define IAS_DB_ROLLBACK 0       /* Database changes are to be rolled back */
#define IAS_DB_COMMIT 1         /* Database changes are to be committed */

/* define the input-output options for arguments & bind variables */
typedef enum
{
    IAS_DB_PARAMETER_MODE_INPUT = 0,       /* input argument */
    IAS_DB_PARAMETER_MODE_OUTPUT = 1,      /* output argument */
    IAS_DB_PARAMETER_MODE_INPUTOUTPUT = 2, /* input/output argument */
    IAS_DB_PARAMETER_MODE_RETURN = 3       /* return value */
} IAS_DB_PARAMETER_MODE_TYPE;

/* define the type of the null indicator variable.  It changes between 64-bit
   and 32-bit platforms. */
#if __WORDSIZE == 64
typedef unsigned long IAS_DB_NULL_TYPE;
#else
typedef int IAS_DB_NULL_TYPE;
#endif

/* provide forward references to the database structures.  Note that the 
   actual contents of the structures are not visible to the users of the 
   library. */
struct ias_db_connection;
struct ias_db_query;

/* function prototypes */
int ias_db_initialize_database_lib();

void ias_db_close_database_lib();

struct ias_db_connection* ias_db_connect_to_database
(
    const char *database_name,  /* I: Database name to connect to */
    const char *user_name,      /* I: user name to use for the connection */
    const char *password,       /* I: user's password for the database */
    const char *host            /* I: host name where the database is located */
);

int ias_db_start_transaction
(
    struct ias_db_connection *db /* I: database connection for transaction */
);

int ias_db_rollback_transaction
(
    struct ias_db_connection *db /* I: database connection for transaction */
);

int ias_db_commit_transaction
(
    struct ias_db_connection *db /* I: database connection for transaction */
);

struct ias_db_connection* ias_db_connect_with_transaction
(
    const char *database_name,  /* I: Database name to connect to */
    const char *user_name,      /* I: user name to use for the connection */
    const char *password,       /* I: user's password for the database */
    const char *host            /* I: host name where the database is located */
);

const char *ias_db_connect_last_error
(
    struct ias_db_connection* db /* I: database connection to close */
);

void ias_db_close_connection
(
    struct ias_db_connection* db /* I: database connection to close */
);

int ias_db_disconnect_with_transaction
(
    struct ias_db_connection *db, /* I/O: Database connection */
    int commit                    /* I: commit flag: IAS_DB_COMMIT means commit
                                        and disconnect, IAS_DB_ROLLBACK means
                                        rollback and disconnect */
);

struct ias_db_query* ias_db_perform_query
(
    struct ias_db_connection* db,   /* I: database connection for query */
    const char *sql_command         /* I: SQL command */
);

int ias_db_query_was_successful
(
    struct ias_db_query* query_handle   /* I: query to check for success */
);

void ias_db_query_get_error_message
(
    struct ias_db_query* query_handle,  /* I: query to get error message for */
    char *msg,                          /* I/O: pointer error message buffer */
    int msg_size                        /* I: size of msg buffer */
);

int ias_db_query_get_modified_rows
(
     struct ias_db_query* query_handle   /* I: query to get error message for */
);

int ias_db_query_is_active
(
    struct ias_db_query* query_handle   /* I: query to check for being active */
);

int ias_db_query_next
(
    struct ias_db_query* query_handle   /* I: query to advance to next record */
);

int ias_db_query_is_null
(
    struct ias_db_query* query_handle,  /* I: query to check */
    int index                           /* I: index in query to check for null*/
);

void ias_db_query_string_value
(
    struct ias_db_query* query_handle, /* I: query to extract string from */
    int index,              /* I: index in query to extract results from */
    char *buffer,           /* O: pointer to buffer for returning the results */
    int buffer_size         /* I: size of buffer, in bytes */
);

void ias_db_query_datetime_value
(
    struct ias_db_query* query_handle, /* I: query to extract string from */
    int index,              /* I: index in query to extract results from */
    char *buffer,           /* O: pointer to buffer for returning the results */
    int buffer_size         /* I: size of buffer, in bytes */
);

int ias_db_query_int_value
(
    struct ias_db_query* query_handle, /* I: query to extract integer from */
    int *int_value,                    /* O: returned integer query value  */
    int index                          /* I: index in query to extract results 
                                             from */
);

int ias_db_query_double_value
(
    struct ias_db_query* query_handle, /* I: query to extract double from */
    double *double_value,              /* O: returned integer query value  */
    int index                          /* I: in in query to extract results 
                                             from */
);

void ias_db_query_close
(
    struct ias_db_query* query_handle   /* I: query to close */
);

int ias_db_query_end
(
struct ias_db_query* query_handle       /* I: query to finish */
);

struct ias_db_query* ias_db_prepare_query
(
    struct ias_db_connection* db,       /* I: database connection for query */
    const char *sql_command             /* I: SQL command */
);

int ias_db_set_rows_to_insert
(
    struct ias_db_query* query_handle,  /* I: query to set rows for */
    int rows                            /* I: rows to insert at once */
);

int ias_db_exec_prepared_query
(
    struct ias_db_query *theQuery       /* I: query to execute */
);

#endif
