/*************************************************************************
 NAME:             ias_misc_set_header_info

 PURPOSE:  These are public interface (API) routines which
           applications/library routines can use to set values for
           the standard IAS_REPORT_HEADER members not initialized from
           the L0R metadata in ias_misc_initialize_report_header().

 NOTES:    If more optional members are added to IAS_REPORT_HEADER, the
           corresponding "setter" functions should be defined in this 
           module and prototyped in "ias_miscellaneous.h"

           The setter functions implemented here currently assume the
           calling routine is passing in "reasonable" parameter values.
           Future updates to these functions may include parameter value
           checking.
**************************************************************************/
#include <stdio.h>
#include <string.h>
#include <libgen.h>                      /* For basename() command */
#include "ias_miscellaneous.h"
#include "ias_miscellaneous_private.h"
#include "ias_logging.h"

#define MIN_WRS_PATH    0
#define MAX_WRS_PATH    233
#define MIN_WRS_ROW     0
#define MAX_WRS_ROW     248

void ias_misc_set_header_band_number
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    int band_number                 /* I: Band number to set */
)
{
    header->band_number = band_number;
}

void ias_misc_set_header_sca_number
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    int sca_number                  /* I: SCA number to set */
)
{
    header->sca_number = sca_number;
}

void ias_misc_set_header_acquisition_date
(
   IAS_REPORT_HEADER *header,       /* I/O: Report header to modify */
   const char *acquisition_date     /* I: Acquisition date */
)
{
    strncpy(header->date_acquired, acquisition_date,
            sizeof(header->date_acquired));
    header->date_acquired[sizeof(header->date_acquired) - 1] = '\0';
}

void ias_misc_set_header_algorithm_name
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    const char *algorithm_name      /* I: Name of application to set */
)
{
    strncpy(header->algorithm_name, algorithm_name,
            sizeof(header->algorithm_name));
    header->algorithm_name[sizeof(header->algorithm_name) - 1] = '\0';
}

int ias_misc_set_header_pathrow
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    int path,                       /* I: WRS2 path to set */
    int row                         /* I: WRS2 row to set */
)
{
    /* Do some bounds checking */
    if (((path < MIN_WRS_PATH) || (path > MAX_WRS_PATH))
            || ((row < MIN_WRS_ROW) || (row > MAX_WRS_ROW)))
    {
        IAS_LOG_ERROR("Invalid path/row inputs");
        header->path = -1;
        header->row = -1;
        return ERROR;
    }

    /* Set the fields */
    header->path = path;
    header->row = row;

    /* Done */
    return SUCCESS;
}

int ias_misc_set_header_starting_ending_pathrow
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    int starting_path,              /* O: Interval starting path */
    int starting_row,               /* O: Interval start row */
    int ending_row                  /* O: Interval end row */
)
{
    /* Do some bounds checking */
    if (((starting_path < MIN_WRS_PATH) || (starting_path > MAX_WRS_PATH))
            || ((starting_row < MIN_WRS_ROW) || (starting_row > MAX_WRS_ROW))
            || ((ending_row < MIN_WRS_ROW) || (ending_row > MAX_WRS_ROW))
            || (starting_row > ending_row))
    {
        IAS_LOG_ERROR("Invalid path/row inputs");
        header->starting_path = -1;
        header->starting_row = -1;
        header->ending_row = -1;
        return ERROR;
    }

    /* Set the fields */
    header->starting_path = starting_path;
    header->starting_row = starting_row;
    header->ending_row = ending_row;

    /* Done */
    return SUCCESS;
}

void ias_misc_set_header_cpf_name
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    const char *cpf_name            /* I: Processing CPF name */
)
{
    char local_cpf_name[PATH_MAX];

    /* Basename() needs a non-constant pointer, so make a local copy
       of the string before using it */
    strncpy(local_cpf_name, cpf_name, sizeof(local_cpf_name));
    local_cpf_name[sizeof(local_cpf_name) - 1] = '\0';

    strncpy(header->cpf_name, basename(local_cpf_name),
        sizeof(header->cpf_name));
    header->cpf_name[sizeof(header->cpf_name) - 1] = '\0';
}

void ias_misc_set_header_collection_type
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    const char *collection_type     /* I: Collection type */
)
{
    strncpy(header->collection_type, collection_type,
            sizeof(header->collection_type));
    header->collection_type[sizeof(header->collection_type) - 1] = '\0';
}

void ias_misc_set_header_l0r_filename
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    const char *l0r_filename        /* L0R file name */
)
{
    char local_l0r_filename[PATH_MAX];

    /* Basename() needs a non-constant pointer, so make a local copy
       of the string before using it */
    strncpy(local_l0r_filename, l0r_filename, sizeof(local_l0r_filename));
    local_l0r_filename[sizeof(local_l0r_filename) - 1] = '\0';

    strncpy(header->l0r_filename, basename(local_l0r_filename),
        sizeof(header->l0r_filename));
    header->l0r_filename[sizeof(header->l0r_filename) - 1] = '\0';
}

void ias_misc_set_header_l1r_filename
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    const char *l1r_filename        /* I: L1R file name */
)
{
    char local_l1r_filename[PATH_MAX];

    /* Basename() needs a non-constant pointer, so make a local copy
       of the string before using it */
    strncpy(local_l1r_filename, l1r_filename, sizeof(local_l1r_filename));
    local_l1r_filename[sizeof(local_l1r_filename) - 1] = '\0';

    strncpy(header->l1r_filename, basename(local_l1r_filename),
        sizeof(header->l1r_filename));
    header->l1r_filename[sizeof(header->l1r_filename) - 1] = '\0';
}

void ias_misc_set_header_spacecraft_id
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    const char *spacecraft_id       /* I: Spacecraft ID */
)
{
    strncpy(header->spacecraft_id, spacecraft_id,
            sizeof(header->spacecraft_id));
    header->spacecraft_id[sizeof(header->spacecraft_id) - 1] = '\0';
}

void ias_misc_set_header_sensor_id
(
    IAS_REPORT_HEADER *header,      /* I/O: Report header to modify */
    const char *sensor_id           /* I: Sensor ID */
)
{
    strncpy(header->sensor_id, sensor_id, sizeof(header->sensor_id));
    header->sensor_id[sizeof(header->sensor_id) - 1] = '\0';
}
