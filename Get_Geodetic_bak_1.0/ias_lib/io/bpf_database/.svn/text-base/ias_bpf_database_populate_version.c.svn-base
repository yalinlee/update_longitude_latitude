#include <stdlib.h>
#include <string.h>
#include "ias_bpf.h"
#include "ias_bpf_database.h"
#include "ias_db.h"
#include "ias_db_query.h"
#include "ias_db_insert.h"
#include "ias_db_stored_proc.h"
#include "ias_types.h"
#include "ias_const.h"
#include "ias_structures.h"
#include "ias_logging.h"
#include "ias_miscellaneous.h"

/****************************************************************************
* Name: ias_bpf_db_populate_version
*
* SELECT COUNT(*), MAX(VERSION_NUMBER)
* FROM BPF_MAIN
* WHERE EFFECTIVE_DATE_BEGIN <= to_date()
* AND EFFECTIVE_DATE_END >= to_date();
*
* Returns: SUCCESS or ERROR
*****************************************************************************/
int ias_bpf_db_populate_version(
    struct ias_db_connection *conn,     /* I: Open db connection */
    IAS_BPF *bpf)          /* I/O: populated BPF structure */
{
    IAS_SENSOR_ID sensor_id;
    const int max_clause = 1023;

    char where_clause[max_clause + 1];
    char db_sensor_id_string[IAS_BPF_SENSOR_NAME_LENGTH];
    int files_available = 0;
    int max_version = 0;
    int status = 0;
    int table_size = 0;

    const struct IAS_BPF_FILE_ATTRIBUTES *file_attributes = NULL; 
    IAS_DATETIME effective_date_begin;  /* begin date of this bpf */
    IAS_DATETIME effective_date_end;    /* end date of this bpf */

    IAS_DB_QUERY_TABLE table[] =
    {
        {"COUNT(*)", &files_available,
            sizeof(int), IAS_QUERY_INT, 1},
        {"MAX(VERSION_NUMBER)", &max_version, sizeof(int), 
            IAS_QUERY_INT, 1}
    };

    /* Make sure we have an open DB connection AND a populated BPF
       data structure. */
    if (conn == NULL)
    {
        IAS_LOG_ERROR("Input ias_db_connection structure is NULL");
        return ERROR;
    }

    if (bpf == NULL)
    {
        IAS_LOG_ERROR("Input BPF structure is NULL");
        return ERROR;
    }

    /* Get the sensor ID number to ensure there's a "valid" sensor name
       in the file attributes information. */
    sensor_id = ias_bpf_get_sensor_id(bpf);
    if (sensor_id == IAS_INVALID_SENSOR_ID)
    {
        IAS_LOG_ERROR("Retrieving sensor ID number from current sensor "
            "name");
        return ERROR;
    }

    /* Get the FILE_ATTRIBUTES group */
    file_attributes = ias_bpf_get_file_attributes(bpf);
    if (file_attributes == NULL)
    {
        IAS_LOG_ERROR("Cannot retrieve IAS_BPF_FILE_ATTRIBUTES group");
        return ERROR;
    }

    /* Make sure the sensor name information is compatible with the
       'sensor_id' field information in the BPF_MAIN table. */
    if (sensor_id == IAS_OLI)
    {
        sprintf(db_sensor_id_string, "OLI");
    }
    else
    {
        sprintf(db_sensor_id_string, "TIRS");
    }

    IAS_LOG_DEBUG("Parsing effective start/end times");
    
    /* effective_date_begin: must be valid date format */
    status = ias_misc_parse_datetime_string(
        file_attributes->effective_date_begin,
        IAS_DATETIME_CPF_FORMAT, &effective_date_begin);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Format of effective_date_begin (%s)",
            file_attributes->effective_date_begin);
        return ERROR;
    }

    /* effective_date_end: must be valid date format and must be 
     * equal to or later to begin date */
    status = ias_misc_parse_datetime_string(
        file_attributes->effective_date_end,
        IAS_DATETIME_CPF_FORMAT, &effective_date_end);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Format of effective_date_end (%s)",
            file_attributes->effective_date_end);
        return ERROR;
    }

    IAS_LOG_DEBUG("Building the query");

    /* Build the where clause for the query */ 
    status = snprintf(where_clause, max_clause,
        "EFFECTIVE_DATE_BEGIN <= "
        "to_date('%04d-%02d-%02d %02d.%02d.%02d','yyyy-MM-dd hh24.mi.ss') "
        "AND EFFECTIVE_DATE_END >= "
        "to_date('%04d-%02d-%02d %02d.%02d.%02d','yyyy-mm-dd hh24.mi.ss') "
        "AND sensor_id = '%s'",
        effective_date_begin.year,         effective_date_begin.month,
        effective_date_begin.day_of_month, effective_date_begin.hour,
        effective_date_begin.minute,       (int)effective_date_begin.second,
        effective_date_end.year,           effective_date_end.month,
        effective_date_end.day_of_month,   effective_date_end.hour,
        effective_date_end.minute,         (int)effective_date_end.second,
        db_sensor_id_string);
    if (status > max_clause)
    {
        IAS_LOG_ERROR("Query for the number of previous collections was"
            " unsuccessful.\n where clause: %s", where_clause);
        return ERROR;
    }

    table_size = sizeof(table)/sizeof(IAS_DB_QUERY_TABLE);

    IAS_LOG_DEBUG("Performing query"); 

    /* Perform the query */
    status = ias_db_query_using_table(conn, "BPF_MAIN",
        table, table_size, where_clause, 1);

    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Unable to perform query for BPF version");
        return ERROR;
    }

    IAS_LOG_DEBUG("Query for max version complete: %d", max_version);

    /* Set the version based on the version found */
    if (files_available < 1) 
    {
        max_version = 1;
        IAS_LOG_DEBUG("BPF is unique for the provided date range, starting "
            "the version at rev %d", max_version);
    }
    else
    {
        IAS_LOG_DEBUG("%d BPFs found for the provided date range with max "
            "version %d.", files_available, max_version);
        /* increment the max version */
        max_version += 1;
    }

    IAS_LOG_DEBUG("Setting the BPF version to %d", max_version);    

    /* todo the set should check for a max version and return a value */
    ias_bpf_set_file_attributes_version(bpf, max_version);

    return SUCCESS;
}

