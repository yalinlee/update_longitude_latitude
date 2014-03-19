/****************************************************************************
* Name: ias_db
*
* Purpose:
*   This module implements a wrapper around the database operations to hide
*   which underlying database API is being used.  This implementation utilizes
*   the ODBC API, but it should be possible to use a different API with 
*   minimal to no impact on the code that uses this module (note that this
*   proved to be true when converting it from the previous use of the Qt
*   SQL classes).
*
* Notes:   
*   A good reference for the ODBC API is available at
*      http://msdn.microsoft.com/en-us/library/ms710252(VS.85).aspx
*
*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sql.h>
#include <sqlext.h>
#include "ias_db.h"
#include "ias_const.h"
#include "ias_math.h"
#include "ias_logging.h"
#include "local.h"


/* The ias_db_connection structure is used to store information about a
   database connection. */
struct ias_db_connection
{
    SQLHDBC database;               /* handle to the database connection */
    int transaction_started;        /* flag to indicate a transaction has been 
                                      started */
    char last_connection_error[800];/* string for last connection error 
                                       message */
};

/* The ias_db_query structure is used to store state information about a 
   query. */
struct ias_db_query
{
    SQLHSTMT query;
    SQLRETURN query_ret;
    int is_active; /* 1 = true; 0 = false */
    int rows_to_insert;
};

/* ODBC environment handle */
static SQLHENV env;

/* flag to indicate the environment is initialized */
static int initialized = 0;

/* The lastConnectionError contains the last error encountered by 
   ias_db_connect_to_database. The contents are retrieved using the 
   ias_db_connect_last_error routine. */
static char lastConnectionError[800];

/* get_error is a helper routine to query an ODBC handle for any error
   message that might be associated with it */
static void get_error
(
    SQLHANDLE handle,       /* I: ODBC handle to get the errors from */
    SQLSMALLINT type,       /* I: handle type */
    char *message_buffer,   /* O: pointer to the buffer for the error message */
    int buffer_size         /* I: size of the message_buffer */
)
{
    SQLINTEGER record = 1;
    SQLINTEGER native;
    SQLCHAR state[7];
    SQLCHAR text[256];
    SQLSMALLINT len;
    SQLRETURN ret;
    int index = 0;

    *message_buffer = '\0';
    do 
    {
        ret = SQLGetDiagRec(type, handle, record, state, &native, text,
                            sizeof(text), &len);
        if (SQL_SUCCEEDED(ret))
        {
            if (index < buffer_size)
            {
                index += snprintf(&message_buffer[index], buffer_size - index,
                              "%s:%ld:%ld:%s\n", state, (long int)record,
                              (long int)native, text);
            }
        }
        record++;
    } while (ret == SQL_SUCCESS);
}

/****************************************************************************
* Name: ias_db_initialize_database_lib
*
* Description: initializes the library for use.  If an error occurs, any error
*   message can be obtained by calling ias_db_connect_last_error.
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_db_initialize_database_lib()
{
    /* if the database environment isn't initialized, initialize it */
    if (!initialized)
    {
        SQLRETURN ret;

        /* clear the last connection error */
        snprintf(lastConnectionError,sizeof(lastConnectionError),
                 "No Error Detected");

        /* allocate an environment handle and indicate ODBC 3 support is
           wanted */
        ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
        if (!SQL_SUCCEEDED(ret))
        {
            get_error(SQL_NULL_HANDLE, SQL_HANDLE_ENV, lastConnectionError,
                      sizeof(lastConnectionError));
            return ERROR;
        }

        ret = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3,
                            0);
        if (!SQL_SUCCEEDED(ret))
        {
            get_error(env, SQL_HANDLE_ENV, lastConnectionError,
                      sizeof(lastConnectionError));
            SQLFreeHandle(SQL_HANDLE_ENV, env);
            return ERROR;
        }

        initialized = 1;
    }
    else
        IAS_LOG_WARNING("Database already initialized");

    /* set up the ODBC environment successfully */
    return SUCCESS;
}

/****************************************************************************
* Name: ias_db_close_database_lib
*
* Description: frees any resources allocated by the library
*****************************************************************************/
void ias_db_close_database_lib()
{
    if (initialized)
    {
        SQLFreeHandle(SQL_HANDLE_ENV, env);
    }
    initialized = 0;
}

/****************************************************************************
* Name: ias_db_connect_to_database
*
* Description: opens a connection to a database.  The connection can be used
*   for multiple queries.
*
* Returns: A pointer to the ias_db_connection structure on success, NULL
*          on failure
*****************************************************************************/
struct ias_db_connection* ias_db_connect_to_database
(
    const char *database_name,  /* I: Database name to connect to */
    const char *user_name,      /* I: user name to use for the connection */
    const char *password,       /* I: user's password for the database */
    const char *host            /* I: host name where the database is located */
)
{
    struct ias_db_connection* db;
    char connect_string[200];
    SQLRETURN ret;
    SQLCHAR outstr[1024];
    SQLSMALLINT outstrlen;
    SQLSMALLINT transactions;

    /* clear the last connection error */
    snprintf(lastConnectionError,sizeof(lastConnectionError),
             "No Error Detected");

    /* if the library hasn't been initialized, return NULL as an error
       indication */
    if (!initialized)
    {
        snprintf(lastConnectionError,sizeof(lastConnectionError),
                 "ias_db library not initialized");
        return NULL;
    }

    /* allocate space for the ias_db_connection object */
    db = (struct ias_db_connection*)malloc(sizeof(*db));
    if (!db)
    {
        /* couldn't allocate memory so set the error message and return NULL */
        snprintf(lastConnectionError,sizeof(lastConnectionError),
                 "Memory allocation error");
        return NULL;
    }
    db->transaction_started = 0;
    
    /* allocate the connection handle */
    ret = SQLAllocHandle(SQL_HANDLE_DBC, env, &db->database);
    if (!SQL_SUCCEEDED(ret))
    {
        free(db);
        snprintf(lastConnectionError,sizeof(lastConnectionError),
                 "Error allocating a connection handle");
        return NULL;
    }

    /* build the connection string */
    snprintf(connect_string, sizeof(connect_string), "DSN=%s;UID=%s;PWD=%s",
             database_name, user_name, password);

    /* connect to the database */
    ret = SQLDriverConnect(db->database, (void *)1, (SQLCHAR*) connect_string, 
             SQL_NTS, outstr, sizeof(outstr), &outstrlen, SQL_DRIVER_NOPROMPT);
    if (!SQL_SUCCEEDED(ret))
    {
        /* get error message and free the resources */
        get_error(db->database, SQL_HANDLE_DBC, lastConnectionError,
                  sizeof(lastConnectionError));
        SQLFreeHandle(SQL_HANDLE_DBC, db->database);
        free(db);
        return NULL;
    }

    /* make sure the driver has transaction support */
    ret = SQLGetInfo(db->database, SQL_TXN_CAPABLE, &transactions, 0, NULL);
    if (!SQL_SUCCEEDED(ret) || (transactions == SQL_TC_NONE))
    {
        SQLDisconnect(db->database);
        SQLFreeHandle(SQL_HANDLE_DBC, db->database);
        free(db);
        snprintf(lastConnectionError,sizeof(lastConnectionError),
                 "Database does not support transactions");
        return NULL;
    }

    /* until a transaction is requested, turn on autocommit */
    ret = SQLSetConnectAttr(db->database, SQL_ATTR_AUTOCOMMIT,
                            (SQLPOINTER)SQL_AUTOCOMMIT_ON, SQL_IS_INTEGER);
    if (!SQL_SUCCEEDED(ret))
    {
        int len;

        snprintf(lastConnectionError,sizeof(lastConnectionError),
                 "Unable to turn on autocommit: ");
        len = strlen(lastConnectionError);
        get_error(db->database, SQL_HANDLE_DBC, &lastConnectionError[len],
                  sizeof(lastConnectionError) - len);
        SQLDisconnect(db->database);
        SQLFreeHandle(SQL_HANDLE_DBC, db->database);
        free(db);
        return NULL;
    }

    /* Copy the lastConnectionError message to last_connection_error pointer
       in ias_db_connection structure */
    strcpy(db->last_connection_error, lastConnectionError); 
    return db;
}

/****************************************************************************
* Name: ias_db_start_transaction
*
* Description: ias_db_start_transaction is called to start a database
*   transaction that will need to be either committed or rolled back (rolled
*   back automatically if the database is closed without committing or the
*   program exits unexpectedly).
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_db_start_transaction
(
    struct ias_db_connection *db /* I: database connection for transaction */
)
{
    SQLRETURN ret;

    /* make sure there isn't a transaction already taking place */
    if (db->transaction_started)
    {
        snprintf(db->last_connection_error,sizeof(db->last_connection_error),
                 "Programming error: starting transaction when one is already"
                 " active");
        return ERROR;
    }

    /* turn off autocommit */
    ret = SQLSetConnectAttr(db->database, SQL_ATTR_AUTOCOMMIT,
                            (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_IS_INTEGER);
    if (SQL_SUCCEEDED(ret))
    {
        db->transaction_started = 1;
        return SUCCESS;
    }
    else
    {
        get_error(db->database, SQL_HANDLE_DBC, db->last_connection_error,
                  sizeof(db->last_connection_error));
        return ERROR;
    }
}

/****************************************************************************
* Name: ias_db_rollback_transaction
*
* Description: this routine is called to cancel the current transaction and
*   rollback the database state to the state that was present when
*   ias_db_start_transaction was called.  It is an error to call this routine
*   if a transaction hasn't been started.
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_db_rollback_transaction
(
    struct ias_db_connection *db /* I: database connection for transaction */
)
{
    SQLRETURN ret;

    /* make sure there is a transaction already taking place */
    if (!db->transaction_started)
    {
        snprintf(db->last_connection_error,sizeof(db->last_connection_error),
                 "Programming error: trying to rollback transaction when one "
                 "is not active");
        return ERROR;
    }

    db->transaction_started = 0;

    ret = SQLEndTran(SQL_HANDLE_DBC, db->database, SQL_ROLLBACK);
    if (SQL_SUCCEEDED(ret))
    {
        /* Autocommit be turned back on after a rollback */
        ret = SQLSetConnectAttr(db->database, SQL_ATTR_AUTOCOMMIT,
              (SQLPOINTER)SQL_AUTOCOMMIT_ON, SQL_IS_INTEGER); 
        return SUCCESS;
    }
    else
    {
        get_error(db->database, SQL_HANDLE_DBC, db->last_connection_error,
                  sizeof(db->last_connection_error));
        return ERROR;
    }
}

/****************************************************************************
* Name: ias_db_commit_transaction
*
* Description: This routine is called to commit the changes made to the
*   database since the ias_db_start_transaction call.  It is an error to call
*   this routine if a transaction hasn't been started.
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_db_commit_transaction
(
    struct ias_db_connection *db /* I: database connection for transaction */
)
{
    SQLRETURN ret;

    /* make sure there is a transaction already taking place */
    if (!db->transaction_started)
    {
        snprintf(db->last_connection_error,sizeof(db->last_connection_error),
                 "Programming error: trying to commit transaction when one "
                 "is not active");
        return ERROR;
    }

    db->transaction_started = 0;

    ret = SQLEndTran(SQL_HANDLE_DBC, db->database, SQL_COMMIT);
    if (SQL_SUCCEEDED(ret))
    {
        /* Autocommit be turned back on after a commit */
        ret = SQLSetConnectAttr(db->database, SQL_ATTR_AUTOCOMMIT,
              (SQLPOINTER)SQL_AUTOCOMMIT_ON, SQL_IS_INTEGER); 
        return SUCCESS; /* succeeded */
    }
    else
    {
        get_error(db->database, SQL_HANDLE_DBC, db->last_connection_error,
                  sizeof(db->last_connection_error));
        return ERROR;
    }
}

/****************************************************************************
* Name: ias_db_connect_with_transaction
*
* Description: This is a convenience routine that initializes the database 
*     library, opens a connection to the database, and begins a transaction.  
*     The connection can be used for multiple queries.
*
* Returns: A pointer to the ias_db_connection structure on success, 
*          NULL on failure
*****************************************************************************/
struct ias_db_connection* ias_db_connect_with_transaction
(
    const char *database_name,  /* I: Database name to connect to */
    const char *user_name,      /* I: user name to use for the connection */
    const char *password,       /* I: user's password for the database */
    const char *host            /* I: host name where the database is located */
)
{
    struct ias_db_connection *db = NULL;

    /* Initialize the library */
    if (ias_db_initialize_database_lib() != SUCCESS)
    {
        IAS_LOG_ERROR("Initializing database library");
        return NULL;
    }

    /* connect to the database */
    db = ias_db_connect_to_database(database_name, user_name, password, host);

    if (!db)
    {
        IAS_LOG_ERROR("Connecting to database: %s",
                ias_db_connect_last_error(db));
        ias_db_close_database_lib();
        return NULL;
    }

    /* start the transaction */
    if (ias_db_start_transaction(db) != SUCCESS)
    {
        IAS_LOG_ERROR("Starting database transaction: %s",
                ias_db_connect_last_error(db));
        ias_db_close_connection(db);
        ias_db_close_database_lib();
        return NULL;
    }
    return db;
}

/****************************************************************************
* Name: ias_db_connect_last_error
*
* Description: returns the error message related to the last operation dealing
*     with a database connection.
*
* Returns: a pointer to a character buffer managed by this library.
*****************************************************************************/
const char *ias_db_connect_last_error
(
    struct ias_db_connection *db /* I: database connection for transaction */ 
)
{
    if (db == NULL)
    {
        return lastConnectionError;
    }
    else
    {
        return db->last_connection_error;
    }
}

/****************************************************************************
* Name: ias_db_close_connection
*
* Description: closes a previously opened database connection.  If it is called
*   with an active transaction taking place, the transaction is rolled back.
*****************************************************************************/
void ias_db_close_connection
(
    struct ias_db_connection* db /* I: database connection to close */
)
{
    /* if an uncommitted transaction is active, roll it back */
    if (db->transaction_started)
        ias_db_rollback_transaction(db);

    SQLDisconnect(db->database);
    SQLFreeHandle(SQL_HANDLE_DBC, db->database);

    free(db);
}

/****************************************************************************
* Name: ias_db_disconnect_with_transaction
*
* Description: This is a convenience routine that completes the transaction, 
*     disconnects from the database, and closes the database library.
*     If the commit argument is 1, it will commit the transaction before closing
*     the database library.  If the commit argument is 0, then it will roll back
*     the transaction before closing the database library.  If the commit 
*     argument is anything else, it will just close the database library and 
*     disconnect.
*     NOTE: If called with anything other than 1 or 0 for the commit
*         argument, it is dependent upon the database being used as
*         to what action will take place with any uncommitted
*         transaction(s).
*
* Returns: SUCCESS if there are no errors
*          ERROR if the commit, rollback, disconnect, or library
*                close can not occur.
*****************************************************************************/
int ias_db_disconnect_with_transaction
(
    struct ias_db_connection *db, /* I/O: Database connection */
    int commit                    /* I: commit flag: IAS_DB_COMMIT means commit
                                        and disconnect, IAS_DB_ROLLBACK means
                                        rollback and disconnect */
)
{
    int status = SUCCESS;

    if (commit == IAS_DB_COMMIT)
    {
        status = ias_db_commit_transaction(db);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Committing transaction: %s",
                    ias_db_connect_last_error(db));
        }
    }
    else if (commit == IAS_DB_ROLLBACK)
    {
        status = ias_db_rollback_transaction(db);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Rolling back transaction: %s",
                    ias_db_connect_last_error(db));
        }
    }

    /* close the database connection and and database library */
    ias_db_close_connection(db);
    ias_db_close_database_lib();

    return status;
}


/****************************************************************************
* Name: ias_db_perform_query performs the query in sql_command using the
*   provided database connection.  
*
* Notes: 
*  - the sql_command is simply a string holding the SQL command to perform.
*    Every effort should be made to ensure the SQL statements do not use
*    proprietary SQL from a particular database vendor.
*  - every ias_db_perform_query call should be matched with a
*    ias_db_query_close call to release the resources used by the query.
*
* Returns: pointer to a ias_db_query structure if successful, NULL if a 
*   failure happens.  Note that even if non-NULL is returned,
*   ias_db_query_was_successful needs to be called to verify the query really
*   worked.
*****************************************************************************/
struct ias_db_query* ias_db_perform_query
(
    struct ias_db_connection* db,   /* I: database connection for query */
    const char *sql_command         /* I: SQL command */
)
{
    struct ias_db_query *query_handle = NULL;
    SQLHSTMT statement;
    SQLRETURN ret;

    query_handle = (struct ias_db_query*)malloc(sizeof(*query_handle));
    if (!query_handle)
    {
        strcpy(db->last_connection_error,
               "Error allocating memory for query structure");
        return NULL;
    }

    /* allocate a query statement handle */
    ret = SQLAllocHandle(SQL_HANDLE_STMT, db->database, &statement);
    if (!SQL_SUCCEEDED(ret))
    {
        int len;

        strcpy(db->last_connection_error, 
                   "Error allocating statement handle: ");
        len = strlen(db->last_connection_error);
        get_error(db->database, SQL_HANDLE_DBC, &db->last_connection_error[len],
                  sizeof(db->last_connection_error) - len);
        free(query_handle);
        return NULL;
    }

    /* set up the query correctly, so save the info and return */
    query_handle->query = statement;
    query_handle->rows_to_insert = 1;

    /* execute the statement */
    query_handle->query_ret
            = SQLExecDirect(statement, (SQLCHAR *)sql_command, SQL_NTS);

    if (SQL_SUCCEEDED(query_handle->query_ret))
        query_handle->is_active = 1;
    else
        query_handle->is_active = 0;

    return query_handle;
}

/****************************************************************************
* Name: ias_db_query_was_successful
*
* Description: This is a routine to determine whether a query completed
*   successfully.  To get the text message associated with an error, use the
*   ias_db_query_get_error_message routine.
*
* Returns: 1 if query was successful, 0 if an error occurred
*****************************************************************************/
int ias_db_query_was_successful
(
    struct ias_db_query* query_handle /* I: query to check for success */
)
{
    if (!query_handle)
        return 0;

    if (SQL_SUCCEEDED(query_handle->query_ret)
        || (query_handle->query_ret == SQL_NO_DATA))
    {
        return 1;
    }
    else
        return 0;
}

/****************************************************************************
* Name: ias_db_query_get_error_message
*
* Description: provides access to the error message for a query.  Use
*   ias_db_query_was_successful to determine whether an error occurred before
*   calling this routine.
*
* Returns: The error message in the msg buffer passed in
*****************************************************************************/
void ias_db_query_get_error_message
(
    struct ias_db_query* query_handle,  /* I: query to get error message for */
    char *msg,                          /* I/O: pointer to the buffer for the 
                                                error message */
    int msg_size                        /* I: size of msg buffer */
)
{
    if (!query_handle)
        snprintf(msg, msg_size, "%s", "Query failed for unknown reason");
    else
        get_error(query_handle->query, SQL_HANDLE_STMT, msg, msg_size);
}

/****************************************************************************
* Name: ias_db_query_get_modified_rows
*
* Description: Returns the number of rows affected by an update, insert, or 
*  delete query.  Do not rely on this count for the number of rows returned by 
*  a select statement, use ias_db_query_next() instead.
*
* Returns: The number of rows affected, -1 on error
*****************************************************************************/
int ias_db_query_get_modified_rows
(
    struct ias_db_query* query_handle   /* I: query to get error message for */
)
{
    int row_count = -1;
    SQLRETURN ret;
    SQLLEN sql_row_count;

    if (query_handle)
    {
        ret = SQLRowCount(query_handle->query, &sql_row_count);
        if (SQL_SUCCEEDED(ret))
            row_count = sql_row_count;
    }

    return row_count;
}

/****************************************************************************
* Name: ias_db_query_is_active
*
* Description: returns whether a query is currently "active".  This is
* typically used to determine whether a select query has results that can be
* obtained.
*
* Returns: 1 if the query is active, 0 if it is not active
*****************************************************************************/
int ias_db_query_is_active
(
    struct ias_db_query* query_handle   /* I: query to check for being active */
)
{
    return query_handle->is_active;
}

/****************************************************************************
* Name: ias_db_query_next
*
* Description: steps to the next row of the results returned by a select query.
* This must be called before the first row is available.
*
* Returns: 0 if a record could not be retrieved, 1 if a record was retrieved
*****************************************************************************/
int ias_db_query_next
(
    struct ias_db_query* query_handle /* I: query to advance to next record */
)
{
    SQLRETURN ret;

    ret = SQLFetch(query_handle->query);
    if (SQL_SUCCEEDED(ret))
        return 1;
    else
    {
        query_handle->is_active = 0;
        return 0;
    }
}

/****************************************************************************
* Name: ias_db_query_is_null
*
* Description: Used to determine whether a select query result for a given
*   index is NULL.
*
* Returns: 1 if the result returned for the particular index is NULL.
*   Otherwise it returns 0.
*****************************************************************************/
int ias_db_query_is_null
(
    struct ias_db_query* query_handle, /* I: query to check */
    int index                          /* I: index in query to check for null */
)
{
    SQLRETURN ret;
    SQLLEN indicator;
    char buf[256];

    ret = SQLGetData(query_handle->query, index + 1, SQL_C_CHAR, buf,
                     sizeof(buf), &indicator);
    if (SQL_SUCCEEDED(ret))
    {
        if (indicator == SQL_NULL_DATA)
            return 1;
    }

    return 0;
}

/****************************************************************************
* Name: ias_db_query_string_value
*
* Description: returns the result of query as a string for a particular index
*   in the select statement.  The index parameter is zero based.  For example
*   "select name, address from table;" will return the value for the "name"
*   field when index is 0 and the "address" field when index is 1.
*
* Returns: the index'th field (zero-based) from the current query row in the
*   buffer
*****************************************************************************/
void ias_db_query_string_value
(
    struct ias_db_query* query_handle, /* I: query to extract string from */
    int index,              /* I: index in query to extract results from */
    char *buffer,           /* O: pointer to buffer for returning the results */
    int buffer_size         /* I: size of buffer, in bytes */
)
{
    SQLRETURN ret;
    SQLLEN indicator;

    ret = SQLGetData(query_handle->query, index + 1, SQL_C_CHAR, buffer,
                     buffer_size, &indicator);
    if (SQL_SUCCEEDED(ret))
    {
        if (indicator == SQL_NULL_DATA)
            snprintf(buffer,buffer_size,"NULL");
    }
    else
        strcpy(buffer,"");
}

/****************************************************************************
* Name: ias_db_query_datetime_value
*
* Description: returns the result of query as a date/time string in a
*   YYYY-MM-DDTHH:MM:SS format for a particular index in the select statement.
*   The index parameter is zero based.  For example "select name, address from
*   table;" will return the value for the "name" field when index is 0 and the
*   "address" field when index is 1.
*
* Returns: the index'th field (zero-based) from the current query row in the
*   buffer
*****************************************************************************/
void ias_db_query_datetime_value
(
    struct ias_db_query* query_handle, /* I: query to extract string from */
    int index,              /* I: index in query to extract results from */
    char *buffer,           /* O: pointer to buffer for returning the results */
    int buffer_size         /* I: size of buffer, in bytes */
)
{
    SQLRETURN ret;
    SQLLEN indicator;
    SQL_TIMESTAMP_STRUCT timestamp;

    ret = SQLGetData(query_handle->query, index + 1, SQL_C_TYPE_TIMESTAMP,
                     &timestamp, 0, &indicator);
    if (!SQL_SUCCEEDED(ret))
    {
        snprintf(buffer, buffer_size, "NULL");
    }
    else
    {
        if (indicator == SQL_NULL_DATA)
            snprintf(buffer, buffer_size, "NULL");
        else
        {
            if (timestamp.fraction == 0)
            {
                snprintf(buffer, buffer_size, "%04d-%02d-%02dT%02d:%02d:%02d",
                         timestamp.year, timestamp.month, timestamp.day,
                         timestamp.hour, timestamp.minute, timestamp.second);
            }
            else
            {
                /* support fractional seconds */
                double seconds = timestamp.second 
                               + (double)timestamp.fraction / 1000000000.0;
                snprintf(buffer, buffer_size,
                         "%04d-%02d-%02dT%02d:%02d:%09.6f",
                         timestamp.year, timestamp.month, timestamp.day,
                         timestamp.hour, timestamp.minute, seconds);
            }
        }
    }
}

/****************************************************************************
* Name: ias_db_query_int_value
*
* Description: returns the result of query as an integer.  The index parameter
*   is zero based.  For example "select name, address from table;" will return
*   the value for the "name" field when index is 0 and the "address" field when
*   index is 1.
*
* Returns: the index'th field (zero-based) from the current query row in the
*   buffer
*
* Note: If the routine is called twice without advancing the row, the return 
*       is SQL_NO_DATA.
*****************************************************************************/
int ias_db_query_int_value
(
    struct ias_db_query* query_handle, /* I: query to extract integer from */
    int *int_value,                    /* O: returned integer query value  */
    int index                          /* I: in query to extract results from*/
)
{
    SQLRETURN ret;
    SQLLEN indicator;

    ret = SQLGetData(query_handle->query, index + 1, SQL_C_SLONG, int_value, 0,
                     &indicator);
    if (!SQL_SUCCEEDED(ret))
    {
        return ERROR;
    }

    return SUCCESS;
}

/****************************************************************************
* Name: ias_db_query_double_value
*
* Description: returns the result of query as a double.  The index parameter is
*   zero based.  For example "select name, address from table;" will return the
*   value for the "name" field when index is 0 and the "address" field when
*   index is 1.
*
* Returns: the index'th field (zero-based) from the current query row in the
*   buffer
*
*****************************************************************************/
int ias_db_query_double_value
(
    struct ias_db_query* query_handle, /* I: query to extract double from */
    double *double_value,              /* O: returned integer query value  */
    int index                          /* I: index in query to extract results 
                                             from */
)
{
    SQLRETURN ret;      /* ODBC call return value */
    SQLLEN indicator;

    ret = SQLGetData(query_handle->query, index + 1, SQL_C_DOUBLE, 
                     double_value, 0, &indicator);
    if (!SQL_SUCCEEDED(ret))
    {
        return ERROR;
    }
    return SUCCESS;

}

/****************************************************************************
* Name: ias_db_query_close
*
* Description: closes a query and frees any resources allocated to the query.
*   It must be called for every query opened.  A query is no longer valid after
*   passed to this routine and it should not be accessed again.
*****************************************************************************/
void ias_db_query_close
(
    struct ias_db_query* query_handle /* I: query to close */
)
{
    if (query_handle)
    {
        SQLRETURN ret;

        ret = SQLFreeHandle(SQL_HANDLE_STMT, query_handle->query);

        free(query_handle);
    }
}

/****************************************************************************
* Name: ias_db_query_end
*
* Description: finishes a query and discards all pending results.
*   The query can be re-executed after calling this.  This is useful
*   for re-using a prepared query statement with a different value
*   in a bound variable.
*****************************************************************************/
int ias_db_query_end
(
    struct ias_db_query* query_handle /* I: query to finish */
)
{
    if (query_handle)
    {
        SQLRETURN ret;
        ret = SQLFreeStmt(query_handle->query, SQL_CLOSE);
        if (!SQL_SUCCEEDED(ret))
        {
            return ERROR;
        }
    }
    return SUCCESS;
}

/****************************************************************************
* Name: ias_db_prepare_query
*
* Description: prepares the query in sql_command.  The sql_command using this
*   should have bind variables for the values that will be bound by the
*   ias_db_bind... functions.  For example:
*             insert into A_TABLE ( col1, col2, col3 ) 
*             values (?, ?, ?)
*
* Notes: 
*   - The ias_db_bind functions have been tested for executing insert
*     statements and stored procs.
*   - the sql_command is simply a string holding the SQL command to perform.
*     The sql_command should include "?" to indicate where values need to be
*     bound (refer to above example)
*   - Every effort should be made to ensure the SQL statements do not use
*     proprietary SQL from a particular database vendor.
*   - every ias_db_prepare_query call should be matched with a 
*     ias_db_query_close call to release the resources used by the query.
*
* Returns: pointer to a ias_db_query structure if successful, NULL if a 
*   failure happens.  Note that even if non-NULL is returned,
*   ias_db_query_was_successful needs to be called to verify the query really
*   worked.
*****************************************************************************/
struct ias_db_query* ias_db_prepare_query
(
    struct ias_db_connection* db,   /* I: database connection for query */
    const char *sql_command         /* I: SQL command */
)
{
    struct ias_db_query *query_handle = NULL;
    SQLHSTMT statement;
    SQLRETURN ret;

    query_handle = (struct ias_db_query*)malloc(sizeof(*query_handle));
    if (!query_handle)
    {
        strcpy(db->last_connection_error,
               "Error allocating memory for query structure");
        return NULL;
    }

    /* allocate a query statement handle */
    ret = SQLAllocHandle(SQL_HANDLE_STMT, db->database, &statement);
    if (!SQL_SUCCEEDED(ret))
    {
        int len;

        strcpy(db->last_connection_error, 
                   "Error allocating statement handle: ");
        len = strlen(db->last_connection_error);
        get_error(db->database, SQL_HANDLE_DBC, &db->last_connection_error[len],
                  sizeof(db->last_connection_error) - len);
        free(query_handle);
        return NULL;
    }

    /* set up the query correctly, so save the info and return */
    query_handle->query = statement;

    /* prepare the statement */
    query_handle->query_ret
            = SQLPrepare(statement, (SQLCHAR *)sql_command, SQL_NTS);
    query_handle->rows_to_insert = 1;

    if (SQL_SUCCEEDED(query_handle->query_ret))
        query_handle->is_active = 1;
    else
        query_handle->is_active = 0;

    return query_handle;
}

/****************************************************************************
* Name: ias_db_set_rows_to_insert
*
* Description: sets the number of rows to insert with a single execute call.
*
* NOTE: it's not generally a good idea to change the number of rows to insert
* on a prepared query after it has been set once and variables bound
*
* Returns: SUCCESS or ERROR.
*****************************************************************************/
int ias_db_set_rows_to_insert
(
    struct ias_db_query* query_handle, /* I: query to set rows for */
    int rows                           /* I: rows to insert at once */
)
{
    SQLRETURN ret;
    /* convert the rows to an SQLPOINTER since that can be different sizes
       when compiling 32-bit or 64-bit builds */
    SQLPOINTER row_count = (SQLPOINTER)(long int)rows;

    ret = SQLSetStmtAttrW(query_handle->query, SQL_ATTR_PARAM_BIND_TYPE,
                         SQL_BIND_BY_COLUMN, 0);
    if (!SQL_SUCCEEDED(ret))
    {
        return ERROR;
    }

    ret = SQLSetStmtAttrW(query_handle->query, SQL_ATTR_PARAMSET_SIZE,
                         row_count, 0);
    if (SQL_SUCCEEDED(ret))
    {
        query_handle->rows_to_insert = rows;
        return SUCCESS;
    }
    else
        return ERROR;
}

/****************************************************************************
* Name: ias_db_bind_char_by_index
*
* Description: Bind a character array to an index (zero based) in a prepared
*   query.  The query must have been prepared using the ias_db_prepare_query
*   function.
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_db_bind_char_by_index
(
    struct ias_db_query *query_handle, /* I: prepared query */
    int index,                         /* I: The index of the bind variable in 
                                             the SQL statement */
    IAS_DB_PARAMETER_MODE_TYPE parameter_mode, /* I: The input-output type of 
                                                     bind variable */
    char *array,                       /* I: The value to bind to the bind 
                                             variable */
    int buf_len,                       /* I: length of each entry in the array*/
    IAS_DB_NULL_TYPE *null_val_flag    /* I: Flags indicating if each value is 
                                             null */
)
{
    SQLRETURN ret;
    SQLSMALLINT input_output_type 
        = ias_db_get_input_output_type(parameter_mode);

    ret = SQLBindParameter(query_handle->query, index+1, input_output_type,
                  SQL_C_CHAR, SQL_VARCHAR, buf_len, 0, array, buf_len, 
                  (SQLLEN *)null_val_flag);
    if (SQL_SUCCEEDED(ret))
        return SUCCESS;
    else
        return ERROR;
}

/****************************************************************************
* Name: ias_db_bind_float_by_index
*
* Description: Bind a float array to an index (zero based) in a prepared
*   query.  The query must have been prepared using the ias_db_prepare_query
*   function.
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_db_bind_float_by_index
(
    struct ias_db_query *query_handle, /* I: query to bind value for */
    int index,                         /* I: The index of the bind variable in 
                                             the SQL statement */
    IAS_DB_PARAMETER_MODE_TYPE parameter_mode, /* I: The input-output type of 
                                                     bind variable */
    float *val,                        /* I: The array of floats to bind */
    IAS_DB_NULL_TYPE *null_val_flag    /* I: Flags indicating if the values are 
                                             null */
)
{
    SQLRETURN ret;
    SQLSMALLINT input_output_type 
        = ias_db_get_input_output_type(parameter_mode);

    ret = SQLBindParameter(query_handle->query, index+1, input_output_type,
                  SQL_C_FLOAT, SQL_FLOAT, 15, 0, val, 0, 
                  (SQLLEN *)null_val_flag);
    if (SQL_SUCCEEDED(ret))
        return SUCCESS;
    else
        return ERROR;
}

/****************************************************************************
* Name: ias_db_bind_double_by_index
*
* Description: Bind a double array to an index (zero based) in a prepared
*   query.  The query must have been prepared using the ias_db_prepare_query
*   function.
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_db_bind_double_by_index
(
    struct ias_db_query *query_handle,  /* I: query to bind value for */
    int index,                          /* I: The index of the bind variable 
                                              in the SQL statement */
    IAS_DB_PARAMETER_MODE_TYPE parameter_mode, /* I: The input-output type of 
                                                     bind variable */
    double *val,                        /* I: The array of doubles to bind */
    IAS_DB_NULL_TYPE *null_val_flag     /* I: Flags indicating if the values 
                                              are null */
)
{
    SQLRETURN ret;
    SQLSMALLINT input_output_type 
        = ias_db_get_input_output_type(parameter_mode);

    ret = SQLBindParameter(query_handle->query, index+1, input_output_type,
                  SQL_C_DOUBLE, SQL_DOUBLE, 15, 0, val, 0, 
                  (SQLLEN *)null_val_flag);
    if (SQL_SUCCEEDED(ret))
        return SUCCESS;
    else
        return ERROR;
}

/****************************************************************************
* Name: ias_db_bind_int_by_index
*
* Description: Bind a int array to an index (zero based) in a prepared
*   query.  The query must have been prepared using the ias_db_prepare_query
*   function.
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_db_bind_int_by_index
(
    struct ias_db_query *query_handle,  /* I: query to bind value for */
    int index,                          /* I: The index of the bind variable 
                                              in the SQL statement */
    IAS_DB_PARAMETER_MODE_TYPE parameter_mode, /* I: The input-output type of 
                                                     bind variable */
    int *val,                           /* I: The value to bind to the bind 
                                              variable */
    IAS_DB_NULL_TYPE *null_val_flag     /* I: Flag indicating if the value is 
                                              null */
)
{
    SQLRETURN ret;
    SQLSMALLINT input_output_type 
        = ias_db_get_input_output_type(parameter_mode);

    ret = SQLBindParameter(query_handle->query, index+1, input_output_type,
                  SQL_C_SLONG, SQL_INTEGER, 0, 0, val, 0, 
                  (SQLLEN *)null_val_flag);
    if (SQL_SUCCEEDED(ret))
        return SUCCESS;
    else
        return ERROR;
}

/****************************************************************************
* Name: ias_db_bind_short_int_by_index
*
* Description: Bind a short int array to an index (zero based) in a prepared
*   query.  The query must have been prepared using the ias_db_prepare_query
*   function.
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_db_bind_short_int_by_index
(
    struct ias_db_query *query_handle,  /* I: query to bind value for */
    int index,                          /* I: The index of the bind variable 
                                              in the SQL statement */
    IAS_DB_PARAMETER_MODE_TYPE parameter_mode, /* I: The input-output type of 
                                                     bind variable */
    short int *val,                     /* I: The value to bind to the bind 
                                              variable */
    IAS_DB_NULL_TYPE *null_val_flag     /* I: Flag indicating if the value is 
                                              null */
)
{
    SQLRETURN ret;
    SQLSMALLINT input_output_type 
        = ias_db_get_input_output_type(parameter_mode);

    ret = SQLBindParameter(query_handle->query, index+1, input_output_type,
                  SQL_C_SSHORT, SQL_SMALLINT, 0, 0, val, 0, 
                  (SQLLEN *)null_val_flag);
    if (SQL_SUCCEEDED(ret))
        return SUCCESS;
    else
        return ERROR;
}

/****************************************************************************
* Name: ias_db_bind_date_by_index
*
* Description: Bind a date array to an index (zero based) in a prepared
*   query.  The query must have been prepared using the ias_db_prepare_query
*   function.
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_db_bind_date_by_index
(
    struct ias_db_query *query_handle,  /* I: query to bind value for */
    int index,                          /* I: The index of the bind variable in
                                              the SQL statement */
    IAS_DB_PARAMETER_MODE_TYPE parameter_mode, /* I: The input-output type of 
                                                     bind variable */
    SQL_DATE_STRUCT *date,              /* I: array of dates to bind */
    IAS_DB_NULL_TYPE *null_val_flag     /* I: Flags indicating if the values 
                                              are null */
)
{
    SQLRETURN ret;
    SQLSMALLINT input_output_type 
        = ias_db_get_input_output_type(parameter_mode);

    ret = SQLBindParameter(query_handle->query, index+1, input_output_type,
                  SQL_C_TYPE_DATE, SQL_TYPE_DATE, 0, 0, date, 0, 
                  (SQLLEN *)null_val_flag);
    if (SQL_SUCCEEDED(ret))
        return SUCCESS;
    else
        return ERROR;
}

/****************************************************************************
* Name: ias_db_bind_timestamp_by_index
*
* Description: Bind a timestamp array to an index (zero based) in a prepared
*   query.  The query must have been prepared using the ias_db_prepare_query
*   function.
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_db_bind_timestamp_by_index
(
    struct ias_db_query *query_handle,  /* I: query to bind value for */
    int index,                          /* I: The index of the bind variable 
                                              in the SQL statement */
    IAS_DB_PARAMETER_MODE_TYPE parameter_mode, /* I: The input-output type of 
                                                     bind variable */
    SQL_TIMESTAMP_STRUCT *timestamps,   /* I: array of timestamps to bind */
    IAS_DB_NULL_TYPE *null_val_flag     /* I: Flags indicating if the values 
                                              are null */
)
{
    SQLRETURN ret;
    SQLSMALLINT input_output_type 
        = ias_db_get_input_output_type(parameter_mode);

    ret = SQLBindParameter(query_handle->query, index+1, input_output_type,
                  SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, 0, 0, timestamps,
                  0, (SQLLEN *)null_val_flag);
    
    if (SQL_SUCCEEDED(ret))
        return SUCCESS;
    else
        return ERROR;
}

/****************************************************************************
* Name: ias_db_exec_prepared_query
*
* Description: executes the query that is passed in.  The query must have been
*   created and prepared using ias_db_prepare_query function, and any necessary
*   bind variables in the query must have been set using ias_db_bind...
*   functions 
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_db_exec_prepared_query
(
    struct ias_db_query *query_handle /* I: query to execute */
)
{
    SQLRETURN ret;

    ret = SQLExecute(query_handle->query);
    if (SQL_SUCCEEDED(ret))
        return SUCCESS;
    else
        return ERROR;
}

/****************************************************************************
*
*  Routines internal to this library
*
*****************************************************************************/

/****************************************************************************
* Name: ias_db_parse_yyyy_mm_dd_time
*
* Description: parses a text string containing date or date and time into
*   a SQL timestamp structure.
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_db_parse_yyyy_mm_dd_time
(
    const char *in_date,           /* I: input date in yyyy-mm-dd format with an
                                         optional "Thh:mm:ss[.sssss]" */
    SQL_TIMESTAMP_STRUCT *out_date /* O: pointer to SQL timestamp structure */
)
{
    int i;
    int status;
    int int_value;
    char temp[5];
    double fractional_seconds;
    int len = strlen(in_date);
    int valid = 1;

    /* verify the input date is in the correct format */
    if ((len != 10) && (len != 19) && (len != 25))
        valid = 0;
    else if ((in_date[4] != '-') || (in_date[7] != '-')
             || ((len == 19) && ((in_date[10] != 'T') || (in_date[13] != ':')
                    || (in_date[16] != ':'))))
    {
        valid = 0;
    }
    if (!valid)
    {
        IAS_LOG_ERROR("Invalid date: %s", in_date);
        return ERROR;
    }

    for (i = 0; i < len; i++)
    {
        if ((i ==  4) || (i == 7) ||             /* date separators     */
            (i == 10) ||                         /* date/time separator */
            (i == 13) || (i == 16) || (i == 19)) /* time separators     */
            continue;
        if (!isdigit(in_date[i]))
        {
            IAS_LOG_ERROR("Invalid date: %s", in_date);
            return ERROR;
        }
    }

    /* convert the year to an integer */
    strncpy(temp, in_date, 4);
    temp[4] = '\0';
    status = sscanf(temp, "%d", &int_value);
    if (status < 1)
    {
        IAS_LOG_ERROR("Unable to parse year from %s", temp);
        return ERROR;
    }
    else
        out_date->year = int_value;

    /* convert the month to an integer */
    strncpy(temp, &in_date[5], 2);
    temp[2] = '\0';
    status = sscanf(temp, "%d", &int_value);
    if (status < 1)
    {
        IAS_LOG_ERROR("Unable to parse month from %s", temp);
        return ERROR;
    }
    else
       out_date->month = int_value;

    /* convert the day to an integer */
    strncpy(temp, &in_date[8], 2);
    temp[2] = '\0';
    status = sscanf(temp, "%d", &int_value);
    if (status < 1)
    {
        IAS_LOG_ERROR("Unable to parse day from %s", temp);
        return ERROR;
    }
    else
       out_date->day = int_value;

    /* default the time to midnight */
    out_date->hour = 0;
    out_date->minute = 0;
    out_date->second = 0;
    out_date->fraction = 0;

    /* if the optional time is included, parse that as well */
    if (len >= 19)
    {
        /* get the hours */
        strncpy(temp, &in_date[11], 2);
        temp[2] = '\0';
        status = sscanf(temp, "%d", &int_value);
        if (status < 1)
        {
            IAS_LOG_ERROR("Unable to parse hour from %s", temp);
            return ERROR;
        }
        else
            out_date->hour = int_value;

        /* get the minutes */
        strncpy(temp, &in_date[14], 2);
        temp[2] = '\0';
        status = sscanf(temp, "%d", &int_value);
        if (status < 1)
        {
            IAS_LOG_ERROR("Unable to parse minutes from %s", temp);
            return ERROR;
        }
        else
            out_date->minute = int_value;

        /* get the seconds */
        strncpy(temp, &in_date[17], 2);
        temp[2] = '\0';
        status = sscanf(temp, "%d", &int_value);
        if (status < 1)
        {
            IAS_LOG_ERROR("Unable to parse seconds from %s", temp);
            return ERROR;
        }
        else
            out_date->second = int_value;

        /* get fractional seconds, if available */
        if (len > 19)
        {
            fractional_seconds = strtod(&in_date[19], NULL);
            out_date->fraction = fractional_seconds * 1000000000.0;
        }
    }

    return SUCCESS;
}

/****************************************************************************
* Name: ias_db_parse_yyyy_ddd
*
* Description: parses a text string containing date (year and day of year)
*   into year, month, and day
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_db_parse_yyyy_ddd
(
    const char *in_date,      /* I: input date in yyyy-ddd or yyyy:ddd format */
    int *year,                /* O: year */
    int *month,               /* O: month */
    int *day                  /* O: day */
)
{
    int i;
    int int_value;
    int status;
    int doy;
    char temp[5];

    /* verify the input data is in the correct format */
    if ((strlen(in_date) != 8) || ((in_date[4] != '-') && (in_date[4] != ':')))
    {
        IAS_LOG_ERROR("Invalid date: %s", in_date);
        return ERROR;
    }

    for (i = 0; i < 8; i++)
    {
        if (i == 4)
            continue;
        if (!isdigit(in_date[i]))
        {
            IAS_LOG_ERROR("Invalid date: %s", in_date);
            return ERROR;
        }
    }

    /* convert the year to an integer */
    strncpy(temp, in_date, 4);
    temp[4] = '\0';
    status = sscanf(temp, "%d", &int_value);
    if (status < 1)
    {
        IAS_LOG_ERROR("Unable to parse year from %s", temp);
        return ERROR;
    }
    else
        *year = int_value;

    /* convert the day of year to an integer */
    strncpy(temp, &in_date[5], 3);
    temp[3] = '\0';
    status = sscanf(temp, "%d", &int_value);
    if (status < 1)
    {
        IAS_LOG_ERROR("Unable to parse day of year from %s", temp);
        return ERROR;
    }
    else
        doy = int_value;

    /* convert the year and doy into month and day */
    ias_math_convert_doy_to_month_day(doy, *year, month, day);

    return SUCCESS;
}

/****************************************************************************
* Name: ias_db_parse_yyyy_ddd_hms
*
* Description: parses a text string containing date (year and day of year)
*   and time into a set of output date/time variables
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
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
)
{
    char date[9];
    char time[100];
    char temp[3];
    int len;
    int i;
    int int_value;
    int status;
    double dbl_value;

    strncpy(date, in_date, 8);
    date[8] = '\0';
    if (ias_db_parse_yyyy_ddd(date, year, month, day) != SUCCESS)
        return ERROR;

    /* verify the date/time divider is either a 'T' or a space or : */
    if ((in_date[8] != 'T') && (in_date[8] != ' ') && (in_date[8] != ':'))
    {
        IAS_LOG_ERROR("Invalid date: %s", in_date);
        return ERROR;
    }

    /* copy the time out of the input date/time */
    strncpy(time, &in_date[9], 100);
    time[99] = '\0';

    len = strlen(time);

    /* verify the ":" appear as expected */
    if ((len < 8) || (time[2] != ':') || (time[5] != ':'))
    {
        IAS_LOG_ERROR("Invalid time: %s", in_date);
        return ERROR;
    }

    /* verify the digits in the time */
    for (i = 0; i < len; i++)
    {
        if ((i == 2) || (i == 5) || (i == 8))
            continue;
        if (!isdigit(time[i]))
        {
            IAS_LOG_ERROR("Invalid time: %s", in_date);
            return ERROR;
        }
    }

    /* get the hours */
    strncpy(temp, time, 2);
    temp[2] = '\0';
    status = sscanf(temp, "%d", &int_value);
    if (status < 1)
    {
        IAS_LOG_ERROR("Unable to parse hours from %s", temp);
        return ERROR;
    }
    else
        *hours = int_value;

    /* get the minutes */
    strncpy(temp, &time[3], 2);
    temp[2] = '\0';
    status = sscanf(temp, "%d", &int_value);
    if (status < 1)
    {
        IAS_LOG_ERROR("Unable to parse minutes from %s", temp);
        return ERROR;
    }
    else
        *min = int_value;

    /* get the seconds */
    status = sscanf(&time[6], "%lf", &dbl_value);
    if (status < 1)
    {
        IAS_LOG_ERROR("Unable to parse seconds from %s", &time[6]);
        return ERROR;
    }
    else
        *seconds = dbl_value;

    return SUCCESS;
}

/****************************************************************************
* Name: ias_db_parse_yyyy_ddd_time
*
* Description: parses a text string containing date (year and day of year)
*   and time into year, month, day, and seconds of day
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_db_parse_yyyy_ddd_time
(
    const char *in_date,      /* I: input date in
                                    yyyy[-|:]ddd[T| |:]SSSSS.sssssssss format */
    int *year,                /* O: year */
    int *month,               /* O: month */
    int *day,                 /* O: day */
    double *seconds           /* O: seconds, including fractional seconds */
)
{
    char date[9];
    char time[100];
    int found_decimal = 0;
    int len;
    int i;

    strncpy(date, in_date, 8);
    date[8] = '\0';
    if (ias_db_parse_yyyy_ddd(date, year, month, day) != SUCCESS)
        return ERROR;

    /* verify the date/time divider is either a 'T' or a space */
    if ((in_date[8] != 'T') && (in_date[8] != ' ') && (in_date[8] != ':'))
    {
        IAS_LOG_ERROR("Invalid date: %s", in_date);
        return ERROR;
    }

    /* copy the time out of the input date/time */
    strncpy(time, &in_date[9], 100);
    time[99] = '\0';

    len = strlen(time);

    /* verify the digits in the time */
    for (i = 0; i < len; i++)
    {
        if (!found_decimal && (time[i] == '.'))
        {
            found_decimal = 1;
            continue;
        }
        if (!isdigit(time[i]))
        {
            IAS_LOG_ERROR("Invalid time: %s", in_date);
            return ERROR;
        }
    }

    /* get the seconds */
    *seconds = atof(time);

    return SUCCESS;
}


/****************************************************************************
* Name: ias_db_get_input_output_type 
*
* Description: helper routine to get the ODBC parameter mode for a given 
*     IAS parameter mode
*
* Returns: SQLSMALLINT ODBC parameter mode
*****************************************************************************/
SQLSMALLINT ias_db_get_input_output_type (
    IAS_DB_PARAMETER_MODE_TYPE parameter_mode /* I: input/output/return mode */
) 
{
    SQLSMALLINT input_output_type = SQL_PARAM_INPUT;
    switch (parameter_mode)
    {
        case IAS_DB_PARAMETER_MODE_INPUT:
            input_output_type = SQL_PARAM_INPUT;
            break;
        case IAS_DB_PARAMETER_MODE_OUTPUT:
        case IAS_DB_PARAMETER_MODE_RETURN:
            input_output_type = SQL_PARAM_OUTPUT;
            break;
        case IAS_DB_PARAMETER_MODE_INPUTOUTPUT:
            input_output_type = SQL_PARAM_INPUT_OUTPUT;
            break;
    }
    return input_output_type;
}
