/**********************************************************************
 NAME:      ias_misc_initialize_gps_report_header

 PURPOSE:   Initializes the IAS GPS report header information

 RETURNS:   Pointer to an IAS_REPORT_HEADER data structure if successful
            NULL pointer if an error has occurred
************************************************************************/
#include <libgen.h>     /* for basename() */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ias_const.h"
#include "ias_l1g.h"
#include "ias_logging.h"
#include "ias_miscellaneous.h"
#include "ias_miscellaneous_private.h"
#include "ias_structures.h"
#include "ias_types.h"
#include "ias_satellite_attributes.h"

/*----------------------------------------------------------------------------
NAME:    ias_misc_initialize_gps_report_header
-----------------------------------------------------------------------------*/
IAS_REPORT_HEADER *ias_misc_initialize_gps_report_header
(
    const char *l1g_filename,       /* I; Path and name of L1G file */
    const char *l0r_filename,       /* I: Basename of L0R file, or NULL */
    const char *work_order_id,      /* I: Work order ID string */
    const char *algorithm_name      /* I: Name of characterization algorithm */
)
{
    IAS_REPORT_HEADER *header = NULL;   /* Common header data structure */
    L1GIO *l1g_ptr = NULL;              /* Open L1G data file */
    IAS_L1G_FILE_METADATA fmd;          /* Image file metadata */
    IAS_L1G_BAND_METADATA band_meta;    /* Band level metadata from L1G file */
    int band_index;                     /* Band index for looping */
    int status;                         /* Return status */
    const char *sw_version = NULL;      /* IAS software version */
    char *processing_center = NULL;     /* IAS processing center */
    char *connection_str;               /* Database connection str */
    char *tmp_ptr;
    int length;
    char local_l0r_filename[PATH_MAX];
    char local_l1g_filename[PATH_MAX];

    /* Allocate a report header data block */
    header = calloc(1, sizeof(*header));
    if (header == NULL)
    {
        IAS_LOG_ERROR("Cannot allocate report header data block");
        return NULL;
    }

    /* Initialize the work order ID and application name */
    strncpy(header->work_order_id, work_order_id,
            sizeof(header->work_order_id));
    header->work_order_id[sizeof(header->work_order_id) - 1] = '\0';
    strncpy(header->algorithm_name, algorithm_name,
            sizeof(header->algorithm_name));
    header->algorithm_name[sizeof(header->algorithm_name) - 1] = '\0';

    /* Initialize the IAS software version */
    sw_version = ias_misc_get_software_version();
    if (sw_version == NULL)
    {
        IAS_LOG_WARNING("IAS software version information unavailable");
    }
    else
    {
        strncpy(header->ias_software_version, sw_version,
                sizeof(header->ias_software_version));
        header->ias_software_version[sizeof(header->ias_software_version) - 1]
            = '\0';
    }

    /* Initialize the processing center */
    processing_center = getenv("PROCESSING_CENTER");
    if (processing_center == NULL)
    {
        IAS_LOG_WARNING("Processing center information unavailable");
        strncpy(header->processing_center, "unknown",
                sizeof(header->processing_center));
        header->processing_center[sizeof(header->processing_center) - 1] = '\0';
    }
    else
    {
        strncpy(header->processing_center, processing_center,
                sizeof(header->processing_center));
        header->processing_center[sizeof(header->processing_center) - 1] = '\0';
    }

    /* Set the database schema based on the IAS_DB_L8_OLITIRS environment
       variable. */
    connection_str = getenv("IAS_DB_L8_OLITIRS");
    if (connection_str != NULL)
    {
        tmp_ptr = strchr(connection_str, '/');
        if (tmp_ptr != NULL)
        {
            length = tmp_ptr - connection_str;
            strncpy(header->database_schema, connection_str, length);
            header->database_schema[length] = '\0';
        }
        else
        {
            IAS_LOG_WARNING("Could not parse schema from IAS_DB_L8_OLITIRS");
            strncpy(header->database_schema, "", 
                sizeof(header->database_schema));
        }
    }
    else
    {
        strncpy(header->database_schema, "", sizeof(header->database_schema));
    }

    /* If the L0R filename was provided, copy that to the header info */
    if ((l0r_filename != NULL) && (strcmp(l0r_filename, "") != 0))
    {
        /* basename needs a non-const pointer, so make local copy before
           using it */
        strncpy(local_l0r_filename, l0r_filename, sizeof(local_l0r_filename));
        local_l0r_filename[sizeof(local_l0r_filename) - 1] = '\0';

        strncpy(header->l0r_filename, basename(local_l0r_filename),
                sizeof(header->l0r_filename));
        header->l0r_filename[sizeof(header->l0r_filename) - 1] = '\0';
    }

    /* Get the L1G filename */
    if ((l1g_filename == NULL) || (strcmp(l1g_filename, "") == 0))
    {
        IAS_LOG_WARNING("L1G filename missing; cannot initialize GPS report "
                "header");
        return header;
    }

    /* basename needs a non-const pointer, so make local copy before using it */
    strncpy(local_l1g_filename, l1g_filename, sizeof(local_l1g_filename));
    local_l1g_filename[sizeof(local_l1g_filename) - 1] = '\0';

    strncpy(header->l1g_filename, basename(local_l1g_filename),
            sizeof(header->l1g_filename));
    header->l1g_filename[sizeof(header->l1g_filename) - 1] = '\0';

    /* Open the L1G file. The ias_l1g_open_image function requires the L1G
       filename string to contain the path and name of the L1G file */
    l1g_ptr = ias_l1g_open_image(l1g_filename, IAS_READ);
    if (l1g_ptr == NULL)
    {
        IAS_LOG_ERROR("Unable to open L1G file %s", l1g_filename);
        return header;
    }

    /* Get the L1G file metadata */
    if(ias_l1g_get_file_metadata(l1g_ptr, &fmd) != SUCCESS)
    {
        IAS_LOG_ERROR("Reading file metadata from L1G file %s", l1g_filename);
        ias_l1g_close_image(l1g_ptr);
        return header;
    }

    /* Get the acquisition type */
    strncpy(header->collection_type, fmd.collection_type,
            sizeof(header->collection_type));
    header->collection_type[sizeof(header->collection_type) - 1] = '\0';

    /* Get the path/row information */
    header->path = fmd.wrs_path;
    header->row  = fmd.wrs_row;

    /* Get the date the scene was acquired */
    strncpy(header->date_acquired, fmd.capture_date,
            sizeof(header->date_acquired));
    header->date_acquired[sizeof(header->date_acquired) - 1] = '\0';

    /* Get the off-nadir roll angle */
    header->roll_angle = fmd.roll_angle;

    /* Get the sensor and instrument information */
    strncpy(header->spacecraft_id, fmd.spacecraft,
            sizeof(header->spacecraft_id));
    header->spacecraft_id[sizeof(header->spacecraft_id) - 1] = '\0';
    for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
    {
        int band_number;

        band_number = ias_sat_attr_convert_band_index_to_number(band_index);
        if (band_number == ERROR)
        {
            IAS_LOG_WARNING("Converting band index %d to band number",
                    band_index);
            continue;
        }

        /* Get the band level metadata for the current band */
        if (ias_l1g_is_band_present(l1g_ptr, band_number))
        {
            status = ias_l1g_get_band_metadata(l1g_ptr, band_number,
                    &band_meta);
            if (status != SUCCESS)
            {
                IAS_LOG_WARNING("Retrieving L1G band metadata for band %d of "
                        "image %s", band_number, l1g_filename);
                continue;
            }

            /* If the current band's instrument source isn't already in the
               string for the header, tack it on to the end. This results in
               a string composed of each sensor present in the data, ordered by
               the sensors listed in the band order. Typically, this will
               result in a string "OLI", if only OLI bands present. If TIRS
               bands present only, a string "TIRS". If OLI and TIRS bands
               present, a string "OLITIRS". */
            if (strstr(header->sensor_id, band_meta.instrument_source) == NULL)
            {
                int cur_len;

                cur_len = strlen(header->sensor_id);
                strncat(header->sensor_id, band_meta.instrument_source,
                        sizeof(header->sensor_id) - 1 - cur_len);
            }
        }
    }

    /* Close down access to the image data */
    ias_l1g_close_image(l1g_ptr);
    l1g_ptr = NULL;

    /* Done */
    return header;
}
