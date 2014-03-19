#ifndef IAS_DB_GET_CONNECT_INFO_H
#define IAS_DB_GET_CONNECT_INFO_H

/****************************************************************************
Note: The callers of these routines are responsible for freeing the returned
memory.
*****************************************************************************/
int ias_db_get_connect_info
(
    char **database_name,  /* I/O: Database name to connect to */
    char **user_name,      /* I/O: user name to use for the connection */
    char **password,       /* I/O: user's password for the database */
    char **host            /* I/O: host name where the database is located */
);

int ias_db_get_login_for_satellite_sensor
(
    const char *satellite,      /* I: Satellite of the schema */
    const char *sensor,         /* I: Sensor of the schema */
    char **schema,              /* O: Schema (user) name */
    char **password             /* O: Password for the schema */
);

#endif  

