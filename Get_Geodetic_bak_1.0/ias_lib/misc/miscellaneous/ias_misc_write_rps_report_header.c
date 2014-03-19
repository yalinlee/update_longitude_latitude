/**********************************************************************
NAME:               ias_misc_write_rps_report_header

PURPOSE:    Writes RPS-specific report header information to a summary
            report

RETURNS:    Integer status code of SUCCESS or ERROR
************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_miscellaneous.h"
#include "ias_types.h"

int ias_misc_write_rps_report_header
(
    char *collection_type,           /* I: Collection type */
    char *cpf_name,                  /* I: Processing CPF name */
    char *sensor_id,                 /* I: Sensor ("OLI", "TIRS", or
                                           "OLI_TIRS") */
    RPS_REPORT_HEADER rps_header,    /* I: RPS-specific header data to write */
    FILE *rpt_fptr                   /* I: Report file handle */
)
{
    int status;

    /* Sanity check */
    if (rpt_fptr == NULL)
    {
        IAS_LOG_ERROR("No open report file");
        return ERROR;
    }

    /* Write the residual common header items for collection type
       and CPF name */
    if (collection_type == NULL)
        status = fprintf(rpt_fptr, "Collection Type:             N/A\n");
    else
    {
        status = fprintf(rpt_fptr, "Collection Type:             %s\n",
            collection_type);
    }
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing collection type to report file");
        return ERROR;
    }

    if (cpf_name == NULL)
        status = fprintf(rpt_fptr, "Processing CPF:              N/A\n");
    else
    {
        status = fprintf(rpt_fptr, "Processing CPF:              %s\n",
             cpf_name);
    }
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing CPF name to report file");
        return ERROR;
    }

    /* Now get to the meat of the RPS report header information */

    /* Scene start and stop times.  If this is a combined OLI/TIRS
       collect, BOTH sets of times are needed.  Using 'strstr' to
       check for the individual sensor ID in the overall sensor_id
       string should allow getting both sets. */
    if (strstr(sensor_id, "OLI"))
    {
        status = fprintf(rpt_fptr, "OLI Image Start Time:        %s\n",
                rps_header.acq_datetime.start_time_oli);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing OLI image start time string to report "
                "file");
            return ERROR;
        }

        status = fprintf(rpt_fptr, "OLI Image Stop Time:         %s\n",
                rps_header.acq_datetime.stop_time_oli);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing OLI image stop time string to report "
                "file");
            return ERROR;
        }

        /* Write total number of frames here */
        status = fprintf(rpt_fptr, "Number of frames:            %d\n",
            rps_header.num_oli_frames);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing number of frames to summary report");
            return ERROR;
        }

        /* Write frame rate information here */
        status = fprintf(rpt_fptr, "Average Frame Rate (frms/s): %d\n",
            rps_header.average_oli_frame_rate);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing OLI frame rate to report file");
            return ERROR;
        }
    }

    if (strstr(sensor_id, "TIRS"))
    {
        status = fprintf(rpt_fptr, "TIRS Image Start Time:       %s\n",
                rps_header.acq_datetime.start_time_tirs);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing TIRS image start time string to report "
                    "file");
            return ERROR;
        }

        status = fprintf(rpt_fptr, "TIRS Image Stop Time:        %s\n",
                rps_header.acq_datetime.stop_time_tirs);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing TIRS image stop time string to report "
                "file");
            return ERROR;
        }

        /* Write total number of frames here */
        status = fprintf(rpt_fptr, "Number of frames:            %d\n",
            rps_header.num_tirs_frames);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing number of frames to summary report");
            return ERROR;
        }

        /* Write frame rate information here */
        status = fprintf(rpt_fptr, "Average Frame Rate (frms/s): %d\n",
            rps_header.average_tirs_frame_rate);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing TIRS frame rate to report file");
            return ERROR;
        }
    }

    /* Sensor integration times and detector select table */
    status = fprintf(rpt_fptr, "MS Integration Time (usec):  %d\n",
            rps_header.ms_integration_time);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing MS integration time string to report file");
        return ERROR;
    }

    status = fprintf(rpt_fptr, "PAN Integration Time (usec): %d\n",
            rps_header.pan_integration_time);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing PAN integration time string to report file");
        return ERROR;
    }

    status = fprintf(rpt_fptr, "Detector Selection Table:    %d\n",
            rps_header.current_detector_select_table);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing detector select table string to report file");
        return ERROR;
    }

    status = fprintf(rpt_fptr, "Data Truncation Setting:     %d\n\n",
            rps_header.image_data_truncation_setting);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing data truncation setting to report file");
        return ERROR;
    }

    /* Focal plane temperatures. It's possible OLI AND TIRS imaged during
       a given collect, so BOTH sets of temperatures need to be written
       here */
    status = fprintf(rpt_fptr, "Focal Plane Temperatures\n");
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing temperature header to report file");
        return ERROR;
    }

    status = fprintf(rpt_fptr, "                                   "
            "  Start     Stop\n");
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing temperature header to report file");
        return ERROR;
    }

    status = fprintf(rpt_fptr, "                                   "
            "  ---------------\n");
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing temperature header string to report file");
        return ERROR;
    }

    /* OLI Temperatures */
    if (strstr(sensor_id, "OLI"))
    {
        status = fprintf(rpt_fptr, "SCA  7 Temperature (C):            "
                "%7.3f   %7.3f\n", rps_header.oli_fpm7_start_temp,
                rps_header.oli_fpm7_stop_temp);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing SCA 7 temperature string to report file");
            return ERROR;
        }

        status = fprintf(rpt_fptr, "SCA 14 Temperature (C):            "
                "%7.3f   %7.3f\n", rps_header.oli_fpm14_start_temp,
                rps_header.oli_fpm14_stop_temp);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing SCA 14 temperature string to report file");
            return ERROR;
        }

        status = fprintf(rpt_fptr, "FPA Window Temperature (C):        "
                "%7.3f   %7.3f\n", rps_header.oli_fpa_window_start_temp,
                rps_header.oli_fpa_window_stop_temp);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing FPA window temperature string to report "
                    "file");
            return ERROR;
        }

        status = fprintf(rpt_fptr, "FPE Chassis Temperature (C):       "
                "%7.3f   %7.3f\n\n", rps_header.oli_fpe_chassis_start_temp,
                rps_header.oli_fpe_chassis_stop_temp);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing FPE chassis temperature string to report "
                    "file");
            return ERROR;
        }
    }

    /* TIRS temperatures */
    if (strstr(sensor_id, "TIRS"))
    {
        status = fprintf(rpt_fptr, "Blackbody 1 Temperature (C):"
                "       %7.3f   %7.3f\n",
                rps_header.tirs_blackbody_1_start_temp,
                rps_header.tirs_blackbody_1_stop_temp);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing TIRS blackbody 1 temperature to report "
                    "file");
            return ERROR;
        }

        status = fprintf(rpt_fptr, "Blackbody 2 Temperature (C):"
                "       %7.3f   %7.3f\n",
                rps_header.tirs_blackbody_2_start_temp,
                rps_header.tirs_blackbody_2_stop_temp);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing TIRS blackbody 2 temperature to report "
                    "file");
            return ERROR;
        }

        status = fprintf(rpt_fptr, "Blackbody 3 Temperature (C):"
                "       %7.3f   %7.3f\n",
                rps_header.tirs_blackbody_3_start_temp,
                rps_header.tirs_blackbody_3_stop_temp);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing TIRS blackbody 3 temperature to report "
                    "file");
            return ERROR;
        }

        status = fprintf(rpt_fptr, "Blackbody 4 Temperature (C):"
                "       %7.3f   %7.3f\n",
                rps_header.tirs_blackbody_4_start_temp,
                rps_header.tirs_blackbody_4_stop_temp);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing TIRS blackbody 4 temperature to report "
                    "file");
            return ERROR;
        }

        status = fprintf(rpt_fptr, "Cryocooler 1 Temperature (C):"
                "      %7.3f   %7.3f\n",
                rps_header.tirs_cryocooler_1_start_temp,
                rps_header.tirs_cryocooler_1_stop_temp);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing TIRS cryocooler 1 temperature to report "
                    "file");
            return ERROR;
        }

        status = fprintf(rpt_fptr, "Cryocooler 2 Temperature (C):"
                "      %7.3f   %7.3f\n",
                rps_header.tirs_cryocooler_2_start_temp,
                rps_header.tirs_cryocooler_2_stop_temp);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing TIRS cryocooler 2 temperature to report "
                    "file");
            return ERROR;
        }
    }

    /* Done */
    return SUCCESS;
}
