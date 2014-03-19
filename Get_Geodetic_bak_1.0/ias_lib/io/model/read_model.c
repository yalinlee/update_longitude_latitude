/*************************************************************************
NAME:    read_model contains the following routines:
             ias_model_read_table
             ias_model_read_frame_times
             ias_model_read_band_model
             ias_model_read_attitude_model
             ias_model_read_ephemeris_model
             ias_model_read_jitter_table
             ias_model_read_ssm_models
             verify_dataset_size
             read_time_header
             read_sca_dynamic_arrays
             read_band_sca_model
             read_attitude_sample_records
             read_ephemeris_sample_records

PURPOSE: Routines to support reading the model from an HDF5 file

RETURNS: SUCCESS or ERROR
**************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "ias_logging.h"
#include "common_model_io.h"

/* local defines */
#define DONOT_READ_SAMPLE_TIME 0
#define READ_SAMPLE_TIME 1

/* local routines */
static int verify_dataset_size
(
    hid_t group_id,             /* I: group id containing the dataset */
    const char *dataset_name,   /* I: name of dataset to check */
    int expected_rank,          /* I: expected rank */
    hsize_t *expected_dims      /* I: expected dimensions (size of rank) */
);
static int read_time_header
(
    hid_t group_id,             /* I: group id to read time header info from */
    double utc_epoch_time[3],   /* O: epoch time to read */
    int read_sample_time,       /* I: flag to indicate the nominal sample time
                                      should be read */
    double *nominal_sample_time /* O: nominal sample time to read */
);
static int read_sca_dynamic_arrays
(
    hid_t group,               /* I: An open HDF5 group handle */
    IAS_SENSOR_SCA_MODEL *sca  /* O: SCA structure to fill */
);
static int read_band_sca_model
(
    hid_t file,                 /* I: An open HDF5 group handle */
    int band_to_read,           /* I: Which band we are working with (index) */
    IAS_LOS_MODEL *los_model    /* O: Model structure to fill */
);
static int read_attitude_sample_records
(
    hid_t group_id,             /* I: An open HDF5 group handle */
    IAS_SC_ATTITUDE_MODEL *att     /* O: attitude structure to fill */
);
static int read_ephemeris_sample_records
(
    hid_t group_id,             /* I: An open HDF5 group handle */
    IAS_SC_EPHEMERIS_MODEL *ephem  /* O: ephemeris structure to be filled */
);
static int read_ssm_records
(
    hid_t group_id,             /* I: An open HDF5 group handle */
    IAS_SENSOR_SCENE_SELECT_MIRROR_MODEL *ssm /* I: ssm data to write */
);


/*************************************************************************
NAME: ias_model_read_table

PURPOSE: Internal routine to read the indicated (param) model table

RETURNS: SUCCESS -- successfully read contents of HDF table to structure
         ERROR -- error reading HDF file table
**************************************************************************/
int ias_model_read_table
(
    hid_t file,             /* I: An open HDF5 file handle */
    MODEL_TABLE table,      /* I: Flag indicating which table to read */
    int band_to_read,       /* I: For reading band or SCA tables, else -1 */
    int sca_to_read,        /* I: For reading SCA table, else -1 */
    IAS_LOS_MODEL *los_model/* O: Model structure to store data */
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
    IAS_SENSOR_MODEL *sensor = &los_model->sensor; /* sensor model pointer */
    void *the_struct = NULL;          /* Pointer to the structure to fill */
    hsize_t record_count = 0;
    int detectors;
    int i, j, k;                      /* General looping counters */

    /* Build a description of the table data.  Also set a pointer to the
       model member structure, or general structure, and set the total
       size of the structure. */
    switch (table)
    {
        case GENERAL_TABLE :
            memset(&model, 0, sizeof(model));
            status = ias_model_build_general_description(offsets, field_names,
                    field_type, fields_to_close, field_sizes, &number_fields);
            strcpy(table_name, GENERAL_TABLE_NAME);
            the_struct = &model;
            total_size = sizeof(GENERAL_MODEL);
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
            memset(sensor_loc_model, 0, sizeof(sensor_loc_model));
            status = ias_model_build_sensor_description(offsets, field_names,
                    field_type, fields_to_close, field_sizes, &number_fields);
            strcpy(table_name, SENSOR_TABLE_NAME);
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
            memset(&band_model, 0, sizeof(band_model));
            status = ias_model_build_band_description(offsets, field_names,
                    field_type, fields_to_close, field_sizes, &number_fields);
            strcpy(table_name, BAND_TABLE_NAME);
            total_size = sizeof(BAND_SAMP_CHAR_MODEL);
            record_count = 1;

            if (status == ERROR)
            {
                IAS_LOG_ERROR("Building table description for table %s, "
                        "band number %d", table_name, band_to_read);
                return_status = ERROR;
                break;
            }

            /* Read the table */
            if (H5TBread_records(file, table_name, 0, record_count, total_size,
                        offsets, field_sizes, &band_model) < 0)
            {
                IAS_LOG_ERROR("Reading band records for table %s, band number "
                        "%d", table_name, band_to_read);
                return_status = ERROR;
                break;
            }

            /* make sure the number of SCAs matches what is already stored in
               the model by the allocation routine */
            if (sensor->bands[band_to_read].sca_count != band_model.sca_count)
            {
                IAS_LOG_ERROR("Unexpected number of SCAs in the model file. "
                        "Band %d, expected %d, read %d", band_to_read,
                        sensor->bands[band_to_read].sca_count,
                        band_model.sca_count);
                return ERROR;
            }

            /* Store the band and sampling characteristics info in the model */
            sensor->bands[band_to_read].band_present = band_model.band_present;
            sensor->bands[band_to_read].utc_epoch_time[0]
                = band_model.utc_epoch_time[0];
            sensor->bands[band_to_read].utc_epoch_time[1]
                = band_model.utc_epoch_time[1];
            sensor->bands[band_to_read].utc_epoch_time[2]
                = band_model.utc_epoch_time[2];
            sensor->bands[band_to_read].sampling_char.integration_time
                = band_model.integration_time;
            sensor->bands[band_to_read].sampling_char.sampling_time
                = band_model.sampling_time;
            sensor->bands[band_to_read].sampling_char.lines_per_frame
                = band_model.lines_per_frame;
            sensor->bands[band_to_read].sampling_char.settling_time
                = band_model.settling_time;
            sensor->bands[band_to_read].sampling_char.along_ifov
                = band_model.along_ifov;
            sensor->bands[band_to_read].sampling_char.across_ifov
                = band_model.across_ifov;
            sensor->bands[band_to_read].sampling_char.maximum_detector_delay
                = band_model.maximum_detector_delay;
            sensor->bands[band_to_read].sampling_char.time_codes_at_frame_start
                = band_model.time_codes_at_frame_start;
            sensor->bands[band_to_read].sampling_char.frame_delay
                = band_model.frame_delay;

            if (read_band_sca_model(file, band_to_read, los_model) != SUCCESS)
            {
                IAS_LOG_ERROR("Reading SCA model for band %d", band_to_read);
                return_status = ERROR;
                break;
            }
            break;
        case BAND_SCA_TABLE :
            memset(&sca_model, 0, sizeof(sca_model));
            detectors
                = sensor->bands[band_to_read].scas[sca_to_read].detectors;
            status = ias_model_build_band_sca_description(detectors, offsets,
                    field_names, field_type, fields_to_close, field_sizes,
                    &number_fields);
            strcpy(table_name, SCA_TABLE_NAME);
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
        IAS_LOG_ERROR("Preparing for read or reading table %s", table_name);
        ias_model_close_the_fields(fields_to_close);
        return ERROR;
    }

    /* Read the table */
    if (table != BAND_TABLE)
    {
        if (H5TBread_records(file, table_name, 0, record_count, total_size,
                    offsets, field_sizes, the_struct) < 0)
        {
            IAS_LOG_ERROR("Reading table %s", table_name);
            return_status = ERROR;
        }
    }

    /* If this is the general table, store the contents in the model */
    if (table == GENERAL_TABLE)
    {
        /* make sure the satellite id matches the one the model is allocated 
           for */
        if (los_model->satellite_id != model.satellite_id)
        {
            IAS_LOG_ERROR("Unexpected satellite ID in the model file.  "
                    "Expected %d, read %d", los_model->satellite_id, 
                    model.satellite_id);
            ias_model_close_the_fields(fields_to_close);
            return ERROR;
        }
        strcpy(los_model->sw_version, model.sw_version);
        los_model->wrs_path = model.wrs_path;
        los_model->wrs_row = model.wrs_row;
        los_model->acquisition_type = model.acquisition_type;
        los_model->correction_type = model.correction_type;
    }

    /* If this is the sensor location table, store the contents in the model */
    if (table == SENSOR_TABLE)
    {
        for (i = 0; i < IAS_MAX_SENSORS; i++)
        {
            sensor->sensors[i].sensor_id = sensor_loc_model[i].sensor_id;
            sensor->sensors[i].sensor_present
                = sensor_loc_model[i].sensor_present;
            for (j = 0; j < TRANSFORMATION_MATRIX_DIM; j++)
            {
                for (k = 0; k < TRANSFORMATION_MATRIX_DIM; k++)
                {
                    sensor->sensors[i].sensor2acs[j][k]
                        = sensor_loc_model[i].sensor2acs[j][k];
                }
                /* Set the center of mass to sensor offsets */
                sensor->sensors[i].center_mass2sensor_offset[j]
                    = sensor_loc_model[i].center_mass2sensor_offset[j];
            }

            /* If the sensor is present and the SSM model present flag is set,
               that is the indication that the ssm_model memory needs to be
               allocated (the ssm_model pointer is the "present flag" for a
               model in memory and converted to the ssm_model_present flag when
               written to the file */
            if (sensor->sensors[i].sensor_present 
                && sensor_loc_model[i].ssm_model_present)
            {
                /* allocate space for the SSM model since it is present.  It
                   will be populated later. */
                sensor->sensors[i].ssm_model
                    = malloc(sizeof(*sensor->sensors[i].ssm_model));
                if (!sensor->sensors[i].ssm_model)
                {
                    /* failed allocation, so error out */
                    IAS_LOG_ERROR("Memory allocation for SSM model failed");
                    ias_model_close_the_fields(fields_to_close);
                    return ERROR;
                }

                /* set the records pointer and count to starting values that
                   will be filled in later */
                sensor->sensors[i].ssm_model->ssm_record_count = 0;
                sensor->sensors[i].ssm_model->records = NULL;
            }
            else
            {
                /* SSM not present, so set its pointer to null */
                sensor->sensors[i].ssm_model = NULL;
            }
        }
    }

    /* If this is the SCA table, store the contents in the model */
    if (table == BAND_SCA_TABLE)
    {
        sensor->bands[band_to_read].scas[sca_to_read].nominal_fill
            = sca_model.nominal_fill;

        for (i = 0; i < IAS_LOS_LEGENDRE_TERMS; i++)
        {
            sensor->bands[band_to_read].scas[sca_to_read].sca_coef_x[i]
                = sca_model.sca_coef_x[i];
            sensor->bands[band_to_read].scas[sca_to_read].sca_coef_y[i]
                = sca_model.sca_coef_y[i];
        }
    }

    ias_model_close_the_fields(fields_to_close);
    return return_status;
}

/*************************************************************************
NAME: ias_model_read_frame_times

PURPOSE: Internal routine to read frame times from epoch

RETURNS: SUCCESS -- successfully read times from HDF5 file to structure
         ERROR -- error in reading HDF5 file
**************************************************************************/
int ias_model_read_frame_times
(
    hid_t file,                 /* I: An open HDF5 file handle */
    IAS_LOS_MODEL *los_model    /* O: Model structure to fill */
)
{
    hid_t space;            /* Identifier for the data space */
    hid_t dataset;          /* Identifier for the dataset */
    hsize_t ndims;          /* Number of row dimensions in array */
    hsize_t dims[2];        /* Number of column dimensions in array */
    int status;             /* Return status of the HDF read routine */
    int sensor_index;
    char dataset_name[256];
    double *frame_seconds_from_epoch = NULL;

    for (sensor_index = 0; sensor_index < IAS_MAX_SENSORS; sensor_index++)
    {
        /* Only try to retrieve the frame times for the sensor if the sensor
           was written to the model file to begin with, based on a flag we
           setup in the model that was read from the file previously. */
        if (los_model->sensor.sensors[sensor_index].sensor_present)
        {
            const char *sensor_name;

            sensor_name = ias_sat_attr_get_sensor_name(sensor_index);
            if (strcmp(sensor_name, "Unknown") == 0)
            {
                IAS_LOG_ERROR("Getting sensor name for sensor index %d",
                        sensor_index);
                return ERROR;
            }

            /* build the dataset name from the sensor name and the base name */
            snprintf(dataset_name, sizeof(dataset_name), "%s %s", sensor_name,
                    FRAME_TIMES_RECORD_NAME);

            /* verify the dataset actually exists */
            if (H5Lexists(file, dataset_name, H5P_DEFAULT) != TRUE)
            {
                /* If the model contained the flag that this sensor is present
                   but we cannot successfully read the frame seconds from
                   epoch for this sensor (which should be there in all cases
                   for a present sensor), then we have an unexpected error
                   situation. */
                IAS_LOG_ERROR("Model frame times do not exist for %s",
                        dataset_name);
                return ERROR;
            }

            /* Open the dataset */
            dataset = H5Dopen(file, dataset_name, H5P_DEFAULT);
            if (dataset < 1)
            {
                IAS_LOG_ERROR("Opening model frame times from epoch dataset %s",
                        dataset_name);
                return ERROR;
            }

            /* Get the data space and its dimensions */
            space = H5Dget_space(dataset);
            if (space < 0)
            {
                IAS_LOG_ERROR("Getting dataspace ID for %s", dataset_name);
                H5Dclose(dataset);
                return ERROR;
            }

            ndims = H5Sget_simple_extent_ndims(space);
            if (ndims != 1)
            {
                IAS_LOG_ERROR("Invalid frame times dimensions for "
                        "dataspace %i", space);
                H5Sclose(space);
                H5Dclose(dataset);
                return ERROR;
            }

            status = H5Sget_simple_extent_dims(space, dims, NULL);
            if (status < 0)
            {
                IAS_LOG_ERROR("Getting dimension for dataspace %i", space);
                H5Sclose(space);
                H5Dclose(dataset);
                return ERROR;
            }

            if (dims[0] <= 0)
            {
                IAS_LOG_ERROR("Invalid dimension for dataspace %i", space);
                H5Sclose(space);
                H5Dclose(dataset);
                return ERROR;
            }

            /* Allocate memory for the data */
            frame_seconds_from_epoch = malloc(sizeof(double) * dims[0]);
            if (!frame_seconds_from_epoch)
            {
                IAS_LOG_ERROR("Allocating frame times for sensor index %d",
                        sensor_index);
                H5Sclose(space);
                H5Dclose(dataset);
                return ERROR;
            }

            /* Read in the data */
            status = H5Dread(dataset, H5T_NATIVE_DOUBLE, space, H5S_ALL,
                    H5P_DEFAULT, frame_seconds_from_epoch);
            if (status < 0)
            {
                IAS_LOG_ERROR("Reading the frame times from epoch for %s",
                        dataset_name);
                free(frame_seconds_from_epoch);
                H5Sclose(space);
                H5Dclose(dataset);
                return ERROR;
            }

            status = H5Sclose(space);
            if (status < 0)
            {
                IAS_LOG_ERROR("Closing dataspace ID for %s", dataset_name);
                H5Dclose(dataset);
                return ERROR;
            }
            status = H5Dclose(dataset);
            if (status < 0)
            {
                IAS_LOG_ERROR("Closing dataset %s", dataset_name);
                return ERROR;
            }

            /* set the frame times in the model */
            if (ias_sensor_set_frame_times(&los_model->sensor, sensor_index,
                        frame_seconds_from_epoch, dims[0]) != SUCCESS)
            {
                IAS_LOG_ERROR("Setting the frame times in the model for "
                        "sensor %d", sensor_index);
                free(frame_seconds_from_epoch);
                return ERROR;
            }
        }
    }

    return SUCCESS;
}

/*************************************************************************
NAME: ias_model_read_band_model

PURPOSE: Internal routine to read the band model from HDF5 file to struct

RETURNS: SUCCESS -- successfully retrieved records to structure
         ERROR -- error reading HDF5 file
**************************************************************************/
int ias_model_read_band_model
(
    hid_t file,                 /* I: An open HDF5 file handle */
    IAS_LOS_MODEL *los_model    /* O: Model structure to store data */
)
{
    hid_t group_id;
    char band_model_group_name[20];
    IAS_SENSOR_MODEL *sensor = &los_model->sensor;
    int band_index;

    /* open the band group */
    for (band_index = 0; band_index < sensor->band_count; band_index++)
    {
        int band_number;

        band_number = ias_sat_attr_convert_band_index_to_number(band_index);
        if (band_number == ERROR)
        {
            IAS_LOG_ERROR("Converting band index %d to band number",
                    band_index);
            return ERROR;
        }
        sprintf(band_model_group_name, "Band %02d", band_number);

        group_id = H5Gopen(file, band_model_group_name, H5P_DEFAULT);
        if (group_id < 0) 
        { 
            IAS_LOG_ERROR("Opening band group %s", band_model_group_name);
            return ERROR; 
        } 

        if (ias_model_read_table(group_id, BAND_TABLE, band_index, -1,
                    los_model) != SUCCESS)
        {
            IAS_LOG_ERROR("Reading model band group %s", band_model_group_name);
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
NAME: ias_model_read_attitude_model

PURPOSE: Internal routine to read the attitude data from the HDF5 file

RETURNS: SUCCESS -- successfully retrieved desired data
         ERROR -- error reading HDF5 file
**************************************************************************/
int ias_model_read_attitude_model
(
    hid_t file,                 /* I: An open HDF5 file handle */
    IAS_SC_ATTITUDE_MODEL *att  /* I: Attitude data to read */
)
{
    hid_t group_id;

    /* open the attitude group */
    group_id = H5Gopen(file, ATTITUDE_MODEL_GROUP_NAME, H5P_DEFAULT);
    if (group_id < 0) 
    { 
        IAS_LOG_ERROR("Opening attitude group");
        return ERROR; 
    } 

    /* read the time header info to the group */
    if (read_time_header(group_id, att->utc_epoch_time, READ_SAMPLE_TIME,
                &att->nominal_sample_time) != SUCCESS)
    {
        IAS_LOG_ERROR("Reading attitude model time header");
        H5Gclose(group_id);
        return ERROR; 
    }

    /* read the attitude sample records */
    if (read_attitude_sample_records(group_id, att) != SUCCESS)
    {
        IAS_LOG_ERROR("Reading attitude model sample records");
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
NAME: ias_model_read_ephemeris_model

PURPOSE: Internal routine to read the ephemeris data from the HDF5 file

RETURNS: SUCCESS -- successfully retrieved desired data
         ERROR -- error reading HDF5 file
**************************************************************************/
int ias_model_read_ephemeris_model
(
    hid_t file,                     /* I: An open HDF5 file handle */
    IAS_SC_EPHEMERIS_MODEL *ephem   /* I: ephemeris data to read */
)
{
    hid_t group_id;

    /* open the ephemeris group */
    group_id = H5Gopen(file, EPHEMERIS_MODEL_GROUP_NAME, H5P_DEFAULT);
    if (group_id < 0) 
    { 
        IAS_LOG_ERROR("Opening ephemeris group");
        return ERROR; 
    } 

    /* read the time header info from the group */
    if (read_time_header(group_id, ephem->utc_epoch_time, READ_SAMPLE_TIME,
                &ephem->nominal_sample_time) != SUCCESS)
    {
        IAS_LOG_ERROR("Reading ephemeris model time header");
        H5Gclose(group_id);
        return ERROR; 
    }

    /* read the ephemeris sample records */
    if (read_ephemeris_sample_records(group_id, ephem) != SUCCESS)
    {
        IAS_LOG_ERROR("Reading ephemeris model sample records");
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
NAME: verify_dataset_size

PURPOSE: Internal routine to check value as array being read from HDF is
         rank (number of dimensions) expected

RETURNS: SUCCESS -- rank matches expected rank/dimensions
         ERROR -- rank mis-match
**************************************************************************/
static int verify_dataset_size
(
    hid_t group_id,             /* I: group id containing the dataset */
    const char *dataset_name,   /* I: name of dataset to check */
    int expected_rank,          /* I: expected rank */
    hsize_t *expected_dims      /* I: expected dimensions (size of rank) */
)
{
    int rank;
    hsize_t dims[expected_rank];
    H5T_class_t class_id;
    size_t type_size;

    if (H5LTget_dataset_ndims (group_id, dataset_name, &rank) < 0)
    {
        IAS_LOG_ERROR("Getting rank for %s", dataset_name);
        return ERROR;
    }
    if (rank != expected_rank)
    {
        IAS_LOG_ERROR("Invalid rank for %s", dataset_name);
        return ERROR;
    }
    if (H5LTget_dataset_info(group_id, dataset_name, dims, &class_id,
                &type_size) < 0)
    {
        IAS_LOG_ERROR("Retrieving dimensions for %s", dataset_name);
        return ERROR;
    }
    for (rank = 0; rank < expected_rank; rank++)
    {
        if (dims[rank] != expected_dims[rank])
        {
            IAS_LOG_ERROR("Unexpected dimension of %llu for rank %d of %s "
                    "(expected %llu)", dims[rank], rank, dataset_name,
                    expected_dims[rank]);
            return ERROR;
        }
    }
    return SUCCESS;
}

/*************************************************************************
NAME: read_time_header

PURPOSE: Internal routine to read UTC epoch time and nominal sample time

RETURNS: SUCCESS -- successfully read the desired values
         ERROR -- error reading HDF5 file
**************************************************************************/
static int read_time_header
(
    hid_t group_id,             /* I: group id to read time header info from */
    double utc_epoch_time[3],   /* O: epoch time to read */
    int read_sample_time,       /* I: flag to indicate the nominal sample time
                                      should be read */
    double *nominal_sample_time /* O: nominal sample time to read */
)
{
    hsize_t dims[1];

    /* verify the dimensions on the UTC epoch time dataset */
    dims[0] = 3;
    if (verify_dataset_size(group_id, UTC_EPOCH_TIME, 1, dims) != SUCCESS)
    {
        IAS_LOG_ERROR("In the dimensions for attitude epoch time");
        return ERROR;
    }

    /* read the UTC epoch time */
    dims[0] = 3;
    if (H5LTread_dataset_double(group_id, UTC_EPOCH_TIME, utc_epoch_time) < 0)
    {
        IAS_LOG_ERROR("Reading UTC epoch time for the attitude");
        return ERROR;
    }

    if (read_sample_time == READ_SAMPLE_TIME)
    {
        /* verify the dimensions on the nominal sample time dataset */
        dims[0] = 1;
        if (verify_dataset_size(group_id, NOMINAL_SAMPLE_TIME, 1, dims)
                != SUCCESS)
        {
            IAS_LOG_ERROR("In the dimensions for attitude nominal sample time");
            return ERROR;
        }

        /* read the nominal sample time */
        dims[0] = 1;
        if (H5LTread_dataset_double(group_id, NOMINAL_SAMPLE_TIME,
                    nominal_sample_time) < 0)
        {
            IAS_LOG_ERROR("Reading nominal sample time for the attitude");
            return ERROR;
        }
    }

    return SUCCESS;
}

/*************************************************************************
NAME: read_sca_dynamic_arrays

PURPOSE: Internal routine to read various dynamic sized values from HDF5
    file to struct

RETURNS: SUCCESS -- successfully retrieved records to structure
         ERROR -- error reading HDF5 file
**************************************************************************/
static int read_sca_dynamic_arrays
(
    hid_t group,               /* I: An open HDF5 group handle */
    IAS_SENSOR_SCA_MODEL *sca  /* O: SCA structure to fill */
)
{
    hid_t space;        /* Identifier for the data space */
    hid_t dataset;      /* Identifier for the dataset */
    hsize_t ndims;      /* Number of row dimensions in array */
    hsize_t dims[2];    /* Number of column dimensions in array */
    int status;         /* Return status of the HDF read routine */

    /*---------------L0R Fill-----------------*/
    /* Open the dataset */
    dataset = H5Dopen(group, SCA_L0R_FILL_RECORD_NAME, H5P_DEFAULT);
    if (dataset < 1)
    {
        IAS_LOG_ERROR("Opening model SCA dynamic array dataset %s",
                SCA_L0R_FILL_RECORD_NAME);
        return ERROR;
    }

    /* Get the data space and its dimensions */
    space = H5Dget_space(dataset);
    if (space < 0)
    {
        IAS_LOG_ERROR("Getting dataspace ID for %s", SCA_L0R_FILL_RECORD_NAME);
        H5Dclose(dataset);
        return ERROR;
    }

    /* The dimensions should be 1 x number of detectors */
    ndims = H5Sget_simple_extent_ndims(space);
    if (ndims != 1)
    {
        IAS_LOG_ERROR("Invalid SCA dynamic array dimensions for dataspace %i",
                space);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    status = H5Sget_simple_extent_dims(space, dims, NULL);
    if (status < 0)
    {
        IAS_LOG_ERROR("Getting dimension for dataspace %i", space);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    /* Check that the number of entries of L0R fill, which should be equal
       to the number of detectors, is consistent with the number of detectors
       already stored in the model by the satellite attributes initialization */
    if (dims[0] != sca->detectors)
    {
        IAS_LOG_ERROR("Unexpected number of detectors in the model file. "
                "Expected %d, read %d", sca->detectors, (int)dims[0]);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    /* Read in the data */
    status = H5Dread(dataset, H5T_NATIVE_INT, space, H5S_ALL, H5P_DEFAULT,
            sca->l0r_detector_offsets);
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading the image model SCA dynamic array for %s",
                SCA_L0R_FILL_RECORD_NAME);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    status = H5Sclose(space);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing dataspace ID for %s", SCA_L0R_FILL_RECORD_NAME);
        H5Dclose(dataset);
        return ERROR;
    }
    status = H5Dclose(dataset);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing dataset %s", SCA_L0R_FILL_RECORD_NAME);
        return ERROR;
    }

    /*---------------Along-track Detector Offsets-----------------*/
    /* Open the dataset */
    dataset = H5Dopen(group, SCA_ALONG_DET_OFF_RECORD_NAME, H5P_DEFAULT);
    if (dataset < 1)
    {
        IAS_LOG_ERROR("Opening model SCA dynamic array dataset %s",
                SCA_ALONG_DET_OFF_RECORD_NAME);
        return ERROR;
    }

    /* Get the data space and its dimensions */
    space = H5Dget_space(dataset);
    if (space < 0)
    {
        IAS_LOG_ERROR("Getting dataspace ID for %s",
                SCA_ALONG_DET_OFF_RECORD_NAME);
        H5Dclose(dataset);
        return ERROR;
    }

    ndims = H5Sget_simple_extent_ndims(space);
    /* The dimensions should be 1 x number of detectors */
    if (ndims != 1)
    {
        IAS_LOG_ERROR("Invalid SCA dynamic array dimensions for dataspace %i",
                space);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    status = H5Sget_simple_extent_dims(space, dims, NULL);
    if (status < 0)
    {
        IAS_LOG_ERROR("Getting dimension for dataspace %i", space);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    /* Check that the number of entries of detector offsets, which should be
       equal to the number of detectors, is consistent with the number of
       detectors already stored in the model by the satellite attributes
       initialization */
    if (dims[0] != sca->detectors)
    {
        IAS_LOG_ERROR("Unexpected number of detectors in the model file. "
                "Expected %d, read %d", sca->detectors, (int)dims[0]);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    /* Read in the data */
    status = H5Dread(dataset, H5T_NATIVE_DOUBLE, space, H5S_ALL, H5P_DEFAULT,
            sca->detector_offsets_along_track);
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading the image model SCA dynamic array for %s",
                SCA_ALONG_DET_OFF_RECORD_NAME);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    status = H5Sclose(space);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing dataspace ID for %s",
                SCA_ALONG_DET_OFF_RECORD_NAME);
        H5Dclose(dataset);
        return ERROR;
    }
    status = H5Dclose(dataset);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing dataset %s", SCA_ALONG_DET_OFF_RECORD_NAME);
        return ERROR;
    }

    /*---------------Across-track Detector Offsets-----------------*/
    /* Open the dataset */
    dataset = H5Dopen(group, SCA_ACROSS_DET_OFF_RECORD_NAME, H5P_DEFAULT);
    if (dataset < 1)
    {
        IAS_LOG_ERROR("Opening model SCA dynamic array dataset %s",
                SCA_ACROSS_DET_OFF_RECORD_NAME);
        return ERROR;
    }

    /* Get the data space and its dimensions */
    space = H5Dget_space(dataset);
    if (space < 0)
    {
        IAS_LOG_ERROR("Getting dataspace ID for %s",
                SCA_ACROSS_DET_OFF_RECORD_NAME);
        H5Dclose(dataset);
        return ERROR;
    }

    ndims = H5Sget_simple_extent_ndims(space);
    /* The dimensions should be 1 x number of detectors */
    if (ndims != 1)
    {
        IAS_LOG_ERROR("Invalid SCA dynamic array dimensions for dataspace %i",
                space);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    status = H5Sget_simple_extent_dims(space, dims, NULL);
    if (status < 0)
    {
        IAS_LOG_ERROR("Getting dimension for dataspace %i", space);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    /* Check that the number of entries of detector offsets, which should be
       equal to the number of detectors, is consistent with the number of
       detectors already stored in the model by the satellite attributes
       initialization */
    if (dims[0] != sca->detectors)
    {
        IAS_LOG_ERROR("Unexpected number of detectors in the model file. "
                "Expected %d, read %d", sca->detectors, (int)dims[0]);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    /* Read in the data */
    status = H5Dread(dataset, H5T_NATIVE_DOUBLE, space, H5S_ALL, H5P_DEFAULT,
            sca->detector_offsets_across_track);
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading the image model SCA dynamic array for %s",
                SCA_ACROSS_DET_OFF_RECORD_NAME);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    status = H5Sclose(space);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing dataspace ID for %s",
                SCA_ACROSS_DET_OFF_RECORD_NAME);
        H5Dclose(dataset);
        return ERROR;
    }
    status = H5Dclose(dataset);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing dataset %s", SCA_ACROSS_DET_OFF_RECORD_NAME);
        return ERROR;
    }

    return SUCCESS;
}

/*************************************************************************
NAME: read_band_sca_model

PURPOSE: Internal routine to read the SCA model for the given band from
    HDF5 file to struct

RETURNS: SUCCESS -- successfully retrieved records to structure
         ERROR -- error reading HDF5 file
**************************************************************************/
static int read_band_sca_model
(
    hid_t file,                 /* I: An open HDF5 group handle */
    int band_to_read,           /* I: Which band we are working with (index) */
    IAS_LOS_MODEL *los_model    /* O: Model structure to fill */
)
{
    hid_t group_id;
    char band_sca_model_group_name[20];
    IAS_SENSOR_MODEL *sensor = &los_model->sensor;
    int sca_index;

    /* Read each SCA group */
    for (sca_index = 0; sca_index < sensor->bands[band_to_read].sca_count;
            sca_index++)
    {
        sprintf(band_sca_model_group_name, "SCA %02d", sca_index);

        group_id = H5Gopen(file, band_sca_model_group_name, H5P_DEFAULT);
        if (group_id < 0) 
        { 
            IAS_LOG_ERROR("Opening SCA group %s", band_sca_model_group_name);
            return ERROR; 
        } 

        if (ias_model_read_table(group_id, BAND_SCA_TABLE, band_to_read,
                    sca_index, los_model) != SUCCESS)
        {
            IAS_LOG_ERROR("Reading model SCA group %s",
                    band_sca_model_group_name);
            H5Gclose(group_id);
            return ERROR;
        }

        if (read_sca_dynamic_arrays(group_id,
                    &sensor->bands[band_to_read].scas[sca_index]) != SUCCESS)
        {
            IAS_LOG_ERROR("Reading model SCA dynamic arrays datasets for "
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
NAME: read_attitude_sample_records

PURPOSE: Internal routine to read attitude sample records from HDF5 file
    to struct

RETURNS: SUCCESS -- successfully retrieved records to structure
         ERROR -- error reading HDF5 file
**************************************************************************/
static int read_attitude_sample_records
(
    hid_t group_id,             /* I: An open HDF5 group handle */
    IAS_SC_ATTITUDE_MODEL *att     /* O: attitude structure to fill */
)
{
    hid_t space;        /* Identifier for the data space */
    hid_t dataset;      /* Identifier for the dataset */
    hid_t vector;       /* Identifier for compound VECTOR type */
    hid_t record;       /* Identifier for compound ATTITUDE_RECORD type */
    hsize_t ndims;      /* Number of row dimensions in array */
    hsize_t dims[2];    /* Number of column dimensions in array */
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

    /* Open the dataset */
    dataset = H5Dopen(group_id, ATTITUDE_RECORD_NAME, H5P_DEFAULT);
    if (dataset < 1)
    {
        IAS_LOG_ERROR("Opening model attitude record dataset %s",
                ATTITUDE_RECORD_NAME);
        H5Tclose(record);
        H5Tclose(vector);
        return ERROR;
    }

    /* Get the data space and its dimensions */
    space = H5Dget_space(dataset);
    if (space < 0)
    {
        IAS_LOG_ERROR("Getting dataspace ID for %s", ATTITUDE_RECORD_NAME);
        H5Tclose(record);
        H5Tclose(vector);
        H5Dclose(dataset);
        return ERROR;
    }

    ndims = H5Sget_simple_extent_ndims(space);
    if (ndims != 1)
    {
        IAS_LOG_ERROR("Invalid attitude record dimensions for dataspace %i",
                space);
        H5Tclose(record);
        H5Tclose(vector);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    status = H5Sget_simple_extent_dims(space, dims, NULL);
    if (status < 0)
    {
        IAS_LOG_ERROR("Getting dimension for dataspace %i", space);
        H5Tclose(record);
        H5Tclose(vector);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    if (dims[0] <= 0)
    {
        IAS_LOG_ERROR("Invalid dimension for dataspace %i", space);
        H5Tclose(record);
        H5Tclose(vector);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }
    att->sample_count = dims[0];

    /* Allocate memory for the data */
    att->sample_records = malloc(sizeof(IAS_SC_ATTITUDE_RECORD)
            * att->sample_count);
    if (!att->sample_records)
    {
        IAS_LOG_ERROR("Allocating model attitude sample records");
        H5Tclose(record);
        H5Tclose(vector);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    /* Read in the data */
    status = H5Dread(dataset, record, space, H5S_ALL, H5P_DEFAULT,
            att->sample_records);
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading the image model attitude sample records for %s",
                ATTITUDE_RECORD_NAME);
        free(att->sample_records);
        att->sample_records = NULL;
        H5Tclose(record);
        H5Tclose(vector);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    status = H5Tclose(record);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing compound record type for %s",
                ATTITUDE_RECORD_NAME);
        H5Tclose(vector);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }
    status = H5Tclose(vector);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing compound vector type for %s",
                ATTITUDE_RECORD_NAME);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }
    status = H5Sclose(space);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing dataspace ID for %s", ATTITUDE_RECORD_NAME);
        H5Dclose(dataset);
        return ERROR;
    }
    status = H5Dclose(dataset);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing dataset %s", ATTITUDE_RECORD_NAME);
        return ERROR;
    }

    return SUCCESS;
}

/*************************************************************************
NAME: read_ephemeris_sample_records

PURPOSE: Internal routine to read ephemeris sample records from HDF5 file
         to structure

RETURNS: SUCCESS -- successfully retrieved records to structure
         ERROR -- error reading HDF5 file
**************************************************************************/
static int read_ephemeris_sample_records
(
    hid_t group_id,             /* I: An open HDF5 group handle */
    IAS_SC_EPHEMERIS_MODEL *ephem  /* O: ephemeris structure to be filled */
)
{
    hid_t space;        /* Identifier for the data space */
    hid_t dataset;      /* Identifier for the dataset */
    hid_t vector;       /* Identifier for compound VECTOR type */
    hid_t record;       /* Identifier for compound ATTITUDE_RECORD type */
    hsize_t ndims;      /* Number of row dimensions in array */
    hsize_t dims[2];    /* Number of column dimensions in array */
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

    /* Open the dataset */
    dataset = H5Dopen(group_id, EPHEMERIS_RECORD_NAME, H5P_DEFAULT);
    if (dataset < 1)
    {
        IAS_LOG_ERROR("Opening model ephemeris record dataset %s",
                EPHEMERIS_RECORD_NAME);
        H5Tclose(record);
        H5Tclose(vector);
        return ERROR;
    }

    /* Get the data space and its dimensions */
    space = H5Dget_space(dataset);
    if (space < 0)
    {
        IAS_LOG_ERROR("Getting dataspace ID for %s", EPHEMERIS_RECORD_NAME);
        H5Tclose(record);
        H5Tclose(vector);
        H5Dclose(dataset);
        return ERROR;
    }

    ndims = H5Sget_simple_extent_ndims(space);
    if (ndims != 1)
    {
        IAS_LOG_ERROR("Invalid ephemeris record dimensions for dataspace %i",
                space);
        H5Tclose(record);
        H5Tclose(vector);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    status = H5Sget_simple_extent_dims(space, dims, NULL);
    if (status < 0)
    {
        IAS_LOG_ERROR("Getting dimension for dataspace %i", space);
        H5Tclose(record);
        H5Tclose(vector);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    if (dims[0] <= 0)
    {
        IAS_LOG_ERROR("Invalid dimension for dataspace %i", space);
        H5Tclose(record);
        H5Tclose(vector);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }
    ephem->sample_count = dims[0];

    /* Allocate memory for the data */
    ephem->sample_records = malloc(sizeof(IAS_SC_EPHEMERIS_RECORD)
            * ephem->sample_count);
    if (!ephem->sample_records)
    {
        IAS_LOG_ERROR("Allocating model ephemeris sample records");
        H5Tclose(record);
        H5Tclose(vector);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    /* Read in the data */
    status = H5Dread(dataset, record, space, H5S_ALL,
            H5P_DEFAULT, ephem->sample_records);
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading the image model ephemeris sample records for %s",
                EPHEMERIS_RECORD_NAME);
        free(ephem->sample_records);
        ephem->sample_records = NULL;
        H5Tclose(record);
        H5Tclose(vector);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    status = H5Tclose(record);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing compound record type for %s",
                EPHEMERIS_RECORD_NAME);
        H5Tclose(vector);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }
    status = H5Tclose(vector);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing compound vector type for %s",
                EPHEMERIS_RECORD_NAME);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }
    status = H5Sclose(space);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing dataspace ID for %s", EPHEMERIS_RECORD_NAME);
        H5Dclose(dataset);
        return ERROR;
    }
    status = H5Dclose(dataset);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing dataset %s", EPHEMERIS_RECORD_NAME);
        return ERROR;
    }

    return SUCCESS;
}

/*************************************************************************
NAME: ias_model_read_jitter_table

PURPOSE: Routine to read jitter table records from HDF5 file to struct

RETURNS: SUCCESS -- successfully retrieved records to structure
         ERROR -- error reading HDF5 file
**************************************************************************/
int ias_model_read_jitter_table
(
    hid_t file,                 /* I: An open HDF5 file handle */
    IAS_SENSOR_MODEL *sensor    /* O: Jitter structure to fill */
)
{
    hid_t group_id;
    hid_t vector;       /* Identifier for compound VECTOR type */
    hid_t dataset;      /* Identifier for the dataset */
    hid_t space;        /* Identifier for the data space */
    hsize_t ndims;      /* Number of row dimensions in array */
    hsize_t dims[2];    /* Number of column dimensions in array */
    int sensor_id;      /* Current sensor ID reading */
    int status;         /* Return status of the HDF read routine */

    /* Open the jitter model group */
    group_id = H5Gopen(file, JITTER_MODEL_GROUP_NAME, H5P_DEFAULT);
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

        /* Verify the dimensions on the jitter entries value */
        dims[0] = 1;
        if (verify_dataset_size(group_id, jit_entries_name, 1, dims)
                != SUCCESS)
        {
            IAS_LOG_ERROR("In the dimensions for jitter entries per frame in "
                    "group %s", JITTER_MODEL_GROUP_NAME);
            return ERROR;
        }

        /* Get the jitter entries per frame */
        if (H5LTread_dataset_int(group_id, jit_entries_name,
                    &sensor->sensors[sensor_id].jitter_entries_per_frame) < 0)
        {
            IAS_LOG_ERROR("Reading jitter entries per frame");
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

        /* Open the dataset */
        dataset = H5Dopen(group_id, jit_table_name, H5P_DEFAULT);
        if (dataset < 1)
        {
            IAS_LOG_ERROR("Opening jitter table record dataset %s",
                    jit_table_name);
            H5Tclose(vector);
            H5Gclose(group_id);
            return ERROR;
        }

        /* Get the data space and its dimensions */
        space = H5Dget_space(dataset);
        if (space < 0)
        {
            IAS_LOG_ERROR("Getting dataspace ID for %s", jit_table_name);
            H5Dclose(dataset);
            H5Tclose(vector);
            H5Gclose(group_id);
            return ERROR;
        }

        ndims = H5Sget_simple_extent_ndims(space);
        if (ndims != 1)
        {
            IAS_LOG_ERROR("Invalid jitter table record dimensions for "
                    "dataspace %i", space);
            H5Sclose(space);
            H5Dclose(dataset);
            H5Tclose(vector);
            H5Gclose(group_id);
            return ERROR;
        }

        status = H5Sget_simple_extent_dims(space, dims, NULL);
        if (status < 0)
        {
            IAS_LOG_ERROR("Getting dimension for dataspace %i", space);
            H5Sclose(space);
            H5Dclose(dataset);
            H5Tclose(vector);
            H5Gclose(group_id);
            return ERROR;
        }

        if (dims[0] <= 0)
        {
            IAS_LOG_ERROR("Invalid dimension for dataspace %i", space);
            H5Sclose(space);
            H5Dclose(dataset);
            H5Tclose(vector);
            H5Gclose(group_id);
            return ERROR;
        }
        sensor->sensors[sensor_id].jitter_table_count = dims[0];

        /* Allocate memory for the data */
        sensor->sensors[sensor_id].jitter_table
            = (IAS_VECTOR *)malloc(sizeof(IAS_VECTOR)
                    * sensor->sensors[sensor_id].jitter_table_count);
        if (!sensor->sensors[sensor_id].jitter_table)
        {
            IAS_LOG_ERROR("Allocating jitter table records");
            H5Sclose(space);
            H5Dclose(dataset);
            H5Tclose(vector);
            H5Gclose(group_id);
            return ERROR;
        }

        /* Read in the data */
        status = H5Dread(dataset, vector, space, H5S_ALL, H5P_DEFAULT,
                sensor->sensors[sensor_id].jitter_table);
        if (status < 0)
        {
            IAS_LOG_ERROR("Reading the sensor model jitter table records "
                    "for %s", jit_table_name);
            H5Sclose(space);
            H5Dclose(dataset);
            H5Tclose(vector);
            H5Gclose(group_id);
            return ERROR;
        }

        status = H5Sclose(space);
        if (status < 0)
        {
            IAS_LOG_ERROR("Closing dataspace ID for %s", jit_table_name);
            H5Dclose(dataset);
            H5Tclose(vector);
            H5Gclose(group_id);
            return ERROR;
        }
        status = H5Dclose(dataset);
        if (status < 0)
        {
            IAS_LOG_ERROR("Closing dataset %s", jit_table_name);
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
NAME: ias_model_read_ssm_models

PURPOSE: Internal routine to read the SSM data from the HDF5 file for sensors
    that have SSM data.

RETURNS: SUCCESS -- successfully retrieved desired data
         ERROR -- error reading HDF5 file
**************************************************************************/
int ias_model_read_ssm_models
(
    hid_t file,                     /* I: An open HDF5 file handle */
    IAS_SENSOR_MODEL *sensor        /* I/O: SSM data to read */
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

        /* open the sensor's scene select mirror group */
        group_id = H5Gopen(file, group_name, H5P_DEFAULT);
        if (group_id < 0) 
        { 
            IAS_LOG_ERROR("Opening SSM group %s", group_name);
            return ERROR; 
        } 

        /* read the epoch time from the group */
        if (read_time_header(group_id, ssm->utc_epoch_time,
                    DONOT_READ_SAMPLE_TIME, NULL) != SUCCESS)
        {
            IAS_LOG_ERROR("Reading SSM epoch time from group %s", group_name);
            H5Gclose(group_id);
            return ERROR; 
        }

        /* verify the dimensions on the alignment matrix */
        dims[0] = TRANSFORMATION_MATRIX_DIM;
        dims[1] = TRANSFORMATION_MATRIX_DIM;
        if (verify_dataset_size(group_id, ALIGNMENT_MATRIX, 2, dims) != SUCCESS)
        {
            IAS_LOG_ERROR("In the dimensions for SSM alignment matrix in "
                          "group %s", group_name);
            return ERROR;
        }

        /* read the alignment matrix time */
        if (H5LTread_dataset_double(group_id, ALIGNMENT_MATRIX,
                    &ssm->alignment_matrix[0][0]) < 0)
        {
            IAS_LOG_ERROR("Reading UTC epoch time for the attitude");
            return ERROR;
        }

        /* read the scene select mirror records */
        if (read_ssm_records(group_id, ssm) != SUCCESS)
        {
            IAS_LOG_ERROR("Reading SSM records from %s", group_name);
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
NAME: read_ssm_records

PURPOSE: Internal routine to read ssm records from HDF5 file to structure

RETURNS: SUCCESS -- successfully retrieved records to structure
         ERROR -- error reading HDF5 file
**************************************************************************/
static int read_ssm_records
(
    hid_t group_id,             /* I: An open HDF5 group handle */
    IAS_SENSOR_SCENE_SELECT_MIRROR_MODEL *ssm /* I/O: ssm data to read */
)
{
    hid_t space;        /* Identifier for the data space */
    hid_t dataset;      /* Identifier for the dataset */
    hid_t record;       /* Identifier for compound type */
    hsize_t ndims;      /* Number of row dimensions in array */
    hsize_t dims[2];    /* Number of column dimensions in array */
    int status;         /* Return status of the HDF read routine */

    /* create the SSM compound type */
    status = ias_model_create_compound_ssm_type(&record);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Creating compound record type for %s",
                SSM_RECORD_NAME);
        return ERROR;
    }

    /* Open the dataset */
    dataset = H5Dopen(group_id, SSM_RECORD_NAME, H5P_DEFAULT);
    if (dataset < 1)
    {
        IAS_LOG_ERROR("Opening model scene select mirror record dataset %s",
                SSM_RECORD_NAME);
        H5Tclose(record);
        return ERROR;
    }

    /* Get the data space and its dimensions */
    space = H5Dget_space(dataset);
    if (space < 0)
    {
        IAS_LOG_ERROR("Getting dataspace ID for %s", SSM_RECORD_NAME);
        H5Tclose(record);
        H5Dclose(dataset);
        return ERROR;
    }

    ndims = H5Sget_simple_extent_ndims(space);
    if (ndims != 1)
    {
        IAS_LOG_ERROR("Invalid scene select mirror record dimensions for "
                      "dataspace %i", space);
        H5Tclose(record);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    status = H5Sget_simple_extent_dims(space, dims, NULL);
    if (status < 0)
    {
        IAS_LOG_ERROR("Getting dimension for dataspace %i", space);
        H5Tclose(record);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    if (dims[0] <= 0)
    {
        IAS_LOG_ERROR("Invalid dimension for dataspace %i", space);
        H5Tclose(record);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }
    ssm->ssm_record_count = dims[0];

    /* Allocate memory for the data */
    ssm->records = malloc(sizeof(IAS_SENSOR_SSM_RECORD)
                          * ssm->ssm_record_count);
    if (!ssm->records)
    {
        IAS_LOG_ERROR("Allocating model SSM records");
        H5Tclose(record);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    /* Read in the data */
    status = H5Dread(dataset, record, space, H5S_ALL, H5P_DEFAULT,
                     ssm->records);
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading the model SSM records for %s", SSM_RECORD_NAME);
        free(ssm->records);
        ssm->records = NULL;
        H5Tclose(record);
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }

    status = H5Tclose(record);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing compound record type for %s",
                SSM_RECORD_NAME);
        free(ssm->records);
        ssm->records = NULL;
        H5Sclose(space);
        H5Dclose(dataset);
        return ERROR;
    }
    status = H5Sclose(space);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing dataspace ID for %s", SSM_RECORD_NAME);
        free(ssm->records);
        ssm->records = NULL;
        H5Dclose(dataset);
        return ERROR;
    }
    status = H5Dclose(dataset);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing dataset %s", SSM_RECORD_NAME);
        free(ssm->records);
        ssm->records = NULL;
        return ERROR;
    }

    return SUCCESS;
}
