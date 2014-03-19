/*----------------------------------------------------------------------------
NAME:    ias_misc_write_gcp_residuals

PURPOSE: Write out the residual for along and across scan components
        for each GCP to the residual file;

RETURNS: SUCCESS or ERROR
----------------------------------------------------------------------------*/
#include <stdio.h>
#include "ias_logging.h"
#include "ias_miscellaneous.h"
#include "ias_misc_gcp_residuals.h"

int ias_misc_write_gcp_residuals
(
    FILE *res_fptr,             /* I: Pointer to the rfile (residual file) */
    int num_gcp,                /* I: The # of GCPs used in the solution */
    int iter_num,               /* I: The iteration #; -1 is final iteration */
    const IAS_MISC_GCP_RESIDUAL *gcp_res,       /* I: Array of GCP information
                                                      structures */
    const IAS_REPORT_HEADER *header_information /* I: Header information */
)
{
    int i;          /* Looping variable */
    int status;     /* Return status */

    /* Output the standard header if on the first iteration. Also output the
       labels and units of the residual information. */
    if (iter_num == 0)
    {
        status = ias_misc_write_gps_report_header(res_fptr, header_information);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Writing the GPS header for this report");
            return ERROR;
        }

        /* Print out the band number, called for by the ADD. Use the band
           number of the first point; it should be the same for all the
           points. */
        status = fprintf(res_fptr, "Band Number:          %d\n\n\n",
                gcp_res[0].band_number);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing additional header information");
            return ERROR;
        }

        status = fprintf(res_fptr,
                "Point_ID    Predicted Predicted      Time      Latitude  "
                "Longitude    "
                "Height       Scan      Along   Across Residual Residual "
                "Outlier  Chip\n"
                "               Line     Sample                          "
                "              "
                "             Angle     Track   Track     In y     In x    "
                "Flag   Source\n"
                "                                                        "
                "              "
                "                      Residual Residual   Dir      Dir   "
                "(0=bad\n"
                "                                     (sec)       (deg)     "
                "(deg)     "
                "(meters)      (deg)    (meters) (meters) (meters) (meters) "
                "1=OK)\n");
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing additional header information");
            return ERROR;
        }
    }

    /* Write the first line with iteration number */
    if (iter_num >= 0)
    {
        status = fprintf(res_fptr, "Iteration %d\n", iter_num);
        if (status < 0)
        {
            IAS_LOG_ERROR("Printing to residual file");
            return ERROR;
        }
    }
    else
    {
        status = fprintf(res_fptr, "Final Iteration\n");
        if (status < 0)
        {
            IAS_LOG_ERROR("Printing to residual file");
            return ERROR;
        }
    }

    /* Loop over all GCPs to write out the residual information */
    for(i = 0; i < num_gcp; i++)
    {
        status = fprintf(res_fptr,
                "%-11s %9.3f %9.3f %12.6f %10.6f %11.6f %9.3f %12.6f "
                "%9.3f %9.3f %9.3f %9.3f %1d %-9s\n",
                gcp_res[i].point_id,           /* Point ID */
                gcp_res[i].predicted_line,     /* Predicted line */
                gcp_res[i].predicted_sample,   /* Predicted samp */
                gcp_res[i].seconds_from_epoch, /* Time, in seconds */
                gcp_res[i].latitude,           /* Latitude, in degrees */
                gcp_res[i].longitude,          /* Longitude, in degrees */
                gcp_res[i].height,             /* Height, in meters */
                gcp_res[i].across_track_angle, /* LOS angle (delta); degrees */
                gcp_res[i].along_track_residual,/* Residual on psi converted
                                                   to meters */
                gcp_res[i].across_track_residual,/* Residual on delta converted
                                                    to meters */
                gcp_res[i].residual_y,         /* Residual in y direction,
                                                  meters */
                gcp_res[i].residual_x,         /* Residual in x direction,
                                                  meters */
                gcp_res[i].outlier_flag,       /* Outlier flag: 1=OK, 0=bad */
                gcp_res[i].gcp_source);        /* Chip source (DOQ, ETM-1,
                                                  ETM-2, etc) */
        if (status < 0)
        {
            IAS_LOG_ERROR("Printing to residual file");
            return ERROR;
        }
    }

    return SUCCESS;
}
