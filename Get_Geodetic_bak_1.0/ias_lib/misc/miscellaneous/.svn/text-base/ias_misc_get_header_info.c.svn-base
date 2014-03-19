/*************************************************************************
 NAME:     ias_misc_get_header_info

 PURPOSE:  These are public interface (API) routines which
           applications/library routines can use to retrieve values for
           the standard IAS_REPORT_HEADER members.

 NOTES:    If more optional members are added to IAS_REPORT_HEADER, the
           corresponding "getter" functions should be defined in this 
           module and prototyped in "ias_miscellaneous.h"
 
 RETURNS:  If a value is not set, that getter routine returns either
           ERROR or NULL (if return type is a pointer).
**************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ias_miscellaneous.h"
#include "ias_miscellaneous_private.h"
#include "ias_logging.h"

int ias_misc_get_header_band_number
(
    IAS_REPORT_HEADER *header   /* I: Report header */
)
{
    if (header->band_number < 0)
    {
        IAS_LOG_DEBUG("Band number not set in report header");
        return ERROR;
    }
    return header->band_number;
}

int ias_misc_get_header_sca_number
(
    IAS_REPORT_HEADER *header   /* I: Report header */
)
{
    if (header->sca_number <= 0)
    {
        IAS_LOG_DEBUG("SCA number not set in report header");
        return ERROR;
    }
    return header->sca_number;
}

char *ias_misc_get_header_algorithm_name
(
    IAS_REPORT_HEADER *header   /* I Report header */
)
{
    if (strcmp(header->algorithm_name, "") == 0)
    {
        IAS_LOG_DEBUG("Algorithm name not set in report header");
        return NULL;
    }
    return header->algorithm_name;
}

void ias_misc_get_header_pathrow
(
    IAS_REPORT_HEADER *header,  /* I: Report header */
    int *path,                  /* O: Returned WRS path */
    int *row                    /* O: Returned WRS row */
)
{
    /* Get the information */
    *path = header->path;
    *row = header->row;
}

void ias_misc_get_header_target_pathrow
(
    IAS_REPORT_HEADER *header,  /* I: Report header */
    int *path,                  /* O: Returned WRS target path */
    int *row                    /* O: Returned WRS target row */
)
{
    /* Get the information */
    *path = header->target_path;
    *row = header->target_row;
}

void ias_misc_get_header_starting_ending_pathrow
(
    IAS_REPORT_HEADER *header,  /* I: Report header */
    int *starting_path,         /* O: Interval start path */
    int *starting_row,          /* O: Interval start row */
    int *ending_row             /* O: Interval end row */
)
{
    /* Get the information */
    *starting_path = header->starting_path;
    *starting_row = header->starting_row;
    *ending_row = header->ending_row;
}

char *ias_misc_get_header_cpf_name
(
    IAS_REPORT_HEADER *header   /* I: Report header */
)
{
    if (strcmp(header->cpf_name, "") == 0)
    {
        IAS_LOG_DEBUG("CPF filename not set in report header");
        return NULL;
    }
    return header->cpf_name;
}

char *ias_misc_get_header_collection_type
(
    IAS_REPORT_HEADER *header   /* I: Report header */
)
{
    if (strcmp(header->collection_type, "") == 0)
    {
        IAS_LOG_DEBUG("Collection type not set in report header");
        return NULL;
    }
    return header->collection_type;
}

char *ias_misc_get_header_work_order_id
(
    IAS_REPORT_HEADER *header   /* I: Report header */
)
{
    if (strcmp(header->work_order_id, "") == 0)
    {
        IAS_LOG_DEBUG("Work order ID not set in report header");
        return NULL;
    }
    return header->work_order_id;
}

char *ias_misc_get_header_l0r_filename
(
    IAS_REPORT_HEADER *header   /* I: Report header */
)
{
    if (strcmp(header->l0r_filename, "") == 0)
    {
        IAS_LOG_DEBUG("L0R file name not set in report header");
        return NULL;
    }
    return header->l0r_filename;
}

char *ias_misc_get_header_l1r_filename
(
    IAS_REPORT_HEADER *header   /* I: Report header */
)
{
    if (strcmp(header->l1r_filename, "") == 0)
    {
        IAS_LOG_DEBUG("L0R file name not set in report header");
        return NULL;
    }
    return header->l1r_filename;
}

char *ias_misc_get_header_spacecraft_id
(
    IAS_REPORT_HEADER *header   /* I: Report header */
)
{
    if (strcmp(header->spacecraft_id, "") == 0)
    {
        IAS_LOG_DEBUG("Spacecraft ID not set in report header");
        return NULL;
    }
    return header->spacecraft_id;
}

char *ias_misc_get_header_sensor_id
(
    IAS_REPORT_HEADER *header   /* I: Report header */
)
{
    if (strcmp(header->sensor_id, "") == 0)
    {
        IAS_LOG_DEBUG("Sensor ID not set in report header");
        return NULL;
    }
    return header->sensor_id;
}

char *ias_misc_get_header_ias_sw_version
(
    IAS_REPORT_HEADER *header   /* I: Report header */
)
{
    if (strcmp(header->ias_software_version, "") == 0)
    {
        IAS_LOG_DEBUG("IAS software version not set in report header");
        return NULL;
    }
    return header->ias_software_version;
}

char *ias_misc_get_header_ingest_sw_version
(
    IAS_REPORT_HEADER *header   /* I: Report header */
)
{
    if (strcmp(header->ingest_software_version, "") == 0)
    {
        IAS_LOG_DEBUG("Ingest software version not set in report header");
        return NULL;
    }
    return header->ingest_software_version;
}

char *ias_misc_get_header_l0rp_subsetter_sw_version
(
    IAS_REPORT_HEADER *header   /* I: Report header */
)
{
    if (strcmp(header->l0rp_subsetter_software_version, "") == 0)
    {
        IAS_LOG_DEBUG("Subsetter version not set in report header");
        return NULL;
    }
    return header->l0rp_subsetter_software_version;
}

char *ias_misc_get_header_date_acquired
(
    IAS_REPORT_HEADER *header   /* I: Report header */
)
{
    if (strcmp(header->date_acquired, "") == 0)
    {
        IAS_LOG_DEBUG("Acquisition date not set in report header");
        return NULL;
    }
    return header->date_acquired;
}

char *ias_misc_get_header_ancillary_start_time
(
    IAS_REPORT_HEADER *header   /* I: Report header */
)
{
    if (strcmp(header->ancillary_start_time, "") == 0)
    {
        IAS_LOG_DEBUG("Ancillary start time not set in report header");
        return NULL;
    }
    return header->ancillary_start_time;
}

char *ias_misc_get_header_ancillary_stop_time
(
    IAS_REPORT_HEADER *header   /* I: Report header */
)
{
    if (strcmp(header->ancillary_stop_time, "") == 0)
    {
        IAS_LOG_DEBUG("Ancillary stop time not set in report header");
        return NULL;
    }
    return header->ancillary_stop_time;
}
