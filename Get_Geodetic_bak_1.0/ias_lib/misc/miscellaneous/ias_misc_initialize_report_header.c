/**********************************************************************
 NAME:              ias_misc_initialize_report_header

 PURPOSE:   Initializes the "standard" IAS report header information

 RETURNS:   Pointer to an IAS_REPORT_HEADER data structure if successful
            NULL pointer if an error has occurred
************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>              /* For basename() command */
#include "ias_const.h"
#include "ias_l0r.h"
#include "ias_logging.h"
#include "ias_miscellaneous.h"
#include "ias_miscellaneous_private.h"
#include "ias_structures.h"
#include "ias_types.h"

/*----------------------------------------------------------------------------
NAME:    ias_misc_initialize_report_header
-----------------------------------------------------------------------------*/
IAS_REPORT_HEADER *ias_misc_initialize_report_header
(
    const char *l0r_filename,       /* I: Name of L0R file */
    const char *work_order_id,      /* I: Work order ID string */
    const char *algorithm_name      /* I: Name of characterization algorithm */
)
{
    char local_l0r_filename[PATH_MAX];
    IAS_REPORT_HEADER *header = NULL;   /* Common header data structure */
    L0RIO *img_data = NULL;             /* Open L0R data file */
    IAS_L0R_INTERVAL_METADATA interval_metadata;  /* Complete set of L0R
                                                     interval metadata */
    IAS_L0R_SCENE_METADATA scene_metadata; /* Set of L0R metadata for a scene */
    int num_earth_scenes;               /* Number of earth scenes in an
                                           earth interval */
    int num_interval_records;           /* Total number of interval
                                           datasets in L0R data file */
    int status;                         /* Return status */
    const char *sw_version = NULL;      /* IAS software version */
    char *connection_str;               /* Database connection string */
    char *tmp_ptr;
    int length;

    /* Allocate a report header data block */
    header = calloc(1, sizeof(*header));
    if (header == NULL)
    {
        IAS_LOG_ERROR("Cannot allocate report header data block");
        return NULL;
    }

    /* Set the band and SCA numbers to -1. These are optional -- RPS Driver
       and GPS characterization routines can set these as needed; the off-line
       RPS characterization routines may or may not use these */
    header->band_number = -1;
    header->sca_number = -1;

    /* Initialize the work order ID and application name */
    strncpy(header->work_order_id, work_order_id,
            sizeof(header->work_order_id));
    header->work_order_id[sizeof(header->work_order_id) - 1] = '\0';

    strncpy(header->algorithm_name, algorithm_name,
            sizeof(header->algorithm_name));
    header->algorithm_name[sizeof(header->algorithm_name) - 1] = '\0';

    /* Initialize the processing system and IAS software version */
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

    /* If the name string is empty or NULL, don't go any further;
       just return what may have already been initialized */
    if ((l0r_filename == NULL) || (strcmp(l0r_filename, "") == 0))
    {
        /* Do not output a warning message since some applications do this
           deliberately */
        return header;
    }

    /* Open the L0R file. The ias_l0r_open_from_filename function uses
       the full L0R filename, assuming file name given consists of a path
       plus a base name. */
    img_data = ias_l0r_open_from_filename(l0r_filename, IAS_L0R_COMPRESSION_ON);
    if (img_data == NULL)
    {
        IAS_LOG_WARNING("Unable to open L0R file %s. Report header initialized "
                "without the L0R file. The header will not contain any "
                "specific scene or interval metadata.", l0r_filename);
        return header;
    }

    /* Open the L0R metadata file for reading */
    status = ias_l0r_open_metadata(img_data, IAS_READ);
    if (status != SUCCESS)
    {
        IAS_LOG_WARNING("Cannot open metadata for reading for L0R file %s. "
                "Report header initialized without the L0R file. The header "
                "will not contain any specific scene or interval metadata",
                l0r_filename);
        ias_l0r_close(img_data);
        return header;
    }

    /* Get the number of interval datasets present in the L0R data file */
    status = ias_l0r_get_interval_metadata_records_count(img_data,
            &num_interval_records);
    if ((status != SUCCESS) || (num_interval_records != 1))
    {
        IAS_LOG_WARNING("Unexpected number of interval records in L0R file %s. "
                "Report header initialized without the L0R file. The header "
                "will not contain any specific scene or interval metadata.",
                l0r_filename);
        ias_l0r_close_metadata(img_data);
        ias_l0r_close(img_data);
        return header;
    }

    /* Read the interval metadata */
    memset(&interval_metadata, 0, sizeof(IAS_L0R_INTERVAL_METADATA));
    status = ias_l0r_get_interval_metadata(img_data, &interval_metadata);
    if (status != SUCCESS)
    {
        IAS_LOG_WARNING("Cannot read L0R interval metadata from %s. Report "
                "header initialized without the L0R file. The header will "
                "not contain any specific scene or interval metadata.",
                l0r_filename);
        ias_l0r_close_metadata(img_data);
        ias_l0r_close(img_data);
        return header;
    }

    /* Now that we were able to open and get data from the L0R, copy the
       filename to the report header to indicate it was used in setting up
       the report header. */

    /* Since the basename() command needs a non-constant pointer, make a
       local copy before using it */
    strncpy(local_l0r_filename, l0r_filename, sizeof(local_l0r_filename));
    local_l0r_filename[sizeof(local_l0r_filename) - 1] = '\0';

    strncpy(header->l0r_filename, basename(local_l0r_filename),
        sizeof(header->l0r_filename));
    header->l0r_filename[sizeof(header->l0r_filename) - 1] = '\0';

    /* Get the collection type */
    strncpy(header->collection_type, interval_metadata.collection_type,
            sizeof(header->collection_type));
    header->collection_type[sizeof(header->collection_type) - 1] = '\0';

    /* Get the sensor/spacecraft ID information */
    strncpy(header->spacecraft_id, interval_metadata.spacecraft_id,
            sizeof(header->spacecraft_id));
    header->spacecraft_id[sizeof(header->spacecraft_id) - 1] = '\0';
    strncpy(header->sensor_id, interval_metadata.sensor_id,
            sizeof(header->sensor_id));
    header->sensor_id[sizeof(header->sensor_id) - 1] = '\0';

    /* Get the interval path/row information whether this is an Earth
       scene or a calibration data interval */
    header->starting_path = interval_metadata.wrs_starting_path;
    header->starting_row  = interval_metadata.wrs_starting_row;
    header->ending_row = interval_metadata.wrs_ending_row;

    /* Get the current interval version number */
    header->interval_version = interval_metadata.interval_version;

    /* Get the ingest software version used to process this L0R interval */
    strncpy(header->ingest_software_version, interval_metadata.is_version,
            sizeof(header->ingest_software_version));
    header->ingest_software_version[sizeof(header->ingest_software_version) - 1]
        = '\0';

    /* Get the ancillary start and stop times */
    strncpy(header->ancillary_start_time,
            interval_metadata.ancillary_start_time,
            sizeof(header->ancillary_start_time));
    header->ancillary_start_time[sizeof(header->ancillary_start_time) - 1]
        = '\0';
    strncpy(header->ancillary_stop_time,
            interval_metadata.ancillary_stop_time,
            sizeof(header->ancillary_stop_time));
    header->ancillary_stop_time[sizeof(header->ancillary_stop_time) - 1] = '\0';

    /* For all calibration data products, get the date the interval
       was acquired. If this is an Earth scene we're dealing with,
       this information will be overwritten. At a bare minimum,
       we'll at least have the information for the interval... */
    strncpy(header->date_acquired, interval_metadata.date_acquired,
            sizeof(header->date_acquired));
    header->date_acquired[sizeof(header->date_acquired) - 1] = '\0';

    /* Check to see if this is an Earth image or calibration data interval.
       If it's an individual Earth scene get some of the information from
       the scene metadata that wouldn't be applicable for calibration data. */
    if (strcmp(header->collection_type, "EARTH_IMAGING") == 0)
    {
        /* It is an individual Earth scene so flag it as such */
        header->is_earth_scene = 1;

        /* Save the WRS type information and interval ID before trying to
           gain access to the scene level metadata */
        header->wrs_type = interval_metadata.wrs_type;
        strncpy(header->landsat_interval_id,
                interval_metadata.landsat_interval_id,
                sizeof(header->landsat_interval_id));
        header->landsat_interval_id[sizeof(header->landsat_interval_id) - 1]
            = '\0';

        /* Get the count for the number of scenes in the interval.
           If this is an individual scene, it should be one. */
        status = ias_l0r_get_scene_metadata_records_count(img_data,
                &num_earth_scenes);
        if (status != SUCCESS)
        {
            IAS_LOG_WARNING("Cannot obtain number of scenes in interval from "
                    "L0R file %s. Report header will not contain any specific "
                    "scene metadata.", l0r_filename);
            ias_l0r_close_metadata(img_data);
            ias_l0r_close(img_data);
            return header;
        }
        if (num_earth_scenes != 1)
        {
            IAS_LOG_WARNING("Invalid scene count %d in scene metadata of L0R "
                    "file %s. Report header will not contain any specific "
                    "scene metadata.", num_earth_scenes, l0r_filename);
            ias_l0r_close_metadata(img_data);
            ias_l0r_close(img_data);
            return header;
        }

        /* Read the metadata for the scene */
        status = ias_l0r_get_scene_metadata(img_data, 0, 1, &scene_metadata);
        if (status != SUCCESS)
        {
            IAS_LOG_WARNING("Cannot read scene metadata in L0R file %s. Report "
                    "header will not contain any specific scene metadata.",
                    l0r_filename);
            ias_l0r_close_metadata(img_data);
            ias_l0r_close(img_data);
            return header;
        }

        /* Get the (WRS) scene number within the interval */
        header->scene_number = scene_metadata.wrs_scene_number;

        /* Get the subsetter version information */
        strncpy(header->l0rp_subsetter_software_version,
                scene_metadata.subsetter_version_l0rp,
                sizeof(header->l0rp_subsetter_software_version));
        header->l0rp_subsetter_software_version
            [sizeof(header->l0rp_subsetter_software_version) - 1] = '\0';

        /* Get the nominal and target path/row information for the scene */
        header->target_path = scene_metadata.target_wrs_path;
        header->target_row = scene_metadata.target_wrs_row;
        header->path = scene_metadata.wrs_path;
        header->row = scene_metadata.wrs_row;

        /* Get the scene ID */
        strncpy(header->landsat_scene_id, scene_metadata.landsat_scene_id,
                sizeof(header->landsat_scene_id));
        header->landsat_scene_id[sizeof(header->landsat_scene_id) - 1] = '\0';

        /* Get the acquisition date */
        strncpy(header->date_acquired, scene_metadata.date_acquired,
                sizeof(header->date_acquired));
        header->date_acquired[sizeof(header->date_acquired) - 1] = '\0';
    }
    else
    {
        /* It's a calibration interval, so save the name of the calibration
           interval ID */
        strncpy(header->landsat_interval_id,
                interval_metadata.landsat_cal_interval_id,
                sizeof(header->landsat_interval_id));
        header->landsat_interval_id[sizeof(header->landsat_interval_id) - 1]
            = '\0';

        /* Set the starting and ending path/row information to 0. The
           L0Ra DFCB indicates this value for calibration data collects. */
        header->starting_path = 0;
        header->starting_row = 0;
        header->ending_row = 0;
    }

    /* Close down access to the image data */
    ias_l0r_close_metadata(img_data);
    ias_l0r_close(img_data);

    /* Done */
    return header;
}
