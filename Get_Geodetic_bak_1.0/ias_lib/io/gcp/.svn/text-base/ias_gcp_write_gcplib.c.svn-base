
/*****************************************************************************
NAME: ias_gcp_write_gcplib

PURPOSE: Creates a GCPLib file from an array of GCP Records.

RETURN VALUE: type = int

    Value    Description
    -------  ----------------------------------------------------------------
    SUCCESS  Successfully retrieved the GCP information.
    ERROR    Failure to get the GCP information.

*****************************************************************************/

/* Standard C Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IAS Includes */
#include "ias_types.h"
#include "ias_gcp.h"
#include "ias_logging.h"
#include "ias_miscellaneous.h"

int ias_gcp_write_gcplib
(
    const char *gcplib_filename,       /* I: Output GCP filename */
    const IAS_GCP_RECORD *gcp_records, /* I: Structure of chip information */
    int number_of_gcps           /* I: Number of ground control points */
)
{
    FILE *output_fd;            /* Pointer to the GCPLib output file */
    int status;                 /* Status of return from function */
    int index;                  /* Loop control variables */
    const char *data_type;      /* To hold the string version of the type */

    output_fd = fopen(gcplib_filename, "w");
    if(output_fd == NULL)
    {
        IAS_LOG_ERROR("Opening output GCPLib file %s", gcplib_filename);
        return ERROR;
    }

    /* Output header information */
    status = fprintf(output_fd,
        "# Unique Ground Control Point ID - integer\n"
        "# Name of the chip image - string\n"
        "# Reference line - double\n"
        "# Reference sample - double\n"
        "# True latitude of reference point in degrees - double\n"
        "# True longitude of reference point in degrees - double\n"
        "# Projection x value of reference point in meters - double\n"
        "# Projection y value of reference point in meters - double\n"
        "# Elevation at the Ground Control Point in meters - double\n"
        "# Pixel size x value in meters - double\n"
        "# Pixel size y value in meters - double\n"
        "# Chip size in lines - double\n"
        "# Chip size in samples - double\n"
        "# Ground Control Point source - string\n"
        "# Ground Control Point type - string\n"
        "# Chip projection - string\n"
        "# Projection UTM zone - integer\n"
        "# Acquisition date - string in form mm-dd-yyyy\n"
        "# Absolute or relative flag - string, should be ABS or REL\n"
        "# Image chip data type - string\n"
        "BEGIN\n");
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing header to GCPLib file");
        fclose(output_fd);
        return ERROR;
    }
    status = fprintf(output_fd, "%d\n", number_of_gcps);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing to GCPLib file");
        fclose(output_fd);
        return ERROR;
    }

    for (index = 0; index < number_of_gcps; index++)
    {
        /* Determine the string representation for the data type */
        status = ias_misc_convert_data_type_to_string(
            gcp_records[index].chip_data_type, &data_type);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Getting string for GCP data type %d for point %d",
                    gcp_records[index].chip_data_type, index);
            fclose(output_fd);
            return ERROR;
        }

        /* Write the record to the file */
        status = fprintf(output_fd,
            "%s %s "
            "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf "
            "%s %s %s %d %s %s %s\n",
            gcp_records[index].point_id,
            gcp_records[index].chip_name,
            gcp_records[index].reference_line,
            gcp_records[index].reference_sample,
            gcp_records[index].latitude,
            gcp_records[index].longitude,
            gcp_records[index].projection_y,
            gcp_records[index].projection_x,
            gcp_records[index].elevation,
            gcp_records[index].pixel_size_x,
            gcp_records[index].pixel_size_y,
            gcp_records[index].chip_size_lines,
            gcp_records[index].chip_size_samples,
            gcp_records[index].source,
            gcp_records[index].chip_type,
            gcp_records[index].projection,
            gcp_records[index].zone,
            gcp_records[index].date,
            gcp_records[index].absolute_or_relative,
            data_type);

        if (status < 0)
        {
            IAS_LOG_ERROR("Writing to GCPLib file");
            fclose(output_fd);
            return ERROR;
        }
    }

    if (fclose(output_fd) != 0)
    {
        IAS_LOG_ERROR("Closing output GCPLib file %s", gcplib_filename);
        return ERROR;
    }

    return SUCCESS;
}
