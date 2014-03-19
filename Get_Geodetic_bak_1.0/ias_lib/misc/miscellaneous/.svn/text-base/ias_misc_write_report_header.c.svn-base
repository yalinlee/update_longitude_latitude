/**********************************************************************
NAME:               ias_misc_write_report_header

PURPOSE:    Writes common IAS report header information to a summary
            report

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

int ias_misc_write_report_header
(
    FILE *rpt_fptr,                        /* I: Summary report file */
    const IAS_REPORT_HEADER *header        /* I: Report header data */
)
{
    char date_string[DATE_STRLEN+1];    /* Date string */
    char time_string[TIME_STRLEN+1];    /* Time string */
    char spaces[] = "                                 ";
    struct tm *timeptr = NULL;          /* Time data structure */
    time_t t;                           /* Time variable */
    int status;
    int n;                              /* Computed # spaces for formatting */

    /* Sanity check -- legitimate file */
    if (rpt_fptr == NULL)
    {
        IAS_LOG_ERROR("Invalid summary report file handle");
        return ERROR;
    }

    /* Get the current date and put it in the correct structured format
       (i.e. Wed. Dec 10, 2007) */
    t = time(NULL);
    timeptr = localtime(&t);

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
    n = (64 - strlen(header->algorithm_name)) / 2;
    if (n < 0)
        n = 0;
    spaces[n] = '\0';

    status = fprintf(rpt_fptr, 
            "%s           OLI and TIRS            Time: %s\n"
            "                           EROS Data Center\n"
            "%s%s Report\n"
            "-----------------------------------------------------------------"
            "----------\n", date_string, time_string, spaces,
            header->algorithm_name);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing title block to the report file");
        return ERROR;
    }

    /* Output the work order ID, path, row, and software version */
    if (strcmp(header->work_order_id, "") == 0)
    {
        IAS_LOG_DEBUG("Work order ID is unavailable and will not be "
            "written to the output header");
    }
    else
    {
        status = fprintf(rpt_fptr, "Work Order ID:        %s\n",
                header->work_order_id);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing work order ID string to the output file");
            return ERROR;
        }
    }

    /* Write the IAS version */
    if (strcmp(header->ias_software_version, "") == 0)
    {
        IAS_LOG_DEBUG("Software version is unavailable and will not be "
                "written to the output header");
    }
    else
    {
        status = fprintf(rpt_fptr, "IAS Version:          %s\n",
            header->ias_software_version);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing IAS software version to the output file");
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
        status = fprintf(rpt_fptr, "Database schema:      %s\n",
            header->database_schema);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing the database schema to the output file");
            return ERROR;
        }
    }

    /* If we're dealing with an Earth scene and the target path/row is
       available, print the L0Rp subsetter software version here */
    if ((header->is_earth_scene) && ((header->path != 0)
        && (header->row != 0)))
    {
        if (strcmp(header->l0rp_subsetter_software_version, "") == 0)
        {
            IAS_LOG_DEBUG("L0Rp subsetter version is unavailable and will "
                    "not be written to the output header");
        }
        else
        {
            status = fprintf(rpt_fptr, "Subsetter Version:    %s\n",
                    header->l0rp_subsetter_software_version);
            if (status < 0)
            {
                IAS_LOG_ERROR("Writing subsetter software version to the "
                        "output file");
                return ERROR;
            }
        }
    }

    /* Write the L0R file name if it's available (e.g. this could be an
       empty string) */
    status = fprintf(rpt_fptr, "\n\nL0R Reference Image:  %s\n",
            header->l0r_filename);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing reference image to the output file");
        return ERROR;
    }

    /* Write target path/row if it's available for an Earth scene */
    /* TJR Always write WRS path / row */
    /* temporarily undid Tim's changes to make unit tests pass */
        status = fprintf(rpt_fptr, "WRS Path:             %03d\nWRS Row:"
            "              %03d\n", header->path, header->row);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing target path/row information to the "
                "output file");
            return ERROR;
        }

    if (!header->is_earth_scene)
    {
        status = fprintf(rpt_fptr, "Start WRS Path:       %03d\n"
                "Start WRS Row:        %03d\nEnd WRS Row:"
                "          %03d\n",
                header->starting_path, header->starting_row,
                header->ending_row);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing interval path/row information to the "
                    "output file");
            return ERROR;
        }
    }

    /* Write acquisition date/time */
    status = fprintf(rpt_fptr, "Date Acquired:        %s\n",
            header->date_acquired);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing acquisition date to the output file");
        return ERROR;
    }

    /* Write band and SCA number if they're available */
    if (header->band_number > 0)
    {
        status = fprintf(rpt_fptr, "Band Number:          %2d\n",
                header->band_number);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing band information to the output file");
            return ERROR;
        }
    }
    else
    {
        IAS_LOG_DEBUG("Band number is unavailable and will not be written to "
                "the output header");
    }
    if (header->sca_number > 0)
    {
        status = fprintf(rpt_fptr, "SCA Number:           %2d\n",
                header->sca_number);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing SCA information to the output file");
            return ERROR;
        }
    }
    else
    {
        IAS_LOG_DEBUG("SCA number is unavailable and will not be written to "
                "the output header");
    }

    /* Write spacecraft/sensor ID */
    status = fprintf(rpt_fptr, "\n\nSpacecraft:           %s\n"
            "Sensor:               %s\n", header->spacecraft_id,
            header->sensor_id);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing spacecraft ID and/or sensor ID to the "
                "output file");
        return ERROR;
    }

    return SUCCESS;
    /* END -- ias_misc_write_report_header */
}
