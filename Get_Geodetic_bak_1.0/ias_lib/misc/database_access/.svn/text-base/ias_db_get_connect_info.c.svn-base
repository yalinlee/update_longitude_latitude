/*******************************************************************************
NAME: ias_db_get_connect_info

PURPOSE: Gets the database connection information needed to connect to the
    database.

NOTES: Need to pass in NULL pointers for the arguments.  The calling routine
    needs to free the memory returned.

*******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_db_get_connect_info.h"

/* ias_db_get_connect_info populates the struct passed in with the connection
 * information for the L8 database.
 * Currently, this method will not set the host to anything since this is
 * not needed with the oracle connection made with the current ias_db
 * library.
 *
 * Returns: SUCCESS on success, or ERROR on failure
 *
 * Notes: The four arguments passed in must be NULL pointers.  This method
 *        will allocate space for each char pointer passed in by using the
 *        strdup method.  Therefore the invoker is responsible for freeing
 *        the memory for each pointer when they are finished using the values
 *        in the variables.
 */
int ias_db_get_connect_info
(
    char **database_name,  /* I/O: Database name to connect to */
    char **user_name,      /* I/O: user name to use for the connection */
    char **password,       /* I/O: user's password for the database */
    char **host            /* I/O: host name where the database is located */
)
{
    const char *env_var = NULL;

    /* The strdup on the getenv is because the getenv method returns a pointer
       to the value as it exists in the environment variable table.  By doing
       the strdup, newly allocated memory is created and the invoker is 
       responsible for cleaning it up. */
    env_var = getenv("TWO_TASK");
    if (!env_var) 
    {
        IAS_LOG_ERROR("TWO_TASK environment variable not set");
        return ERROR;
    }

    *database_name = strdup(env_var);
    if (!*database_name)
    {
        IAS_LOG_ERROR("Allocating memory for database name");
        return ERROR;
    }

    /* TODO - retrieve the satellite and sensor from the satellite attributes
       library (per related bug tracker) */
    if (ias_db_get_login_for_satellite_sensor("L8", "OLITIRS", user_name,
        password) != SUCCESS)
    {
        free(*database_name);
        *database_name = NULL;
        IAS_LOG_ERROR("Unable to retrieve database username/password");
        return ERROR;
    }

    *host = NULL;

    return SUCCESS;
}

/****************************************************************************
Name: ias_db_get_login_for_satellite_sensor

Purpose: Given a satellite name and sensor (i.e. L8 and OLITIRS), extracts
    the schema name and password from the associated environment variable
    named IAS_DB_<satellite>_<sensor>.

Returns:
    SUCCESS or ERROR

Notes:
    - The caller of the routine is responsible for freeing the memory that
      this routine allocates for the returned schema and password.

****************************************************************************/
int ias_db_get_login_for_satellite_sensor
(
    const char *satellite,      /* I: Satellite of the schema */
    const char *sensor,         /* I: Sensor of the schema */
    char **schema,              /* O: Schema (user) name */
    char **password             /* O: Password for the schema */
)
{
    char env_var_name[200];
    const char *env_var = NULL;
    const char *temp_ptr;
    int length;
    int status;

    /* Create the name of the environment variable that stores the login
       info */
    status = snprintf(env_var_name, sizeof(env_var_name), "IAS_DB_%s_%s",
        satellite, sensor);
    if (status <= 0 || status >= sizeof(env_var_name))
    {
        IAS_LOG_ERROR("Environment variable name buffer is too small");
        return ERROR;
    }

    /* get the schema(username)/password for the database */
    env_var = getenv(env_var_name);
    if (!env_var)
    {
        IAS_LOG_ERROR("%s environment variable not set", env_var_name);
        return ERROR;
    }

    /* split the schema(username) and password up */
    temp_ptr = strchr(env_var, '/');
    if (!temp_ptr)
    {
        IAS_LOG_ERROR("Parsing schema name and password from environment "
            "variable %s", env_var_name);
        return ERROR;
    }
    length = temp_ptr - env_var + 1;

    *schema = malloc(length * sizeof(char));
    if (!*schema)
    {
        IAS_LOG_ERROR("Allocating memory for schema name");
        return ERROR;
    }
    strncpy(*schema, env_var, length);
    (*schema)[length - 1] = '\0';

    *password = strdup(temp_ptr + 1);
    if (!*password)
    {
        free(*schema);
        *schema = NULL;
        IAS_LOG_ERROR("Allocating memory for database password");
        return ERROR;
    }

    return SUCCESS;
}
