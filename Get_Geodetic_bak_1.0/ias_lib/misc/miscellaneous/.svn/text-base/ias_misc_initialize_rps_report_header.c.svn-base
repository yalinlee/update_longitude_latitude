/**************************************************************************
 NAME:         ias_misc_initialize_rps_report_header

 PURPOSE:  If available, reads the OLI/TIRS acquisition times,
           OLI/TIRS image header/frame header, and OLI/TIRS
           telemetry information from the specified L0R data file.
           The information extracted from these data sources is
           considered to be of use to radiometric analysts

 RETURNS:  Integer status code of SUCCESS or ERROR
***************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "ias_l0r.h"
#include "ias_logging.h"
#include "ias_miscellaneous.h"
#include "ias_math.h"
#include "ias_types.h"
#include "ias_const.h"

int ias_misc_initialize_rps_report_header
(
    const char *l0r_filename,       /* I: Name of L0R file */
    RPS_REPORT_HEADER *rps_header   /* O: Populated RPS_REPORT_HEADER
                                          data structure */
)
{
    L0RIO *img_data = NULL;                     /* L0R dataset handle */
    IAS_L0R_INTERVAL_METADATA interval_metadata;/* L0R interval metadata */
    IAS_L0R_SCENE_METADATA scene_metadata;      /* L0R scene metadata */
    IAS_L0R_OLI_IMAGE_HEADER oli_ih;            /* Image header group data */
    IAS_DATETIME start_time;           /* Image start time broken down to
                                          constituent components */
    IAS_DATETIME stop_time;            /* Image stop time broken down to
                                          constituent components */
    double time1[3];                   /* Time 0: year, 1: DOY, 2: SOD */
    double time2[3];                   /* Time 0: year, 1: DOY, 2: SOD */
    int num_image_header_records;      /* Number of OLI image headers in an 
                                          L0R dataset */
    int num_earth_scenes;              /* Number of earth scenes in an
                                          earth interval */
    int num_interval_records;          /* Total number of interval
                                          datasets in L0R data file */
    int num_telemetry_records;
    int status;                                 /* Return status */
    int starttime_parse_status;                 /* Return status from parsing
                                                   start time string */
    int stoptime_parse_status;                  /* Return status from parsing
                                                   stop time status */
    double total_acquisition_time;              /* Local variable for elapsed
                                                   time calculations */

    /* Initialize the L0R data structures with 0's */
    memset(&interval_metadata, 0, sizeof(IAS_L0R_INTERVAL_METADATA));
    memset(&scene_metadata, 0, sizeof(IAS_L0R_SCENE_METADATA));
    memset(&oli_ih, 0, sizeof(IAS_L0R_OLI_IMAGE_HEADER));

    /* Initialize the RPS header data structure with 0's */
    memset(rps_header, 0, sizeof(RPS_REPORT_HEADER));

    /* Open the L0R file. The ias_l0r_open_from_filename function uses
       the full L0R filename, assuming file name given consists of a path
       plus a base name. */
    img_data = ias_l0r_open_from_filename(l0r_filename,
        IAS_L0R_COMPRESSION_ON);
    if (img_data == NULL)
    {
        IAS_LOG_WARNING("Unable to open L0R file %s. RPS header "
                "initialization cannot be completed.", l0r_filename);
        return SUCCESS;
    }

    /* Open the L0R metadata file for reading */
    status = ias_l0r_open_metadata(img_data, IAS_READ);
    if (status != SUCCESS)
    {
        IAS_LOG_WARNING("Cannot open metadata for reading for L0R file %s. "
                "RPS header initialization cannot be completed.",
                l0r_filename);
        ias_l0r_close(img_data);
        return SUCCESS;
    }

    /* Get the number of interval datasets present in the L0R data file */
    status = ias_l0r_get_interval_metadata_records_count(img_data,
            &num_interval_records);
    if ((status != SUCCESS) || (num_interval_records != 1))
    {
        IAS_LOG_ERROR("Unexpected number of interval records in L0R file %s. "
                "RPS header initialization cannot be completed.", l0r_filename);
        ias_l0r_close_metadata(img_data);
        ias_l0r_close(img_data);
        return ERROR;
    }

    /* Read the interval metadata */
    status = ias_l0r_get_interval_metadata(img_data, &interval_metadata);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Cannot read L0R interval metadata from %s. RPS header "
                "initialization cannot be completed.", l0r_filename);
        ias_l0r_close_metadata(img_data);
        ias_l0r_close(img_data);
        return ERROR;
    }

    /* Read the scene metadata if the interval is an Earth collect */
    if (strcmp(interval_metadata.collection_type, "EARTH_IMAGING") == 0)
    {
        status = ias_l0r_get_scene_metadata_records_count(img_data,
                &num_earth_scenes);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Cannot obtain number of scenes in interval from "
                "L0R file %s. RPS header initialization cannot be "
                "completed.", l0r_filename);
            ias_l0r_close_metadata(img_data);
            ias_l0r_close(img_data);
            return ERROR;
        }
        if (num_earth_scenes != 1)
        {
            IAS_LOG_ERROR("Invalid scene count %d in scene metadata of L0R "
                    "file %s. RPS header initialization cannot be completed.",
                    num_earth_scenes, l0r_filename);
            ias_l0r_close_metadata(img_data);
            ias_l0r_close(img_data);
            return ERROR;
        }
        status = ias_l0r_get_scene_metadata(img_data, 0, 1, &scene_metadata);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Cannot read L0R scene metadata in L0R file %s. "
                "RPS header initialization cannot be completed.",
                l0r_filename);
            ias_l0r_close_metadata(img_data);
            ias_l0r_close(img_data);
            return ERROR;
        }
    }

    /* Close the metadata */
    ias_l0r_close_metadata(img_data);

    /* Open the L0R header data for reading */
    status = ias_l0r_open_header(img_data, IAS_READ);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Cannot prepare image header for reading from L0R "
            "file %s. RPS header initialization cannot be completed.",
            l0r_filename);
        ias_l0r_close(img_data);
        return ERROR;
    }

    /* Open the ancillary data */
    status = ias_l0r_open_ancillary(img_data, IAS_READ);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Cannot prepare ancillary data for reading from "
            "L0R %s. RPS header initialization cannot be completed.",
            l0r_filename);
        ias_l0r_close_header(img_data);
        ias_l0r_close(img_data);
        return ERROR;
    }

    if ((strcmp(interval_metadata.sensor_id, "OLI") == 0)
            || (strcmp(interval_metadata.sensor_id, "OLI_TIRS") == 0))
    {

        /* Make sure the header exists */
        status = ias_l0r_is_oli_image_header_present(img_data,
                &num_image_header_records);
        if ((status != SUCCESS) || (num_image_header_records <= 0))
        {
            IAS_LOG_ERROR("Cannot obtain reliable number of OLI image header "
                "records from L0R %s. RPS header initialization cannot be "
                "completed.", l0r_filename);
            ias_l0r_close_header(img_data);
            ias_l0r_close(img_data);
            return ERROR;
        }

        /* Read the image header */
        status = ias_l0r_get_oli_image_header(img_data, &oli_ih);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Cannot read OLI image header from L0R %s. RPS "
                    "header initialization cannot be completed.",
                    l0r_filename);
            ias_l0r_close_header(img_data);
            ias_l0r_close(img_data);
            return ERROR;
        }
        /* Get the total number of OLI telemetry records */
        status = ias_l0r_get_ancillary_oli_telemetry_records_count(img_data,
                &num_telemetry_records);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Cannot obtain number of ancillary OLI telemetry "
                    "records in L0R %s. RPS header initialization cannot be "
                    "completed.", l0r_filename);
            ias_l0r_close_ancillary(img_data);
            ias_l0r_close_header(img_data);
            ias_l0r_close(img_data);
            return ERROR;
        }

        /* Allocate memory for the OLI telemetry data buffer if there are
           any telemetry records */
        if (num_telemetry_records > 0)
        {
            IAS_L0R_OLI_TELEMETRY *oli_telem;   /* OLI telemetry data */

            oli_telem = calloc(num_telemetry_records,
                    sizeof(IAS_L0R_OLI_TELEMETRY));
            if (oli_telem == NULL)
            {
                IAS_LOG_ERROR("Unable to allocate memory for OLI telemetry. "
                        "RPS header initialization cannot be completed.");
                ias_l0r_close_ancillary(img_data);
                ias_l0r_close_header(img_data);
                ias_l0r_close(img_data);
                return ERROR;
            }

            /* Get all of the available OLI telemetry records */
            status = ias_l0r_get_ancillary_oli_telemetry(img_data, 0,
                    num_telemetry_records, oli_telem);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Cannot obtain OLI ancillary telemetry group "
                        "data from L0R %s. RPS header initialization cannot "
                        "be completed.", l0r_filename);
                ias_l0r_close_ancillary(img_data);
                ias_l0r_close_header(img_data);
                ias_l0r_close(img_data);
                free(oli_telem);
                return ERROR;
            }

            /* Get the selected OLI temperatures at the start and end of the
               data collect. These are the temperatures that Cal/Val has
               expressed the most interest in initially; if other temperatures
               are desired, add them here. */
            rps_header->oli_fpm7_start_temp =
                  oli_telem[0].oli_telemetry_header_3.fpm_7_temp_celsius;
            rps_header->oli_fpm7_stop_temp =
                oli_telem[num_telemetry_records - 1].oli_telemetry_header_3
                .fpm_7_temp_celsius;

            rps_header->oli_fpm14_start_temp =
                oli_telem[0].oli_telemetry_header_3.fpm_14_temp_celsius;
            rps_header->oli_fpm14_stop_temp =
                oli_telem[num_telemetry_records - 1].oli_telemetry_header_3
                .fpm_14_temp_celsius;

            rps_header->oli_fpa_window_start_temp =
                oli_telem[0].oli_telemetry_header_3.fp_window_temp_celsius;
            rps_header->oli_fpa_window_stop_temp =
                oli_telem[num_telemetry_records - 1].oli_telemetry_header_3
                .fp_window_temp_celsius;

            rps_header->oli_fpe_chassis_start_temp =
                oli_telem[0].oli_telemetry_header_3.fp_chassis_temp_celsius;
            rps_header->oli_fpe_chassis_stop_temp =
                oli_telem[num_telemetry_records - 1].oli_telemetry_header_3
                .fp_chassis_temp_celsius;

            free(oli_telem);
        }
    }

    /* Set the image header information */
    if ((strcmp(interval_metadata.sensor_id, "OLI") == 0)
            || (strcmp(interval_metadata.sensor_id, "OLI_TIRS") == 0))
    {
        rps_header->ms_integration_time = (int)oli_ih.ms_integration_time;
        rps_header->pan_integration_time = (int)oli_ih.pan_integration_time;
        rps_header->current_detector_select_table
            = (int)oli_ih.current_detector_select_table;
        rps_header->image_data_truncation_setting
            = (int)oli_ih.image_data_truncation_setting;

        /* Extract the required RPS header information for an OLI
           scene/interval */
        if (strcmp(interval_metadata.collection_type, "EARTH_IMAGING") == 0)
        {
            /* Get the scene OLI image start/stop times for the particular
               sensor */
            strncpy(rps_header->acq_datetime.start_time_oli,
                    scene_metadata.start_time,
                    sizeof(rps_header->acq_datetime.start_time_oli));
            strncpy(rps_header->acq_datetime.stop_time_oli,
                    scene_metadata.stop_time,
                    sizeof(rps_header->acq_datetime.stop_time_oli));
        }
        else
        {
            /* Get the interval OLI image start/stop times */
            strncpy(rps_header->acq_datetime.start_time_oli,
                    interval_metadata.start_time_oli,
                    sizeof(rps_header->acq_datetime.start_time_oli));
            strncpy(rps_header->acq_datetime.stop_time_oli,
                    interval_metadata.stop_time_oli,
                    sizeof(rps_header->acq_datetime.stop_time_oli));
        }
        rps_header->acq_datetime.start_time_oli
            [sizeof(rps_header->acq_datetime.start_time_oli) - 1] = '\0';
        rps_header->acq_datetime.stop_time_oli
            [sizeof(rps_header->acq_datetime.stop_time_oli) - 1] = '\0';

        /* Calculate the elapsed time of the collect in seconds */
        starttime_parse_status = ias_misc_parse_datetime_string(
                rps_header->acq_datetime.start_time_oli,
                IAS_DATETIME_L0R_FORMAT, &start_time);
        stoptime_parse_status = ias_misc_parse_datetime_string(
                rps_header->acq_datetime.stop_time_oli,
                IAS_DATETIME_L0R_FORMAT, &stop_time);
        if ((starttime_parse_status != SUCCESS)
                || (stoptime_parse_status != SUCCESS))
        {
            IAS_LOG_ERROR("Parsing OLI start/stop time information. RPS "
                "header initialization cannot be completed.");
            ias_l0r_close_ancillary(img_data);
            ias_l0r_close_header(img_data);
            ias_l0r_close(img_data);
            return ERROR;
        }

        /* Set up the start and stop time arrays for the math routine */
        time1[0] = start_time.year;
        time1[1] = start_time.day_of_year;
        time1[2] = (start_time.hour * 3600) + (start_time.minute * 60)
            + (start_time.second);
        time2[0] = stop_time.year;
        time2[1] = stop_time.day_of_year;
        time2[2] = (stop_time.hour * 3600) + (stop_time.minute * 60)
            + (stop_time.second);

        /* Determine the elapsed acquisition time */
        status = ias_math_get_time_difference(time1, time2,
                &total_acquisition_time);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Determining elapsed OLI acquisition duration.  "
                "RPS header initialization cannot be completed.");
            ias_l0r_close_ancillary(img_data);
            ias_l0r_close_header(img_data);
            ias_l0r_close(img_data);
            return ERROR;
        }
        rps_header->acq_datetime.total_time_oli =
            fabs(total_acquisition_time);

        /* Determine the average frame rate as the ratio of the number of
           frames to the difference in timestamps between the first
           and last image frame headers */
        IAS_L0R_OLI_FRAME_HEADER first_oli_image_frame_header;
        IAS_L0R_OLI_FRAME_HEADER last_oli_image_frame_header;
        int num_oli_frame_headers;

        /* a.  Get the frame header count */
        status = ias_l0r_get_oli_frame_headers_records_count(img_data,
            &num_oli_frame_headers);
        if ((status != SUCCESS) || (num_oli_frame_headers == 0))
        {
            IAS_LOG_ERROR("Retrieving number of OLI frame headers needed "
               "to determine average frame rate.  RPS header "
               "initialization cannot be completed.");
            ias_l0r_close_ancillary(img_data);
            ias_l0r_close_header(img_data);
            ias_l0r_close(img_data);
            return ERROR;
        }
        rps_header->num_oli_frames = num_oli_frame_headers;

        /* b.  Get the first and last frame headers themselves.  We need
               the information in the l0r_time members */
        memset(&first_oli_image_frame_header, 0,
            sizeof(IAS_L0R_OLI_FRAME_HEADER));
        status = ias_l0r_get_oli_frame_headers(img_data, 0, 1,
            &first_oli_image_frame_header);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Retrieving first OLI frame header needed "
               "to determine average frame rate.  RPS header "
               "initialization cannot be completed.");
            ias_l0r_close_ancillary(img_data);
            ias_l0r_close_header(img_data);
            ias_l0r_close(img_data);
            return ERROR;
        }
        memset(&last_oli_image_frame_header, 0,
            sizeof(IAS_L0R_OLI_FRAME_HEADER));
        status = ias_l0r_get_oli_frame_headers(img_data,
            (num_oli_frame_headers - 1), 1, &last_oli_image_frame_header);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Retrieving last OLI frame header needed "
               "to determine average frame rate.  RPS header "
               "initialization cannot be completed.");
            ias_l0r_close_ancillary(img_data);
            ias_l0r_close_header(img_data);
            ias_l0r_close(img_data);
            return ERROR;
        }

        /* c. Calculate the timestamp delta */
        double lastfirst_oli_timestamp_delta =
            IAS_L0R_CONVERT_TIME_TO_SECONDS_SINCE_J2000(
                last_oli_image_frame_header.l0r_time)
          - IAS_L0R_CONVERT_TIME_TO_SECONDS_SINCE_J2000(
                first_oli_image_frame_header.l0r_time);

        rps_header->average_oli_frame_rate = (int)(floor(
            (num_oli_frame_headers / lastfirst_oli_timestamp_delta)
                + 0.5));
    }

    /* Do a similar setup for TIRS if it's present */
    if ((strcmp(interval_metadata.sensor_id, "TIRS") == 0)
            || (strcmp(interval_metadata.sensor_id, "OLI_TIRS") == 0))
    {
        double lastfirst_tirs_timestamp_delta;

        /* Get the required RPS header information for a TIRS collect.
           The start and stop times should come from the L0R scene
           metadata if this is an Earth imaging scene */
        if (strcmp(interval_metadata.collection_type, "EARTH_IMAGING") == 0)
        {
            /* Get the start and stop times for the scene */
            strncpy(rps_header->acq_datetime.start_time_tirs,
                    scene_metadata.start_time,
                    sizeof(rps_header->acq_datetime.start_time_tirs));
            strncpy(rps_header->acq_datetime.stop_time_tirs,
                    scene_metadata.stop_time,
                    sizeof(rps_header->acq_datetime.stop_time_tirs));
        }
        else
        {
            /* Get the start and stop times for the interval */
            strncpy(rps_header->acq_datetime.start_time_tirs,
                    interval_metadata.start_time_tirs,
                    sizeof(rps_header->acq_datetime.start_time_tirs));
            strncpy(rps_header->acq_datetime.stop_time_tirs,
                    interval_metadata.stop_time_tirs,
                    sizeof(rps_header->acq_datetime.stop_time_tirs));
        }
        rps_header->acq_datetime.start_time_tirs
            [sizeof(rps_header->acq_datetime.start_time_tirs) - 1] = '\0';
        rps_header->acq_datetime.stop_time_tirs
            [sizeof(rps_header->acq_datetime.stop_time_tirs) - 1] = '\0';

        /* Calculate the elapsed time in seconds */
        starttime_parse_status = ias_misc_parse_datetime_string(
                rps_header->acq_datetime.start_time_tirs,
                IAS_DATETIME_L0R_FORMAT, &start_time);
        stoptime_parse_status = ias_misc_parse_datetime_string(
                rps_header->acq_datetime.stop_time_tirs,
                IAS_DATETIME_L0R_FORMAT, &stop_time);

        if ((starttime_parse_status != SUCCESS)
                || (stoptime_parse_status != SUCCESS))
        {
            IAS_LOG_ERROR("Parsing TIRS start/stop time information. RPS "
                    "header initialization cannot be completed.");
            ias_l0r_close_ancillary(img_data);
            ias_l0r_close_header(img_data);
            ias_l0r_close(img_data);
            return ERROR;
        }

        /* Set up the start and stop time arrays for the math routine */
        time1[0] = start_time.year;
        time1[1] = start_time.day_of_year;
        time1[2] = (start_time.hour * 3600) + (start_time.minute * 60)
            + (start_time.second);
        time2[0] = stop_time.year;
        time2[1] = stop_time.day_of_year;
        time2[2] = (stop_time.hour * 3600) + (stop_time.minute * 60)
            + (stop_time.second);

        /* Determine the elapsed acquisition time */
        status = ias_math_get_time_difference(time1, time2,
                &total_acquisition_time);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Determining elapsed TIRS acquisition duration. "
                "RPS header initialization cannot be completed.");
            ias_l0r_close_ancillary(img_data);
            ias_l0r_close_header(img_data);
            ias_l0r_close(img_data);
            return ERROR;
        }
        rps_header->acq_datetime.total_time_tirs =
            fabs(total_acquisition_time);

        /* Determine the average frame rate as the ratio of the number of
           frames to the difference in timestamps between the first
           and last image frame headers */
        IAS_L0R_TIRS_FRAME_HEADER first_tirs_image_frame_header;
        IAS_L0R_TIRS_FRAME_HEADER last_tirs_image_frame_header;
        int num_tirs_frame_headers;

        /* a.  Get the frame header count */
        status = ias_l0r_get_tirs_frame_headers_records_count(img_data,
            &num_tirs_frame_headers);
        if ((status != SUCCESS) || (num_tirs_frame_headers == 0))
        {
            IAS_LOG_ERROR("Retrieving number of TIRS frame headers needed "
               "to determine average frame rate.  RPS header "
               "initialization cannot be completed.");
            ias_l0r_close_ancillary(img_data);
            ias_l0r_close_header(img_data);
            ias_l0r_close(img_data);
            return ERROR;
        }
        rps_header->num_tirs_frames = num_tirs_frame_headers;

        /* b.  Get the first and last frame headers themselves.  We need
               the information in the l0r_time members, and the total
               number of frames requested in the first frame header
               (assuming it won't change) */
        memset(&first_tirs_image_frame_header, 0,
            sizeof(IAS_L0R_TIRS_FRAME_HEADER));
        status = ias_l0r_get_tirs_frame_headers(img_data, 0, 1,
            &first_tirs_image_frame_header);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Retrieving first TIRS frame header needed "
               "to determine average frame rate.  RPS header "
               "initialization cannot be completed.");
            ias_l0r_close_ancillary(img_data);
            ias_l0r_close_header(img_data);
            ias_l0r_close(img_data);
            return ERROR;
        }

        rps_header->num_tirs_frames =
            first_tirs_image_frame_header.total_frames_requested;

        memset(&last_tirs_image_frame_header, 0,
            sizeof(IAS_L0R_TIRS_FRAME_HEADER));
        status = ias_l0r_get_tirs_frame_headers(img_data,
            (num_tirs_frame_headers - 1), 1, &last_tirs_image_frame_header);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Retrieving last TIRS frame header needed "
               "to determine average frame rate.  RPS header "
               "initialization cannot be completed.");
            ias_l0r_close_ancillary(img_data);
            ias_l0r_close_header(img_data);
            ias_l0r_close(img_data);
            return ERROR;
        }

        /* c. Calculate the timestamp delta */
        lastfirst_tirs_timestamp_delta =
            IAS_L0R_CONVERT_TIME_TO_SECONDS_SINCE_J2000(
                last_tirs_image_frame_header.l0r_time)
          - IAS_L0R_CONVERT_TIME_TO_SECONDS_SINCE_J2000(
                first_tirs_image_frame_header.l0r_time);

        rps_header->average_tirs_frame_rate = (int)(floor(
            (num_tirs_frame_headers / lastfirst_tirs_timestamp_delta) + 0.5));
    }

    /* Close access to the open image groups and the image itself. */
    ias_l0r_close_ancillary(img_data);
    ias_l0r_close_header(img_data);
    ias_l0r_close(img_data);


    return SUCCESS;
}
