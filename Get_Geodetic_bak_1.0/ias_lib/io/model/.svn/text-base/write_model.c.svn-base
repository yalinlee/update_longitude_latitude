/*************************************************************************
NAME:    write_model contains the following routines:
             ias_model_write_table
             ias_model_write_frame_times
             ias_model_write_band_model
             ias_model_write_attitude_model
             ias_model_write_ephemeris_model
             ias_model_write_jitter_table
             ias_model_write_ssm_model
             write_time_header
             write_sca_dynamic_arrays
             write_band_sca_model
             write_attitude_sample_records
             write_ephemeris_sample_records

PURPOSE: Routines to support writing the model to an HDF5 file

RETURNS: SUCCESS or ERROR
**************************************************************************/
#include <string.h>
#include "ias_logging.h"
#include "common_model_io.h"

/* local defines */
#define DONOT_WRITE_SAMPLE_TIME 0
#define WRITE_SAMPLE_TIME 1

/* local functions */
static int write_time_header
(
    hid_t group_id,             /* I: group id to write time header info to */
    const double utc_epoch_time[3], /* I: epoch time to write */
    int write_sample_time,      /* I: flag to indicate the nominal sample time
                                      should be written */
    double nominal_sample_time  /* I: nominal sample time to write */
);
static int write_sca_dynamic_arrays
(
    hid_t group,                  /* I: An open HDF5 group handle */
    const IAS_SENSOR_SCA_MODEL *sca /* I: SCA structure with data to write */
);
static int write_band_sca_model
(
    hid_t file,                    /* I: An open HDF5 file handle */
    int band_to_write,             /* I: Which band working with (index) */
    const IAS_LOS_MODEL *los_model /* I: Model structure with data to write */
);
static int write_attitude_sample_records
(
    hid_t group,                  /* I: An open HDF5 group handle */
    const IAS_SC_ATTITUDE_MODEL *att /* I: Attitude struct with data to write */
);
static int write_ephemeris_sample_records
(
    hid_t group,                     /* I: An open HDF5 group handle */
    const IAS_SC_EPHEMERIS_MODEL *ephem /* I: ephemeris data to write */
);
static int write_ssm_records
(
    hid_t group,                  /* I: An open HDF5 group handle */
    const IAS_SENSOR_SCENE_SELECT_MIRROR_MODEL *ssm /* I: ssm data to write */
);

/*************************************************************************
NAME: ias_model_write_table

PURPOSE: Internal routine to write the indicated (param) model table

RETURNS: SUCCESS -- successfully wrote contents of structure to HDF table
         ERROR -- error writing to HDF file table
**************************************************************************/
int ias_model_write_table
(
    hid_t file,           /* I: An open HDF5 file handle */
    MODEL_TABLE table,    /* I: Flag indicating which table to read */
    int band_to_write,    /* I: For writing band or SCA tables, or -1 */
    int sca_to_write,     /* I: For writing SCA table, or -1 */
    const IAS_LOS_MODEL *los_model /* I: Model structure with data to write */
)
{
    size_t offsets[NFIELDS];          /* These arrays are used to     */
    const char *field_names[NFIELDS]; /*  set up the definition of    */
    char table_name[PATH_MAX];        /*  the table being read. They  */
    hid_t field_type[NFIELDS];        /*  will be filled by calls     */
    size_t field_sizes[NFIELDS];      /*  to the appropriate table    */
    hid_t fields_to_close[NFIELDS];   /*  definition build routine.   */
    int status = SUCCESS;             /* Return status of the description build
                                         routines */
    int return_status = SUCCESS;      /* Return status of this routine */
    int number_fields;                /* # of fields in the table description */
    size_t total_size = 0;            /* Total size of the table */
    GENERAL_MODEL model;              /* Subset of the LOS model structure
                                         containing members that go into a
                                         general table. */
    SENSOR_LOCATION_MODEL sensor_loc_model[IAS_MAX_SENSORS];
                                      /* Slightly modified version of the
                                         IAS_SENSOR_LOCATION_MODEL to save
                                         a flag for SSM model present*/
    BAND_SAMP_CHAR_MODEL band_model;  /* Subset of the LOS model band & sampling
                                         characteristics structures containing
                                         members that go into the band table. */
    SCA_MODEL sca_model;              /* Subset of the LOS model SCA structure
                                         containing members that go into the
                                         SCA table */
    const IAS_SENSOR_MODEL *sensor;   /* pointer to the sensor model */
    const void *the_struct = NULL;    /* Pointer to the structure to fill */
    hsize_t record_count = 0;
    int detectors;
    int i, j, k;                      /* General looping counters */

    sensor = &los_model->sensor;

    /* Build a description of the table data.  Also set a pointer to the
       model member structure, or general structure, and set the total
       size of the structure. */
    switch (table)
    {
        case GENERAL_TABLE :
            status = ias_model_build_general_description(offsets, field_names,
                    field_type, fields_to_close, field_sizes, &number_fields);
            strcpy(table_name, GENERAL_TABLE_NAME);

            /* Populate the general HDF model structure */
            memset(&model, 0, sizeof(model));
            model.satellite_id = los_model->satellite_id;
            strcpy(model.sw_version, los_model->sw_version);
            model.wrs_path = los_model->wrs_path;
            model.wrs_row = los_model->wrs_row;
            model.acquisition_type = los_model->acquisition_type;
            model.correction_type = los_model->correction_type;
            total_size = sizeof(GENERAL_MODEL);
            the_struct = &model;
            record_count = 1;
            break;
        case EARTH_TABLE :
            status = ias_model_build_earth_description(offsets, field_names,
                    field_type, fields_to_close, field_sizes, &number_fields);
            strcpy(table_name, EARTH_TABLE_NAME);
            total_size = sizeof(IAS_EARTH_CHARACTERISTICS);
            the_struct = &los_model->earth;
            record_count = 1;
            break;
        case SENSOR_TABLE :
            status = ias_model_build_sensor_description(offsets, field_names,
                    field_type, fields_to_close, field_sizes, &number_fields);
            strcpy(table_name, SENSOR_TABLE_NAME);

            memset(sensor_loc_model, 0, sizeof(sensor_loc_model));
            /* Populate the general HDF model structure */
            for (i = 0; i < IAS_MAX_SENSORS; i++)
            {
                sensor_loc_model[i].sensor_id = sensor->sensors[i].sensor_id;
                sensor_loc_model[i].sensor_present
                    = sensor->sensors[i].sensor_present;
                for (j = 0; j < TRANSFORMATION_MATRIX_DIM; j++)
                {
                    for (k = 0; k < TRANSFORMATION_MATRIX_DIM; k++)
                    {
                        sensor_loc_model[i].sensor2acs[j][k]
                            = sensor->sensors[i].sensor2acs[j][k];
                    }
                    /* Set the center of mass to sensor offsets */
                    sensor_loc_model[i].center_mass2sensor_offset[j]
                        = sensor->sensors[i].center_mass2sensor_offset[j];
                }

                /* set the ssm model present flag */
                if (sensor->sensors[i].ssm_model != NULL)
                    sensor_loc_model[i].ssm_model_present = 1;
                else
                    sensor_loc_model[i].ssm_model_present = 0;
            }
            total_size = sizeof(SENSOR_LOCATION_MODEL);
            the_struct = sensor_loc_model;
            record_count = IAS_MAX_SENSORS;
            break;
        case PRECISION_TABLE :
            status = ias_model_build_precision_description(offsets, field_names,
                    field_type, fields_to_close, field_sizes, &number_fields);
            strcpy(table_name, PRECISION_TABLE_NAME);
            total_size = sizeof(IAS_SC_PRECISION_MODEL);
            the_struct = &los_model->spacecraft.precision;
            record_count = 1;
            break;
        case BAND_TABLE :
            status = ias_model_build_band_description(offsets, field_names,
                    field_type, fields_to_close, field_sizes, &number_fields);
            strcpy(table_name, BAND_TABLE_NAME);

            /* Populate the band model structure */
            memset(&band_model, 0, sizeof(band_model));
            band_model.band_present = sensor->bands[band_to_write].band_present;
            band_model.sca_count = sensor->bands[band_to_write].sca_count;
            band_model.utc_epoch_time[0]
                = sensor->bands[band_to_write].utc_epoch_time[0];
            band_model.utc_epoch_time[1]
                = sensor->bands[band_to_write].utc_epoch_time[1];
            band_model.utc_epoch_time[2]
                = sensor->bands[band_to_write].utc_epoch_time[2];
            band_model.integration_time
                = sensor->bands[band_to_write].sampling_char.integration_time;
            band_model.sampling_time
                = sensor->bands[band_to_write].sampling_char.sampling_time;
            band_model.lines_per_frame
                = sensor->bands[band_to_write].sampling_char.lines_per_frame;
            band_model.settling_time
                = sensor->bands[band_to_write].sampling_char.settling_time;
            band_model.along_ifov
                = sensor->bands[band_to_write].sampling_char.along_ifov;
            band_model.across_ifov
                = sensor->bands[band_to_write].sampling_char.across_ifov;
            band_model.maximum_detector_delay = sensor->bands[band_to_write].
                sampling_char.maximum_detector_delay;
            band_model.time_codes_at_frame_start = sensor->bands[band_to_write].
                        sampling_char.time_codes_at_frame_start;
            band_model.frame_delay = sensor->bands[band_to_write].sampling_char.
                        frame_delay;
            total_size = sizeof(BAND_SAMP_CHAR_MODEL);
            record_count = 1;

            if (status == ERROR)
            {
                IAS_LOG_ERROR("Building table description for table %s, "
                        "band number %d", table_name, band_to_write);
                return_status = ERROR;
                break;
            }

            /* Write the table */
            if (H5TBmake_table(ROOT_TABLE_NAME, file, table_name, number_fields,
                        record_count, total_size, field_names, offsets,
                        field_type, 1, NULL, 0, &band_model) < 0)
            {
                IAS_LOG_ERROR("Writing band records for table %s, band number "
                        "%d", table_name, band_to_write);
                return_status = ERROR;
                break;
            }
            if (write_band_sca_model(file, band_to_write, los_model) != SUCCESS)
            {
                IAS_LOG_ERROR("Writing SCA model for band %d", band_to_write);
                return_status = ERROR;
                break;
            }
            break;
        case BAND_SCA_TABLE :
            detectors
                = sensor->bands[band_to_write].scas[sca_to_write].detectors;
            status = ias_model_build_band_sca_description(detectors, offsets,
                    field_names, field_type, fields_to_close, field_sizes,
                    &number_fields);
            strcpy(table_name, SCA_TABLE_NAME);

            /* Populate the SCA model structure */
            memset(&sca_model, 0, sizeof(sca_model));

            sca_model.nominal_fill
                = sensor->bands[band_to_write].scas[sca_to_write].nominal_fill;

            for (i = 0; i < IAS_LOS_LEGENDRE_TERMS; i++)
            {
                sca_model.sca_coef_x[i] = sensor->bands[band_to_write].
                    scas[sca_to_write].sca_coef_x[i];
                sca_model.sca_coef_y[i] = sensor->bands[band_to_write].
                    scas[sca_to_write].sca_coef_y[i];
            }
            total_size = sizeof(SCA_MODEL);
            the_struct = &sca_model;
            record_count = 1;
            break;
        default :
            IAS_LOG_ERROR("Unexpected table type");
            return ERROR;
    }
    if (status == ERROR || return_status == ERROR)
    {
        IAS_LOG_ERROR("Preparing for write or writing table %s", table_name);
        ias_model_close_the_fields(fields_to_close);
        return ERROR;
    }

    /* Write the table (already wrote all band tables) */
    if (table != BAND_TABLE)
    {
        if (H5TBmake_table(ROOT_TABLE_NAME, file, table_name, number_fields,
                    record_count, total_size, field_names, offsets, field_type,
                    1, NULL, 0, the_struct) < 0)
        {
            IAS_LOG_ERROR("Writing table %s", table_name);
            return_status = ERROR;
        }
    }

    ias_model_close_the_fields(fields_to_close);
    return return_status;
}

/*************************************************************************
NAME: ias_model_write_frame_times

PURPOSE: Internal routine to write the frame times from epoch to file

RETURNS: SUCCESS -- successfully wrote times to the HDF5 file
         ERROR -- error in writing to the HDF5 file
**************************************************************************/
int ias_model_write_frame_times
(
    hid_t file,                    /* I: An open HDF5 file handle */
    const IAS_LOS_MODEL *los_model /* I: Model structure with data to write */
)
{
    hid_t space;            /* Identifier for the data space */
    hid_t dataset;          /* Identifier for the dataset */
    hsize_t dim[1];         /* Array dimension */
    int status;             /* Return status of the HDF write routine */
    char dataset_name[256];
    int sensor_index;
    const IAS_SENSOR_MODEL *sensor = &los_model->sensor;
    const char *sensor_name;

    for (sensor_index = 0; sensor_index < IAS_MAX_SENSORS; sensor_index++)
    {
        /* skip entries that are not set */
        if (!sensor->frame_seconds_from_epoch[sensor_index])
        {
            /* If the frame seconds from epoch for this sensor have not been
               set yet (pointer is NULL) but the model contains flag that
               this sensor and its data should be present in the model, then
               we have an unexpected error situation. */
            if (sensor->sensors[sensor_index].sensor_present)
            {
                IAS_LOG_ERROR("Sensor %d should be present, but frame times "
                              "missing", sensor_index);
                return ERROR;
            }
            else
            {
                continue;
            }
        }
        if (!sensor->sensors[sensor_index].sensor_present)
            continue;

        /* Create a simple data space */
        dim[0] = sensor->frame_counts[sensor_index];
        space = H5Screate_simple(1, dim, NULL);
        if (space < 0)
        {
            IAS_LOG_ERROR("Creating data space for model time from epoch for "
                    "sensor index %d", sensor_index);
            return ERROR;
        }

        sensor_name = ias_sat_attr_get_sensor_name(sensor_index);
        if (strcmp(sensor_name, "Unknown") == 0)
        {
            IAS_LOG_ERROR("Getting sensor name for sensor index %d",
                    sensor_index);
            H5Sclose(space);
            return ERROR;
        }

        /* build the dataset name from the sensor name and the base name */
        snprintf(dataset_name, sizeof(dataset_name), "%s %s", sensor_name,
                FRAME_TIMES_RECORD_NAME);

        /* Define and create a dataset */
        dataset = H5Dcreate(file, dataset_name, H5T_NATIVE_DOUBLE, space,
                H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if (dataset < 0)
        {
            IAS_LOG_ERROR("Creating data set for model time from epoch "
                    "dataset %s", dataset_name);
            H5Sclose(space);
            return ERROR;
        }

        /* Write out the data */
        status = H5Dwrite(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
                    H5P_DEFAULT,
                    sensor->frame_seconds_from_epoch[sensor_index]);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing the frame times from epoch for %s",
                    dataset_name);
            H5Dclose(dataset);
            H5Sclose(space);
            return ERROR;
        }

        status = H5Dclose(dataset);
        if (status < 0)
        {
            IAS_LOG_ERROR("Closing data set for model time from epoch "
                    "dataset %s", dataset_name);
            H5Sclose(space);
            return ERROR;
        }
        status = H5Sclose(space);
        if (status < 0)
        {
            IAS_LOG_ERROR("Closing data space for model time from epoch for "
                    "sensor index %d", sensor_index);
            return ERROR;
        }
    }
    return SUCCESS;
}

/*************************************************************************
NAME: ias_model_write_band_model

PURPOSE: Internal routine to write the contents of the band model
         structure to the HDF5 file

RETURNS: SUCCESS -- successfully wrote band data to the HDF5 file
         ERROR -- error in writing to the HDF5 file
**************************************************************************/
int ias_model_write_band_model
(
    hid_t file,                    /* I: An open HDF5 file handle */
    const IAS_LOS_MODEL *los_model /* I: Model structure with data to write */
)
{
    hid_t group_id;
    char band_model_group_name[20];
    const IAS_SENSOR_MODEL *sensor = &los_model->sensor;
    int band_index;
    int band_number;

    /* create new band group for each band */
    for (band_index = 0; band_index < sensor->band_count; band_index++)
    {
        band_number = ias_sat_attr_convert_band_index_to_number(band_index);
        if (band_number == ERROR)
        {
            IAS_LOG_ERROR("Converting band index %d to band number",
                    band_index);
            return ERROR;
        }
        sprintf(band_model_group_name, "Band %02d", band_number);

        group_id = H5Gcreate(file, band_model_group_name, H5P_DEFAULT, 
                H5P_DEFAULT, H5P_DEFAULT);
        if (group_id < 0) 
        { 
            IAS_LOG_ERROR("Opening band group %s", band_model_group_name);
            return ERROR; 
        } 

        if (ias_model_write_table(group_id, BAND_TABLE, band_index, -1,
                    los_model) != SUCCESS)
        {
            IAS_LOG_ERROR("Writing model band group %s", band_model_group_name);
            H5Gclose(group_id);
            return ERROR;
        }

        /* close the group */
        if (H5Gclose(group_id) < 0)
        {
            IAS_LOG_ERROR("Closing band group %s", band_model_group_name);
            return ERROR;
        }
    }

    return SUCCESS;
}

/*************************************************************************
NAME: ias_model_write_attitude_model

PURPOSE: Internal routine to write the contents of the attitude model
         structure to the HDF5 file

RETURNS: SUCCESS -- successfully wrote attitude data to the HDF5 file
         ERROR -- error in writing to the HDF5 file
**************************************************************************/
int ias_model_write_attitude_model
(
    hid_t file,                         /* I: An open HDF5 file handle */
    const IAS_SC_ATTITUDE_MODEL *att    /* I: Attitude data to write */
)
{
    hid_t group_id;

    /* create the attitude group */
    group_id = H5Gcreate(file, ATTITUDE_MODEL_GROUP_NAME, H5P_DEFAULT, 
            H5P_DEFAULT, H5P_DEFAULT);
    if (group_id < 0) 
    { 
        IAS_LOG_ERROR("Opening attitude group");
        return ERROR; 
    } 

    /* write the time header info to the group */
    if (write_time_header(group_id, att->utc_epoch_time, WRITE_SAMPLE_TIME,
                att->nominal_sample_time) != SUCCESS)
    {
        IAS_LOG_ERROR("Writing attitude model time header");
        H5Gclose(group_id);
        return ERROR; 
    }

    /* write the attitude sample records */
    if (write_attitude_sample_records(group_id, att) != SUCCESS)
    {
        IAS_LOG_ERROR("Writing attitude model sample records");
        H5Gclose(group_id);
        return ERROR;
    }

    /* close the group */
    if (H5Gclose(group_id) < 0)
    {
        IAS_LOG_ERROR("Closing attitude group");
        return ERROR;
    }

    return SUCCESS;
}

/*************************************************************************
NAME: ias_model_write_ephemeris_model

PURPOSE: Internal routine to write the contents of the ephemeris model
         structure to the HDF5 file

RETURNS: SUCCESS -- successfully wrote ephemeris data to the HDF5 file
         ERROR -- error in writing to the HDF5 file
**************************************************************************/
int ias_model_write_ephemeris_model
(
    hid_t file,                         /* I: An open HDF5 file handle */
    const IAS_SC_EPHEMERIS_MODEL *ephem /* I: Ephemeris data to write */
)
{
    hid_t group_id;

    /* create the ephemeris group */
    group_id = H5Gcreate(file, EPHEMERIS_MODEL_GROUP_NAME, H5P_DEFAULT, 
            H5P_DEFAULT, H5P_DEFAULT);
    if (group_id < 0) 
    { 
        IAS_LOG_ERROR("Opening ephemeris group");
        return ERROR; 
    } 

    /* write the time header info to the group */
    if (write_time_header(group_id, ephem->utc_epoch_time, WRITE_SAMPLE_TIME,
                ephem->nominal_sample_time) != SUCCESS)
    {
        IAS_LOG_ERROR("Writing ephemeris model time header");
        H5Gclose(group_id);
        return ERROR; 
    }

    /* write the ephemeris sample records */
    if (write_ephemeris_sample_records(group_id, ephem) != SUCCESS)
    {
        IAS_LOG_ERROR("Writing ephemeris model sample records");
        H5Gclose(group_id);
        return ERROR;
    }

    /* close the group */
    if (H5Gclose(group_id) < 0)
    {
        IAS_LOG_ERROR("Closing ephemeris group");
        return ERROR;
    }

    return SUCCESS;
}

/*************************************************************************
NAME: write_time_header

PURPOSE: Internal routine to write the UTC epoch time and nominal sample
         time to the HDF5 file

RETURNS: SUCCESS -- successfully wrote times to the HDF5 file
         ERROR -- error in writing to the HDF5 file
**************************************************************************/
static int write_time_header
(
    hid_t group_id,             /* I: group id to write time header info to */
    const double utc_epoch_time[3], /* I: epoch time to write */
    int write_sample_time,      /* I: flag to indicate the nominal sample time
                                      should be written */
    double nominal_sample_time  /* I: nominal sample time to write */
)
{
    hsize_t dims[1];

    /* write the UTC epoch time */
    dims[0] = 3;
    if (H5LTmake_dataset_double(group_id, UTC_EPOCH_TIME, 1, dims,
                utc_epoch_time) < 0)
    {
        IAS_LOG_ERROR("Writing UTC epoch time");
        return ERROR;
    }

    if (write_sample_time == WRITE_SAMPLE_TIME)
    {
        /* write the nominal sample time */
        dims[0] = 1;
        if (H5LTmake_dataset_double(group_id, NOMINAL_SAMPLE_TIME, 1, dims,
                    &nominal_sample_time) < 0)
        {
            IAS_LOG_ERROR("Writing nominal sample time");
            return ERROR;
        }
    }

    return SUCCESS;
}

/*************************************************************************

NAME: write_sca_dynamic_arrays

PURPOSE: Internal routine to write the SCA dynamic arrays to the HDF5 file

RETURNS: SUCCESS -- successfully wrote the arrays to the file
         ERROR -- error in writing to the file
**************************************************************************/
static int write_sca_dynamic_arrays
(
    hid_t group,                  /* I: An open HDF5 group handle */
    const IAS_SENSOR_SCA_MODEL *sca /* I: SCA structure with data to write */
)
{
    hid_t space;        /* Identifier for the data space */
    hid_t dataset;      /* Identifier for the dataset */
    hsize_t dim[1];     /* Number of column dimensions in array */
    int status;         /* Return status of the HDF read routine */

    /*---------------L0R Fill-----------------*/
    /* Create a simple data space */
    dim[0] = sca->detectors;
    space = H5Screate_simple(1, dim, NULL);
    if (space < 0)
    {
        IAS_LOG_ERROR("Creating data space for model SCA dynamic array");
        return ERROR;
    }

    /* Define and create a dataset */
    dataset = H5Dcreate(group, SCA_L0R_FILL_RECORD_NAME, H5T_NATIVE_INT, space,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dataset < 0)
    {
        IAS_LOG_ERROR("Creating data set for model SCA dynamic array");
        H5Sclose(space);
        return ERROR;
    }

    /* Write out the data */
    status = H5Dwrite(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
            sca->l0r_detector_offsets);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing the image model SCA dynamic array");
        H5Dclose(dataset);
        H5Sclose(space);
        return ERROR;
    }

    status = H5Dclose(dataset);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing data set for model SCA dynamic array");
        H5Sclose(space);
        return ERROR;
    }
    status = H5Sclose(space);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing data space for model SCA dynamic array");
        return ERROR;
    }

    /*---------------Along-track Detector Offsets-----------------*/
    /* Create a simple data space */
    dim[0] = sca->detectors;
    space = H5Screate_simple(1, dim, NULL);
    if (space < 0)
    {
        IAS_LOG_ERROR("Creating data space for model SCA dynamic array");
        return ERROR;
    }

    /* Define and create a dataset */
    dataset = H5Dcreate(group, SCA_ALONG_DET_OFF_RECORD_NAME, H5T_NATIVE_DOUBLE,
            space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dataset < 0)
    {
        IAS_LOG_ERROR("Creating data set for model SCA dynamic array");
        H5Sclose(space);
        return ERROR;
    }

    /* Write out the data */
    status = H5Dwrite(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT,
            sca->detector_offsets_along_track);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing the image model SCA dynamic array");
        H5Dclose(dataset);
        H5Sclose(space);
        return ERROR;
    }

    status = H5Dclose(dataset);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing data set for model SCA dynamic array");
        H5Sclose(space);
        return ERROR;
    }
    status = H5Sclose(space);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing data space for model SCA dynamic array");
        return ERROR;
    }

    /*---------------Across-track Detector Offsets-----------------*/
    /* Create a simple data space */
    dim[0] = sca->detectors;
    space = H5Screate_simple(1, dim, NULL);
    if (space < 0)
    {
        IAS_LOG_ERROR("Creating data space for model SCA dynamic array");
        return ERROR;
    }

    /* Define and create a dataset */
    dataset = H5Dcreate(group, SCA_ACROSS_DET_OFF_RECORD_NAME,
            H5T_NATIVE_DOUBLE, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dataset < 0)
    {
        IAS_LOG_ERROR("Creating data set for model SCA dynamic array");
        H5Sclose(space);
        return ERROR;
    }

    /* Write out the data */
    status = H5Dwrite(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT,
            sca->detector_offsets_across_track);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing the image model SCA dynamic array");
        H5Dclose(dataset);
        H5Sclose(space);
        return ERROR;
    }

    status = H5Dclose(dataset);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing data set for model SCA dynamic array");
        H5Sclose(space);
        return ERROR;
    }
    status = H5Sclose(space);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing data space for model SCA dynamic array");
        return ERROR;
    }

    return SUCCESS;
}

/*************************************************************************
NAME: write_band_sca_model

PURPOSE: Internal routine to write the contents of the SCA model for the
    given band to the HDF5 file

RETURNS: SUCCESS -- successfully wrote band data to the HDF5 file
         ERROR -- error in writing to the HDF5 file
**************************************************************************/
static int write_band_sca_model
(
    hid_t file,                    /* I: An open HDF5 file handle */
    int band_to_write,             /* I: Which band working with (index) */
    const IAS_LOS_MODEL *los_model /* I: Model structure with data to write */
)
{
    hid_t group_id;
    char band_sca_model_group_name[20];
    const IAS_SENSOR_MODEL *sensor = &los_model->sensor;
    int sca_index;

    /* create new SCA group for each SCA */
    for (sca_index = 0; sca_index < sensor->bands[band_to_write].sca_count;
            sca_index++)
    {
        sprintf(band_sca_model_group_name, "SCA %02d", sca_index);

        group_id = H5Gcreate(file, band_sca_model_group_name, H5P_DEFAULT, 
                H5P_DEFAULT, H5P_DEFAULT);
        if (group_id < 0) 
        { 
            IAS_LOG_ERROR("Opening SCA group %s", band_sca_model_group_name);
            return ERROR; 
        } 

        if (ias_model_write_table(group_id, BAND_SCA_TABLE, band_to_write,
                    sca_index, los_model) != SUCCESS)
        {
            IAS_LOG_ERROR("Writing model SCA group %s",
                    band_sca_model_group_name);
            H5Gclose(group_id);
            return ERROR;
        }

        if (write_sca_dynamic_arrays(group_id,
                    &sensor->bands[band_to_write].scas[sca_index]) != SUCCESS)
        {
            IAS_LOG_ERROR("Writing model SCA dynamic arrays datasets for "
                    "SCA group %s", band_sca_model_group_name);
            H5Gclose(group_id);
            return ERROR;
        }

        /* close the group */
        if (H5Gclose(group_id) < 0)
        {
            IAS_LOG_ERROR("Closing SCA group %s", band_sca_model_group_name);
            return ERROR;
        }
    }

    return SUCCESS;
}

/*************************************************************************

NAME: write_attitude_sample_records

PURPOSE: Internal routine to write the attitude sample records to the
    HDF5 file

RETURNS: SUCCESS -- successfully wrote the records to the file
         ERROR -- error in writing to the file
**************************************************************************/
static int write_attitude_sample_records
(
    hid_t group,                  /* I: An open HDF5 group handle */
    const IAS_SC_ATTITUDE_MODEL *att /* I: Attitude struct with data to write */
)
{
    hid_t space;        /* Identifier for the data space */
    hid_t dataset;      /* Identifier for the dataset */
    hid_t vector;       /* Identifier for compound VECTOR type */
    hid_t record;       /* Identifier for compound ATTITUDE_RECORD type */
    hsize_t dim[1];     /* Number of column dimensions in array */
    int status;         /* Return status of the HDF read routine */

    /* Create a nested data structure for the attitude record */
    status = ias_model_create_compound_vector_type(&vector);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Creating compound vector type for %s",
                ATTITUDE_RECORD_NAME);
        return ERROR;
    }
    status = ias_model_create_compound_attitude_type(vector, &record);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Creating compound record type for %s",
                ATTITUDE_RECORD_NAME);
        H5Tclose(vector);
        return ERROR;
    }

    /* Create a simple data space */
    dim[0] = att->sample_count;
    space = H5Screate_simple(1, dim, NULL);
    if (space < 0)
    {
        IAS_LOG_ERROR("Creating dataspace ID for %s", ATTITUDE_RECORD_NAME);
        H5Tclose(record);
        H5Tclose(vector);
        return ERROR;
    }

    /* Define and create a dataset */
    dataset = H5Dcreate(group, ATTITUDE_RECORD_NAME, record, space, H5P_DEFAULT,
            H5P_DEFAULT, H5P_DEFAULT);
    if (dataset < 0)
    {
        IAS_LOG_ERROR("Creating attitude record dataset %s",
                ATTITUDE_RECORD_NAME);
        H5Tclose(record);
        H5Tclose(vector);
        H5Sclose(space);
        return ERROR;
    }

    /* Write out the data */
    status = H5Dwrite(dataset, record, H5S_ALL, H5S_ALL,
            H5P_DEFAULT, att->sample_records);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing the image model attitude sample records for %s",
                ATTITUDE_RECORD_NAME);
        H5Tclose(record);
        H5Tclose(vector);
        H5Dclose(dataset);
        H5Sclose(space);
        return ERROR;
    }

    status = H5Tclose(record);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing compound record type for %s",
                ATTITUDE_RECORD_NAME);
        H5Tclose(vector);
        H5Dclose(dataset);
        H5Sclose(space);
        return ERROR;
    }
    status = H5Tclose(vector);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing compound vector type for %s",
                ATTITUDE_RECORD_NAME);
        H5Dclose(dataset);
        H5Sclose(space);
        return ERROR;
    }
    status = H5Dclose(dataset);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing dataset %s", ATTITUDE_RECORD_NAME);
        H5Sclose(space);
        return ERROR;
    }
    status = H5Sclose(space);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing dataspace ID for %s", ATTITUDE_RECORD_NAME);
        return ERROR;
    }

    return SUCCESS;
}

/*************************************************************************

NAME: write_ephemeris_sample_records

PURPOSE: Internal routine to write the ephemeris sample records to the
    HDF5 file

RETURNS: SUCCESS -- successfully wrote the records to the file
         ERROR -- error writing to the file
**************************************************************************/
static int write_ephemeris_sample_records
(
    hid_t group,                     /* I: An open HDF5 group handle */
    const IAS_SC_EPHEMERIS_MODEL *ephem /* I: ephemeris data to write */
)
{
    hid_t space;        /* Identifier for the data space */
    hid_t dataset;      /* Identifier for the dataset */
    hid_t vector;       /* Identifier for compound VECTOR type */
    hid_t record;       /* Identifier for compound EPHEMERIS_RECORD type */
    hsize_t dim[1];     /* Number of column dimensions in array */
    int status;         /* Return status of the HDF read routine */

    /* Create a nested data structure for the ephemeris record */
    status = ias_model_create_compound_vector_type(&vector);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Creating compound vector type for %s",
                EPHEMERIS_RECORD_NAME);
        return ERROR;
    }
    status = ias_model_create_compound_ephemeris_type(vector, &record);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Creating compound record type for %s",
                EPHEMERIS_RECORD_NAME);
        H5Tclose(vector);
        return ERROR;
    }

    /* Create a simple data space */
    dim[0] = ephem->sample_count;
    space = H5Screate_simple(1, dim, NULL);
    if (space < 0)
    {
        IAS_LOG_ERROR("Creating data space for model ephemeris record");
        H5Tclose(record);
        H5Tclose(vector);
        return ERROR;
    }

    /* Define and create a dataset */
    dataset = H5Dcreate(group, EPHEMERIS_RECORD_NAME, record, space,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dataset < 0)
    {
        IAS_LOG_ERROR("Creating dataspace ID for %s", EPHEMERIS_RECORD_NAME);
        H5Tclose(record);
        H5Tclose(vector);
        H5Sclose(space);
        return ERROR;
    }

    /* Write out the data */
    status = H5Dwrite(dataset, record, H5S_ALL, H5S_ALL,
            H5P_DEFAULT, ephem->sample_records);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing the image model ephemeris sample records for %s",
                EPHEMERIS_RECORD_NAME);
        H5Tclose(record);
        H5Tclose(vector);
        H5Dclose(dataset);
        H5Sclose(space);
        return ERROR;
    }

    status = H5Tclose(record);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing compound record type for %s",
                EPHEMERIS_RECORD_NAME);
        H5Tclose(vector);
        H5Dclose(dataset);
        H5Sclose(space);
        return ERROR;
    }
    status = H5Tclose(vector);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing compound vector type for %s",
                EPHEMERIS_RECORD_NAME);
        H5Dclose(dataset);
        H5Sclose(space);
        return ERROR;
    }
    status = H5Dclose(dataset);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing dataset %s", EPHEMERIS_RECORD_NAME);
        H5Sclose(space);
        return ERROR;
    }
    status = H5Sclose(space);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing dataspace ID for %s", EPHEMERIS_RECORD_NAME);
        return ERROR;
    }

    return SUCCESS;
}

/*************************************************************************

NAME: ias_model_write_jitter_table

PURPOSE: Routine to write the jitter table records to the HDF5 file

RETURNS: SUCCESS -- successfully wrote the records to the file
         ERROR -- error in writing to the file
**************************************************************************/
int ias_model_write_jitter_table
(
    hid_t file,                         /* I: An open HDF5 file handle */
    const IAS_SENSOR_MODEL *sensor      /* I: Jitter table data to write */
)
{
    hid_t group_id;
    hid_t vector;       /* Identifier for compound VECTOR type */
    hid_t space;        /* Identifier for the data space */
    hid_t dataset;      /* Identifier for the dataset */
    hsize_t dim[1];     /* Number of column dimensions in array */
    int sensor_id;      /* Current sensor ID writing */
    int status;         /* Return status of the HDF read routine */

    /* Create the jitter model group */
    group_id = H5Gcreate(file, JITTER_MODEL_GROUP_NAME, H5P_DEFAULT, 
            H5P_DEFAULT, H5P_DEFAULT);
    if (group_id < 0) 
    { 
        IAS_LOG_ERROR("Opening jitter model group");
        return ERROR; 
    } 

    for (sensor_id = 0; sensor_id < IAS_MAX_SENSORS; sensor_id++)
    {
        char jit_table_name[JITTER_TABLE_NAME_MAX_LENGTH + 1];
        char jit_entries_name[JITTER_ENTRIES_NAME_MAX_LENGTH + 1];
        if (sensor_id == IAS_OLI)
        {
            strncpy(jit_table_name, OLI_JITTER_TABLE_NAME,
                    JITTER_TABLE_NAME_MAX_LENGTH);
            jit_table_name[JITTER_TABLE_NAME_MAX_LENGTH] = '\0';
            strncpy(jit_entries_name, OLI_JITTER_ENTRIES_NAME,
                    JITTER_ENTRIES_NAME_MAX_LENGTH);
            jit_entries_name[JITTER_ENTRIES_NAME_MAX_LENGTH] = '\0';
        }
        else if (sensor_id == IAS_TIRS)
        {
            strncpy(jit_table_name, TIRS_JITTER_TABLE_NAME,
                    JITTER_TABLE_NAME_MAX_LENGTH);
            jit_table_name[JITTER_TABLE_NAME_MAX_LENGTH] = '\0';
            strncpy(jit_entries_name, TIRS_JITTER_ENTRIES_NAME,
                    JITTER_ENTRIES_NAME_MAX_LENGTH);
            jit_entries_name[JITTER_ENTRIES_NAME_MAX_LENGTH] = '\0';
        }
        else
        {
            IAS_LOG_ERROR("Unsupported sensor ID %d", sensor_id);
            H5Gclose(group_id);
            return ERROR;
        }

        /* Skip the sensor if it isn't present */
        if (!sensor->sensors[sensor_id].sensor_present)
            continue;

        /* Write the jitter entries per frame */
        dim[0] = 1;
        if (H5LTmake_dataset_int(group_id, jit_entries_name, 1, dim,
                    &sensor->sensors[sensor_id].jitter_entries_per_frame) < 0)
        {
            IAS_LOG_ERROR("Writing jitter entries per frame");
            H5Gclose(group_id);
            return ERROR;
        }

        /* Create the vector type */
        status = ias_model_create_compound_vector_type(&vector);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Creating compound vector type for %s",
                    jit_table_name);
            H5Gclose(group_id);
            return ERROR;
        }

        /* Create a simple data space */
        dim[0] = sensor->sensors[sensor_id].jitter_table_count;
        space = H5Screate_simple(1, dim, NULL);
        if (space < 0)
        {
            IAS_LOG_ERROR("Creating dataspace ID for %s", jit_table_name);
            H5Tclose(vector);
            H5Gclose(group_id);
            return ERROR;
        }

        /* Define and create a dataset */
        dataset = H5Dcreate(group_id, jit_table_name, vector, space,
                H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if (dataset < 0)
        {
            IAS_LOG_ERROR("Creating jitter table record dataset %s",
                    jit_table_name);
            H5Sclose(space);
            H5Tclose(vector);
            H5Gclose(group_id);
            return ERROR;
        }

        /* Write out the data */
        status = H5Dwrite(dataset, vector, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                sensor->sensors[sensor_id].jitter_table);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing the sensor model jitter table records "
                    "for %s", jit_table_name);
            H5Dclose(dataset);
            H5Sclose(space);
            H5Tclose(vector);
            H5Gclose(group_id);
            return ERROR;
        }

        status = H5Dclose(dataset);
        if (status < 0)
        {
            IAS_LOG_ERROR("Closing dataset %s", jit_table_name);
            H5Sclose(space);
            H5Tclose(vector);
            H5Gclose(group_id);
            return ERROR;
        }
        status = H5Sclose(space);
        if (status < 0)
        {
            IAS_LOG_ERROR("Closing dataspace ID for %s", jit_table_name);
            H5Tclose(vector);
            H5Gclose(group_id);
            return ERROR;
        }
        status = H5Tclose(vector);
        if (status < 0)
        {
            IAS_LOG_ERROR("Closing compound vector type for %s",
                    jit_table_name);
            H5Gclose(group_id);
            return ERROR;
        }
    }

    /* Close the group */
    if (H5Gclose(group_id) < 0)
    {
        IAS_LOG_ERROR("Closing jitter model group");
        return ERROR;
    }

    return SUCCESS;
}

/*************************************************************************
NAME: ias_model_write_ssm_model

PURPOSE: Internal routine to write the scene select mirror to the HDF5 file.

RETURNS: SUCCESS -- successfully wrote data to the HDF5 file
         ERROR -- error in writing to the HDF5 file
**************************************************************************/
int ias_model_write_ssm_model
(
    hid_t file,                         /* I: An open HDF5 file handle */
    const IAS_SENSOR_MODEL *sensor      /* I: SSM data to write */
)
{
    int sensor_id;

    for (sensor_id = 0; sensor_id < IAS_MAX_SENSORS; sensor_id++)
    {
        hid_t group_id;
        char group_name[80];
        hsize_t dims[2];
        IAS_SENSOR_SCENE_SELECT_MIRROR_MODEL *ssm 
                = sensor->sensors[sensor_id].ssm_model;

        /* skip the sensor if it isn't present */
        if (!sensor->sensors[sensor_id].sensor_present)
            continue;

        /* skip sensors without SSM data */
        if (!ssm)
            continue;

        /* create the group name */
        if (snprintf(group_name, sizeof(group_name),
             "Sensor %d Scene Select Mirror", sensor_id) >= sizeof(group_name))
        {
            IAS_LOG_ERROR("Buffer size is not large enough");
            return ERROR;
        }

        /* create the attitude group */
        group_id = H5Gcreate(file, group_name, H5P_DEFAULT, H5P_DEFAULT,
                             H5P_DEFAULT);
        if (group_id < 0) 
        { 
            IAS_LOG_ERROR("Opening SSM group %s", group_name);
            return ERROR; 
        } 

        /* write the epoch time to the group */
        if (write_time_header(group_id, ssm->utc_epoch_time,
                    DONOT_WRITE_SAMPLE_TIME, 0.0) != SUCCESS)
        {
            IAS_LOG_ERROR("Writing SSM epoch time header to group %s",
                          group_name);
            H5Gclose(group_id);
            return ERROR; 
        }

        /* write the alignment matrix to the group */
        dims[0] = TRANSFORMATION_MATRIX_DIM;
        dims[1] = TRANSFORMATION_MATRIX_DIM;
        if (H5LTmake_dataset_double(group_id, ALIGNMENT_MATRIX, 2, dims,
                    &ssm->alignment_matrix[0][0]) < 0)
        {
            IAS_LOG_ERROR("Writing SSM alignment matrix to group %s",
                          group_name);
            return ERROR;
        }

        /* write the SSM sample records */
        if (write_ssm_records(group_id, ssm) != SUCCESS)
        {
            IAS_LOG_ERROR("Writing SSM records to %s", group_name);
            H5Gclose(group_id);
            return ERROR;
        }

        /* close the group */
        if (H5Gclose(group_id) < 0)
        {
            IAS_LOG_ERROR("Closing SSM group %s", group_name);
            return ERROR;
        }
    }

    return SUCCESS;
}

/*************************************************************************

NAME: write_ssm_records

PURPOSE: Internal routine to write the SSM sample records to the HDF5 file

RETURNS: SUCCESS -- successfully wrote the records to the file
         ERROR -- error in writing to the file
**************************************************************************/
static int write_ssm_records
(
    hid_t group,                  /* I: An open HDF5 group handle */
    const IAS_SENSOR_SCENE_SELECT_MIRROR_MODEL *ssm /* I: ssm data to write */
)
{
    hid_t space;        /* Identifier for the data space */
    hid_t dataset;      /* Identifier for the dataset */
    hid_t record;       /* Identifier for compound type */
    hsize_t dim[1];     /* Number of column dimensions in array */
    int status;         /* Return status of the HDF read routine */

    /* create the SSM compound type */
    status = ias_model_create_compound_ssm_type(&record);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Creating compound record type for %s", SSM_RECORD_NAME);
        return ERROR;
    }

    /* Create a simple data space */
    dim[0] = ssm->ssm_record_count;
    space = H5Screate_simple(1, dim, NULL);
    if (space < 0)
    {
        IAS_LOG_ERROR("Creating dataspace ID for %s", SSM_RECORD_NAME);
        H5Tclose(record);
        return ERROR;
    }

    /* Define and create a dataset */
    dataset = H5Dcreate(group, SSM_RECORD_NAME, record, space, H5P_DEFAULT,
            H5P_DEFAULT, H5P_DEFAULT);
    if (dataset < 0)
    {
        IAS_LOG_ERROR("Creating SSM record dataset %s", SSM_RECORD_NAME);
        H5Tclose(record);
        H5Sclose(space);
        return ERROR;
    }

    /* Write out the data */
    status = H5Dwrite(dataset, record, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                      ssm->records);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing the SSM records for %s", SSM_RECORD_NAME);
        H5Tclose(record);
        H5Dclose(dataset);
        H5Sclose(space);
        return ERROR;
    }

    status = H5Tclose(record);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing compound record type for %s",
                SSM_RECORD_NAME);
        H5Dclose(dataset);
        H5Sclose(space);
        return ERROR;
    }
    status = H5Dclose(dataset);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing dataset %s", SSM_RECORD_NAME);
        H5Sclose(space);
        return ERROR;
    }
    status = H5Sclose(space);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing dataspace ID for %s", SSM_RECORD_NAME);
        return ERROR;
    }

    return SUCCESS;
}
