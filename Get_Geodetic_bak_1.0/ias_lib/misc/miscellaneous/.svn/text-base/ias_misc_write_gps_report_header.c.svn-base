/**********************************************************************
NAME:       ias_misc_write_gps_report_header

PURPOSE:    Writes IAS GPS report header information to a summary report

RETURNS:    Integer status code of SUCCESS or ERROR
************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "ias_l0r.h"
#include "ias_miscellaneous.h"
#include "ias_miscellaneous_private.h"
#include "ias_structures.h"
#include "ias_const.h"
#include "ias_logging.h"

int ias_misc_write_gps_report_header
(
    FILE *file_ptr,                        /* I: Summary report file */
    const IAS_REPORT_HEADER *header        /* I: Report header data */
)
{
    char date_string[DATE_STRLEN+1];    /* Date string */
    char time_string[TIME_STRLEN+1];    /* Time string */
    char spaces[] = "                                  ";
    struct tm *timeptr = NULL;          /* Time data structure */
    time_t cur_time;                    /* Time variable */
    int status;
    int num_spaces;                     /* Computed # spaces for formatting */

    /* Sanity check -- legitimate file */
    if (file_ptr == NULL)
    {
        IAS_LOG_ERROR("Invalid GPS summary report file handle");
        return ERROR;
    }

    /* Get the current date and put it in the correct structured format
       (i.e. Wed. Dec 10, 2007) */
    cur_time = time(NULL);
    timeptr = localtime(&cur_time);

    if (!strftime(date_string, DATE_STRLEN, "%a. %b. %e, %Y", timeptr))
    {
        IAS_LOG_ERROR("Extracting the current date");
        return ERROR;
    }

    /* Get the current time and put it in the correct structured format
       (i.e. 17:24) */
    if (!strftime(time_string, TIME_STRLEN, "%R", timeptr))
    {
        IAS_LOG_ERROR("Extracting the current time");
        return ERROR;
    }

    /* Write out the date, time, and application name */
    num_spaces = (66 - strlen(header->algorithm_name)) / 2;
    if (num_spaces < 0)
        num_spaces = 0;
    spaces[num_spaces] = '\0';

    status = fprintf(file_ptr, 
            "%s          Processed at %s          Time: %s\n"
            "%s%s Report\n"
            "-----------------------------------------------------------------"
            "----------\n", date_string, header->processing_center, time_string,
            spaces, header->algorithm_name);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing title block to the report file");
        return ERROR;
    }

    /* Output the work order ID */
    if ((header->work_order_id == NULL) ||
        (strcmp(header->work_order_id, "") == 0))
    {
        IAS_LOG_DEBUG("Work Order ID is unavailable and will not be "
            "written to the output header");
    }
    else
    {
        status = fprintf(file_ptr, "Work Order ID:        %s\n",
                header->work_order_id);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing Work Order ID string to the output file");
            return ERROR;
        }
    }

    /* Write software version */
    if (strcmp(header->ias_software_version, "") == 0)
    {
        IAS_LOG_DEBUG("Software Version is unavailable and will not be "
                "written to the output header");
    }
    else
    {
        status = fprintf(file_ptr, "IAS Version:          %s\n",
            header->ias_software_version);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing IAS Version to the output file");
            return ERROR;
        }
    }

    /* Write database schema */
    if (strcmp(header->database_schema, "") == 0)
    {
        IAS_LOG_DEBUG("Database schema is unavailable and will not be written "
            "to the output header");
    }
    else
    {
        status = fprintf(file_ptr, "Database schema:      %s\n",
            header->database_schema);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing the database schema to the output file");
            return ERROR;
        }
    }

    /* ------------------------------------------------------------------ */

    /* Write the L0R file name if it's available (e.g. this could be an
       empty string) */
    status = fprintf(file_ptr, "\n\nL0R Image:            %s\n",
            header->l0r_filename);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing L0R Image to the output file");
        return ERROR;
    }

    /* Write the L1G filename */
    status = fprintf(file_ptr, "L1G/T Image:          %s\n",
            header->l1g_filename);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing L1G/T Image to the output file");
        return ERROR;
    }
    
    /* Write target path/row */
    status = fprintf(file_ptr, "WRS Path:             %03d\nWRS Row:"
        "              %03d\n", header->path, header->row);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing WRS Path/Row to the output file");
        return ERROR;
    }

    /* Write acquisition date */
    status = fprintf(file_ptr, "Acquisition Date:     %s\n",
            header->date_acquired);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing Acquisition Date to the output file");
        return ERROR;
    }

    /* ------------------------------------------------------------------ */

    /* Write spacecraft */
    status = fprintf(file_ptr, "\n\nSpacecraft:           %s\n",
            header->spacecraft_id);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing Spacecraft to the output file");
        return ERROR;
    }
    
    /* Write the instrument */
    status = fprintf(file_ptr, "Instrument:           %s\n", header->sensor_id);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing Instrument to the output file");
        return ERROR;
    }

    /* Write the acquisition type */
    status = fprintf(file_ptr, "Acquisition Type:     %s\n",
            header->collection_type);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing Acquisition Type to the output file");
        return ERROR;
    }

    /* Write the off-nadir roll angle */
    status = fprintf(file_ptr, "Off-Nadir Angle:      %.1lf\n",
            header->roll_angle);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing Off-Nadir Angle to the output file");
        return ERROR;
    }

    /* Add a little space to separate the report "header" from the report
       "contents" the applications will be writing out following this */
    status = fprintf(file_ptr, "\n\n");
    if (status < 0)
    {
        IAS_LOG_WARNING("Writing formatting spacing between report header "
                "and body");
    }

    return SUCCESS;
    /* END -- ias_misc_write_gps_report_header */
}
