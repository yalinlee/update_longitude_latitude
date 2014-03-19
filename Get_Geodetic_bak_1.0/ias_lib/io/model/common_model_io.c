/*************************************************************************
NAME:    common_model contains the following routines:
             ias_model_build_general_description
             ias_model_build_earth_description
             ias_model_build_sensor_description
             ias_model_build_precision_description
             ias_model_build_band_description
             ias_model_build_band_sca_description
             ias_model_create_compound_vector_type
             ias_model_create_compound_attitude_type
             ias_model_create_compound_ephemeris_type
             ias_model_close_the_fields

PURPOSE: Two routines to read and write the LOS model from/to an HDF 5 file

RETURNS: SUCCESS or ERROR
**************************************************************************/
#include "ias_logging.h"
#include "common_model_io.h"

#define NUM_VECTOR_ITEMS        3
#define NUM_ATTITUDE_ITEMS      3
#define NUM_EPHEMERIS_ITEMS     9
#define NUM_SSM_ITEMS           2

/*************************************************************************

NAME: ias_model_build_general_description

PURPOSE: Internal routine to build a description of the general table

RETURNS: SUCCESS -- successfully created the general description table
         ERROR -- error in creating the general description table
**************************************************************************/
int ias_model_build_general_description
(
    size_t offsets[NFIELDS],          /* O: Offsets into data structure */
    const char *field_names[NFIELDS], /* O: Names of the fields */
    hid_t field_types[NFIELDS],       /* O: Data types of the fields */
    hid_t fields_to_close[NFIELDS],   /* O: List of fields to be closed */
    size_t field_sizes[NFIELDS],      /* O: Sizes of the fields */
    int *number_fields                /* O: Total number of fields */
)
{
    int i;                          /* loop control variable */
    int return_status = SUCCESS;    /* Return status of this routine */
    GENERAL_MODEL model;            /* Subset of the LOS model structure
                                       containing members that go into a
                                       general table. */

    /* Clear the fields to close */
    for (i = 0; i < NFIELDS; i++)
        fields_to_close[i] = -1;

    i = 0;
    offsets[i] = HOFFSET(GENERAL_MODEL, satellite_id);
    field_names[i] = "Satellite ID";
    field_types[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(model.satellite_id);

    offsets[++i] = HOFFSET(GENERAL_MODEL, sw_version);
    field_names[i] = "L0R Software Version";
    field_types[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_types[i], sizeof(model.sw_version)) < 0)
        return_status = ERROR;
    fields_to_close[i] = field_types[i];
    field_sizes[i] = sizeof(model.sw_version);

    offsets[++i] = HOFFSET(GENERAL_MODEL, wrs_path);
    field_names[i] = "WRS Path";
    field_types[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(model.wrs_path);

    offsets[++i] = HOFFSET(GENERAL_MODEL, wrs_row);
    field_names[i] = "WRS Row";
    field_types[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(model.wrs_row);

    offsets[++i] = HOFFSET(GENERAL_MODEL, acquisition_type);
    field_names[i] = "Acquisition Type";
    field_types[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(model.acquisition_type);

    offsets[++i] = HOFFSET(GENERAL_MODEL, correction_type);
    field_names[i] = "Correction Type";
    field_types[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(model.correction_type);

    *number_fields = i + 1;

    /* Error checks */
    if (return_status == ERROR)
        IAS_LOG_ERROR("Building model general table definition");
    if (*number_fields > NFIELDS)
    {
        IAS_LOG_ERROR( "Too many fields found - stack probably corrupted");
        return_status = ERROR;
    }
    else if (*number_fields != NUM_GENERAL_FIELDS)
    {
        IAS_LOG_ERROR("Building model general table definition");
        return_status = ERROR;
    }

    if (return_status != SUCCESS)
        ias_model_close_the_fields(fields_to_close);

    return return_status;
}

/*************************************************************************

NAME: ias_model_build_earth_description

PURPOSE: Internal routine to build a description of the earth model table

RETURNS: SUCCESS -- successfully created the earth description table
         ERROR -- error in creating the earth description table
**************************************************************************/
int ias_model_build_earth_description
(
    size_t offsets[NFIELDS],          /* O: Offsets into data structure */
    const char *field_names[NFIELDS], /* O: Names of the fields */
    hid_t field_types[NFIELDS],       /* O: Data types of the fields */
    hid_t fields_to_close[NFIELDS],   /* O: List of fields to be closed */
    size_t field_sizes[NFIELDS],      /* O: Sizes of the fields */
    int *number_fields                /* O: Total number of fields */
)
{
    int i;                              /* loop control variable */
    int return_status = SUCCESS;        /* Return status of this routine */
    IAS_EARTH_CHARACTERISTICS model;    /* Convenience structure for getting
                                           size and offsets */

    /* Clear the fields to close */
    for (i = 0; i < NFIELDS; i++)
        fields_to_close[i] = -1;

    i = 0;
    offsets[i] = HOFFSET(IAS_EARTH_CHARACTERISTICS, ut1_utc_correction);
    field_names[i] = "UT1 - UTC Conversion";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(model.ut1_utc_correction);

    offsets[++i] = HOFFSET(IAS_EARTH_CHARACTERISTICS, pole_wander_x);
    field_names[i] = "Pole Wander X";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(model.pole_wander_x);

    offsets[++i] = HOFFSET(IAS_EARTH_CHARACTERISTICS, pole_wander_y);
    field_names[i] = "Pole Wander Y";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(model.pole_wander_y);

    offsets[++i] = HOFFSET(IAS_EARTH_CHARACTERISTICS, semi_major_axis);
    field_names[i] = "Semi Major Axis";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(model.semi_major_axis);

    offsets[++i] = HOFFSET(IAS_EARTH_CHARACTERISTICS, semi_minor_axis);
    field_names[i] = "Semi Minor Axis";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(model.semi_minor_axis);

    offsets[++i] = HOFFSET(IAS_EARTH_CHARACTERISTICS, eccentricity);
    field_names[i] = "Eccentricity";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(model.eccentricity);

    offsets[++i] = HOFFSET(IAS_EARTH_CHARACTERISTICS, earth_angular_velocity);
    field_names[i] = "Angular Velocity";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(model.earth_angular_velocity);

    offsets[++i] = HOFFSET(IAS_EARTH_CHARACTERISTICS, speed_of_light);
    field_names[i] = "Speed of Light";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(model.speed_of_light);

    *number_fields = i + 1;

    /* Error checks */
    if (*number_fields > NFIELDS)
    {
        IAS_LOG_ERROR("Too many fields found - stack probably corrupted");
        ias_model_close_the_fields(fields_to_close);
        return_status = ERROR;
    }
    else if (*number_fields != NUM_EARTH_FIELDS)
    {
        IAS_LOG_ERROR("Building model earth model table definition");
        ias_model_close_the_fields(fields_to_close);
        return_status = ERROR;
    }

    return return_status;
}

/*************************************************************************

NAME: ias_model_build_sensor_description

PURPOSE: Internal routine to build a description of the sensor table

RETURNS: SUCCESS -- successfully created the sensor description table
         ERROR -- error in creating the sensor description table
**************************************************************************/
int ias_model_build_sensor_description
(
    size_t offsets[NFIELDS],          /* O: Offsets into data structure */
    const char *field_names[NFIELDS], /* O: Names of the fields */
    hid_t field_types[NFIELDS],       /* O: Data types of the fields */
    hid_t fields_to_close[NFIELDS],   /* O: List of fields to be closed */
    size_t field_sizes[NFIELDS],      /* O: Sizes of the fields */
    int *number_fields                /* O: Total number of fields */
)
{
    int i;                          /* loop control variable */
    int return_status = SUCCESS;    /* Return status of this routine */
    SENSOR_LOCATION_MODEL model;    /* Convenience structure for getting size
                                       and offsets */
    hsize_t parm_dims[2];           /* Dimension of array field */

    /* Clear the fields to close */
    for (i = 0; i < NFIELDS; i++)
        fields_to_close[i] = -1;

    i = 0;

    offsets[i] = HOFFSET(SENSOR_LOCATION_MODEL, sensor_id);
    field_names[i] = "Sensor ID";
    field_types[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(model.sensor_id);

    offsets[++i] = HOFFSET(SENSOR_LOCATION_MODEL, sensor_present);
    field_names[i] = "Sensor Present Flag";
    field_types[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(model.sensor_present);

    offsets[++i] = HOFFSET(SENSOR_LOCATION_MODEL, sensor2acs);
    field_names[i] = "Sensor to ACS Transformation";
    parm_dims[0] = 3;
    parm_dims[1] = 3;
    field_types[i] = H5Tarray_create(H5T_NATIVE_DOUBLE, 2, parm_dims);
    if (field_types[i] < 0)
        return_status = ERROR;
    fields_to_close[i] = field_types[i];
    field_sizes[i] = sizeof(model.sensor2acs);

    offsets[++i] = HOFFSET(SENSOR_LOCATION_MODEL, center_mass2sensor_offset);
    field_names[i] = "Offset Between SC Center of Mass and Sensor";
    parm_dims[0] = sizeof(model.center_mass2sensor_offset)
        / sizeof(model.center_mass2sensor_offset[0]);
    field_types[i] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, parm_dims);
    if (field_types[i] < 0)
        return_status = ERROR;
    fields_to_close[i] = field_types[i];
    field_sizes[i] = sizeof(model.center_mass2sensor_offset);

    offsets[++i] = HOFFSET(SENSOR_LOCATION_MODEL, ssm_model_present);
    field_names[i] = "Scene Select Mirror Model Present Flag";
    field_types[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(model.ssm_model_present);

    *number_fields = i + 1;

    /* Error checks */
    if (return_status == ERROR)
        IAS_LOG_ERROR("Building model sensor table definition");
    if (*number_fields > NFIELDS)
    {
        IAS_LOG_ERROR("Too many fields found - stack probably corrupted");
        return_status = ERROR;
    }
    else if (*number_fields != NUM_SENSOR_FIELDS)
    {
        IAS_LOG_ERROR("Building model sensor table definition");
        return_status = ERROR;
    }

    if (return_status != SUCCESS)
        ias_model_close_the_fields(fields_to_close);

    return return_status;
}

/*************************************************************************

NAME: ias_model_build_precision_description

PURPOSE: Internal routine to build a description of the precision table

RETURNS: SUCCESS -- successfully created the precision description table
         ERROR -- error in creating the precision description table
**************************************************************************/
int ias_model_build_precision_description
(
    size_t offsets[NFIELDS],          /* O: Offsets into data structure */
    const char *field_names[NFIELDS], /* O: Names of the fields */
    hid_t field_types[NFIELDS],       /* O: Data types of the fields */
    hid_t fields_to_close[NFIELDS],   /* O: List of fields to be closed */
    size_t field_sizes[NFIELDS],      /* O: Sizes of the fields */
    int *number_fields                /* O: Total number of fields */
)
{
    int i;                          /* loop control variable */
    int return_status = SUCCESS;    /* Return status of this routine */
    IAS_SC_PRECISION_MODEL model;   /* Convenience structure for getting size
                                       and offsets */
    hsize_t parm_dims[1];           /* Dimension of array field */

    /* Clear the fields to  close */
    for (i = 0; i < NFIELDS; i++)
        fields_to_close[i] = -1;

    i = 0;
    offsets[i] = HOFFSET(IAS_SC_PRECISION_MODEL, seconds_from_image_epoch);
    field_names[i] = "Seconds From Epoch";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(model.seconds_from_image_epoch);

    offsets[++i] = HOFFSET(IAS_SC_PRECISION_MODEL, ephemeris_order);
    field_names[i] = "Number Ephemeris Correction Coefficients";
    field_types[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(model.ephemeris_order);

    offsets[++i] = HOFFSET(IAS_SC_PRECISION_MODEL, x_corr);
    field_names[i] = "Ephemeris Correction X Direction";
    parm_dims[0] = sizeof(model.x_corr) / sizeof(model.x_corr[0]);
    field_types[i] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, parm_dims);
    if (field_types[i] < 0)
        return_status = ERROR;
    fields_to_close[i] = field_types[i];
    field_sizes[i] = sizeof(model.x_corr);

    offsets[++i] = HOFFSET(IAS_SC_PRECISION_MODEL, y_corr);
    field_names[i] = "Ephemeris Correction Y Direction";
    parm_dims[0] = sizeof(model.y_corr) / sizeof(model.y_corr[0]);
    field_types[i] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, parm_dims);
    if (field_types[i] < 0)
        return_status = ERROR;
    fields_to_close[i] = field_types[i];
    field_sizes[i] = sizeof(model.y_corr);

    offsets[++i] = HOFFSET(IAS_SC_PRECISION_MODEL, z_corr);
    field_names[i] = "Ephemeris Correction Z Direction";
    parm_dims[0] = sizeof(model.z_corr) / sizeof(model.z_corr[0]);
    field_types[i] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, parm_dims);
    if (field_types[i] < 0)
        return_status = ERROR;
    fields_to_close[i] = field_types[i];
    field_sizes[i] = sizeof(model.z_corr);

    offsets[++i] = HOFFSET(IAS_SC_PRECISION_MODEL, attitude_order);
    field_names[i] = "Number of Attitude Correction Coefficients";
    field_types[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(model.attitude_order);

    offsets[++i] = HOFFSET(IAS_SC_PRECISION_MODEL, roll_corr);
    field_names[i] = "Ephemeris Roll Correction";
    parm_dims[0] = sizeof(model.roll_corr) / sizeof(model.roll_corr[0]);
    field_types[i] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, parm_dims);
    if (field_types[i] < 0)
        return_status = ERROR;
    fields_to_close[i] = field_types[i];
    field_sizes[i] = sizeof(model.roll_corr);

    offsets[++i] = HOFFSET(IAS_SC_PRECISION_MODEL, pitch_corr);
    field_names[i] = "Ephemeris Pitch Correction";
    parm_dims[0] = sizeof(model.pitch_corr) / sizeof(model.pitch_corr[0]);
    field_types[i] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, parm_dims);
    if (field_types[i] < 0)
        return_status = ERROR;
    fields_to_close[i] = field_types[i];
    field_sizes[i] = sizeof(model.pitch_corr);

    offsets[++i] = HOFFSET(IAS_SC_PRECISION_MODEL, yaw_corr);
    field_names[i] = "Ephemeris Yaw Correction";
    parm_dims[0] = sizeof(model.yaw_corr) / sizeof(model.yaw_corr[0]);
    field_types[i] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, parm_dims);
    if (field_types[i] < 0)
        return_status = ERROR;
    fields_to_close[i] = field_types[i];
    field_sizes[i] = sizeof(model.yaw_corr);

    *number_fields = i + 1;

    /* Error checks */
    if (return_status == ERROR)
        IAS_LOG_ERROR("Building model precision table definition");
    if (*number_fields > NFIELDS)
    {
        IAS_LOG_ERROR("Too many fields found - stack probably corrupted");
        return_status = ERROR;
    }
    else if (*number_fields != NUM_PRECISION_FIELDS)
    {
        IAS_LOG_ERROR("Building model precision table definition");
        return_status = ERROR;
    }

    if (return_status != SUCCESS)
        ias_model_close_the_fields(fields_to_close);

    return return_status;
}

/*************************************************************************

NAME: ias_model_build_band_description

PURPOSE: Internal routine to build a description of the band table

RETURNS: SUCCESS -- successfully created the band description table
         ERROR -- error in creating the band description table
**************************************************************************/
int ias_model_build_band_description
(
    size_t offsets[NFIELDS],          /* O: Offsets into data structure */
    const char *field_names[NFIELDS], /* O: Names of the fields */
    hid_t field_types[NFIELDS],       /* O: Data types of the fields */
    hid_t fields_to_close[NFIELDS],   /* O: List of fields to be closed */
    size_t field_sizes[NFIELDS],      /* O: Sizes of the fields */
    int *number_fields                /* O: Total number of fields */
)
{
    int i;                          /* loop control variable */
    int return_status = SUCCESS;    /* Return status of this routine */
    BAND_SAMP_CHAR_MODEL model;     /* Convenience structure for getting size
                                       and offsets */
    hsize_t parm_dims[1];           /* Dimension of array field */

    /* Clear the fields to close */
    for (i = 0; i < NFIELDS; i++)
        fields_to_close[i] = -1;

    i = 0;

    offsets[i] = HOFFSET(BAND_SAMP_CHAR_MODEL, band_present);
    field_names[i] = "Band Present Flag";
    field_types[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(model.band_present);

    offsets[++i] = HOFFSET(BAND_SAMP_CHAR_MODEL, sca_count);
    field_names[i] = "Number of SCAs";
    field_types[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(model.sca_count);

    offsets[++i] = HOFFSET(BAND_SAMP_CHAR_MODEL, utc_epoch_time);
    field_names[i] = "UTC Epoch Time";
    parm_dims[0] = sizeof(model.utc_epoch_time)
        / sizeof(model.utc_epoch_time[0]);
    field_types[i] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, parm_dims);
    if (field_types[i] < 0)
        return_status = ERROR;
    fields_to_close[i] = field_types[i];
    field_sizes[i] = sizeof(model.utc_epoch_time);

    offsets[++i] = HOFFSET(BAND_SAMP_CHAR_MODEL, integration_time);
    field_names[i] = "Integration Time";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(model.integration_time);

    offsets[++i] = HOFFSET(BAND_SAMP_CHAR_MODEL, sampling_time);
    field_names[i] = "Sampling Time";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(model.sampling_time);

    offsets[++i] = HOFFSET(BAND_SAMP_CHAR_MODEL, lines_per_frame);
    field_names[i] = "Lines per Frame";
    field_types[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(model.lines_per_frame);

    offsets[++i] = HOFFSET(BAND_SAMP_CHAR_MODEL, settling_time);
    field_names[i] = "Settling Time";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(model.settling_time);

    offsets[++i] = HOFFSET(BAND_SAMP_CHAR_MODEL, along_ifov);
    field_names[i] = "Along Track IFOV";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(model.along_ifov);

    offsets[++i] = HOFFSET(BAND_SAMP_CHAR_MODEL, across_ifov);
    field_names[i] = "Across Track IFOV";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(model.across_ifov);

    offsets[++i] = HOFFSET(BAND_SAMP_CHAR_MODEL, maximum_detector_delay);
    field_names[i] = "Maximum Detector Delay";
    field_types[i] = H5T_NATIVE_DOUBLE;
    field_sizes[i] = sizeof(model.maximum_detector_delay);

    offsets[++i] = HOFFSET(BAND_SAMP_CHAR_MODEL, time_codes_at_frame_start);
    field_names[i] = "Time Codes are at Frame Start";
    field_types[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(model.time_codes_at_frame_start);

    offsets[++i] = HOFFSET(BAND_SAMP_CHAR_MODEL, frame_delay);
    field_names[i] = "Frame Times Have Delay";
    field_types[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(model.frame_delay);

    *number_fields = i + 1;

    /* Error checks */
    if (return_status == ERROR)
        IAS_LOG_ERROR("Building model band table definition");
    if (*number_fields > NFIELDS)
    {
        IAS_LOG_ERROR("Too many fields found - stack probably corrupted");
        return_status = ERROR;
    }
    else if (*number_fields != NUM_BAND_SAMP_CHAR_FIELDS)
    {
        IAS_LOG_ERROR("Building model band table definition");
        return_status = ERROR;
    }

    if (return_status != SUCCESS)
        ias_model_close_the_fields(fields_to_close);

    return return_status;
}

/*************************************************************************

NAME: ias_model_build_band_sca_description

PURPOSE: Internal routine to build a description of the sca table within
         the band table

RETURNS: SUCCESS -- successfully created the band/sca description table
         ERROR -- error in creating the band/sca description table
**************************************************************************/
int ias_model_build_band_sca_description
(
    int detectors,                    /* I: Number of detectors on this SCA */
    size_t offsets[NFIELDS],          /* O: Offsets into data structure */
    const char *field_names[NFIELDS], /* O: Names of the fields */
    hid_t field_types[NFIELDS],       /* O: Data types of the fields */
    hid_t fields_to_close[NFIELDS],   /* O: List of fields to be closed */
    size_t field_sizes[NFIELDS],      /* O: Sizes of the fields */
    int *number_fields                /* O: Total number of fields */
)
{
    int i;                          /* loop control variable */
    int return_status = SUCCESS;    /* Return status of this routine */
    SCA_MODEL model;                /* Convenience structure for getting size
                                       and offsets */
    hsize_t parm_dims[1];           /* Dimension of array field */

    /* Clear the fields to close */
    for (i = 0; i < NFIELDS; i++)
        fields_to_close[i] = -1;

    i = 0;
    offsets[i] = HOFFSET(SCA_MODEL, nominal_fill);
    field_names[i] = "Nominal Fill";
    field_types[i] = H5T_NATIVE_INT;
    field_sizes[i] = sizeof(model.nominal_fill);

    offsets[++i] = HOFFSET(SCA_MODEL, sca_coef_x);
    field_names[i] = "Along-track Legendre terms (SCA coefficients X)";
    parm_dims[0] = sizeof(model.sca_coef_x) / sizeof(model.sca_coef_x[0]);
    field_types[i] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, parm_dims);
    if (field_types[i] < 0)
        return_status = ERROR;
    fields_to_close[i] = field_types[i];
    field_sizes[i] = sizeof(model.sca_coef_x);

    offsets[++i] = HOFFSET(SCA_MODEL, sca_coef_y);
    field_names[i] = "Across-track Legendre terms (SCA coefficients Y)";
    parm_dims[0] = sizeof(model.sca_coef_y) / sizeof(model.sca_coef_y[0]);
    field_types[i] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, parm_dims);
    if (field_types[i] < 0)
        return_status = ERROR;
    fields_to_close[i] = field_types[i];
    field_sizes[i] = sizeof(model.sca_coef_y);

    *number_fields = i + 1;

    /* Error checks */
    if (return_status == ERROR)
        IAS_LOG_ERROR("Building model sca table definition");
    if (*number_fields > NFIELDS)
    {
        IAS_LOG_ERROR("Too many fields found - stack probably corrupted");
        return_status = ERROR;
    }
    else if (*number_fields != NUM_SCA_FIELDS)
    {
        IAS_LOG_ERROR("Building model sca table definition");
        return_status = ERROR;
    }

    if (return_status != SUCCESS)
        ias_model_close_the_fields(fields_to_close);

    return return_status;
}

/*************************************************************************

NAME: ias_model_create_compound_vector_type

PURPOSE: Local routine to create an HDF compound data type of IAS_VECTOR.
         It is the caller's responsibility to close the ID when no longer
         needed.

RETURNS: SUCCESS or ERROR
**************************************************************************/
int ias_model_create_compound_vector_type
(
    hid_t *vector                       /* O: Vector inserted */
)
{
    herr_t insert_status[NUM_VECTOR_ITEMS];
    hid_t local_vector;
    int i;

    local_vector = H5Tcreate(H5T_COMPOUND, sizeof(IAS_VECTOR));
    if (local_vector < 0)
    {
        IAS_LOG_ERROR("Cannot create IAS_VECTOR memory datatype");
        return ERROR;
    }

    insert_status[0] = H5Tinsert(local_vector, "X", HOFFSET(IAS_VECTOR, x),
            H5T_NATIVE_DOUBLE);
    insert_status[1] = H5Tinsert(local_vector, "Y", HOFFSET(IAS_VECTOR, y),
            H5T_NATIVE_DOUBLE);
    insert_status[2] = H5Tinsert(local_vector, "Z", HOFFSET(IAS_VECTOR, z),
            H5T_NATIVE_DOUBLE);

    /* Check for insert errors */
    for (i = 0; i < NUM_VECTOR_ITEMS; i++)
    {
        if (insert_status[i] < 0)
        {
            IAS_LOG_ERROR("Could not create IAS_VECTOR memory file datatype -- "
                    "one or more members failed to insert");
            H5Tclose(local_vector);
            return ERROR;
        }
    }

    *vector = local_vector;

    return SUCCESS;
}

/*************************************************************************

NAME: ias_model_create_compound_attitude_type

PURPOSE: Local routine to create an HDF compound data type of
         IAS_SC_ATTITUDE_RECORD. It is the caller's responsibility to close
         the ID when no longer needed.

RETURNS: SUCCESS or ERROR
**************************************************************************/
int ias_model_create_compound_attitude_type
(
    hid_t vector,       /* I: An open ID of an IAS_VECTOR compound data type */
    hid_t *record       /* O: Record inserted */
)
{
    herr_t insert_status[NUM_ATTITUDE_ITEMS];
    hid_t local_record;
    int i;

    /* Create a nested data structure for the attitude record */
    local_record = H5Tcreate(H5T_COMPOUND, sizeof(IAS_SC_ATTITUDE_RECORD));
    if (local_record < 0)
    {
        IAS_LOG_ERROR("Cannot create IAS_SC_ATTITUDE_RECORD memory datatype");
        return ERROR;
    }

    insert_status[0] = H5Tinsert(local_record, "Time From Epoch",
            HOFFSET(IAS_SC_ATTITUDE_RECORD, seconds_from_epoch),
            H5T_NATIVE_DOUBLE);
    insert_status[1] = H5Tinsert(local_record, "Satellite Attitude",
            HOFFSET(IAS_SC_ATTITUDE_RECORD, satellite_attitude),
            vector);
    insert_status[2] = H5Tinsert(local_record, "Precision Attitude",
            HOFFSET(IAS_SC_ATTITUDE_RECORD, precision_attitude),
            vector);

    /* Check for insert errors */
    for (i = 0; i < NUM_ATTITUDE_ITEMS; i++)
    {
        if (insert_status[i] < 0)
        {
            IAS_LOG_ERROR("Could not create IAS_SC_ATTITUDE_RECORD memory file "
                    "datatype -- one or more members failed to insert");
            H5Tclose(local_record);
            return ERROR;
        }
    }

    *record = local_record;

    return SUCCESS;
}

/*************************************************************************

NAME: ias_model_create_compound_ephemeris_type

PURPOSE: Local routine to create an HDF compound data type of
         IAS_SC_EPHEMERIS_RECORD. It is the caller's responsibility to close
         the ID when no longer needed.

RETURNS: SUCCESS or ERROR
**************************************************************************/
int ias_model_create_compound_ephemeris_type
(
    hid_t vector,       /* I: An open ID of an IAS_VECTOR compound data type */
    hid_t *record       /* O: Record inserted */
)
{
    herr_t insert_status[NUM_EPHEMERIS_ITEMS];
    hid_t local_record;
    int i;

    local_record = H5Tcreate(H5T_COMPOUND, sizeof(IAS_SC_EPHEMERIS_RECORD));
    if (local_record < 0)
    {
        IAS_LOG_ERROR("Cannot create IAS_SC_EPHEMERIS_RECORD memory datatype");
        return ERROR;
    }

    insert_status[0] = H5Tinsert(local_record, "Time From Epoch",
            HOFFSET(IAS_SC_EPHEMERIS_RECORD, seconds_from_epoch),
            H5T_NATIVE_DOUBLE);
    insert_status[1] = H5Tinsert(local_record, "ECI Position",
            HOFFSET(IAS_SC_EPHEMERIS_RECORD, eci_position), vector);
    insert_status[2] = H5Tinsert(local_record, "ECI Velocity",
            HOFFSET(IAS_SC_EPHEMERIS_RECORD, eci_velocity), vector);
    insert_status[3] = H5Tinsert(local_record, "ECEF Position",
            HOFFSET(IAS_SC_EPHEMERIS_RECORD, ecef_position), vector);
    insert_status[4] = H5Tinsert(local_record, "ECEF Velocity",
            HOFFSET(IAS_SC_EPHEMERIS_RECORD, ecef_velocity), vector);
    insert_status[5] = H5Tinsert(local_record, "Precision ECI Position",
            HOFFSET(IAS_SC_EPHEMERIS_RECORD, precision_eci_position), vector);
    insert_status[6] = H5Tinsert(local_record, "Precision ECI Velocity",
            HOFFSET(IAS_SC_EPHEMERIS_RECORD, precision_eci_velocity), vector);
    insert_status[7] = H5Tinsert(local_record, "Precision ECEF Position",
            HOFFSET(IAS_SC_EPHEMERIS_RECORD, precision_ecef_position), vector);
    insert_status[8] = H5Tinsert(local_record, "Precision ECEF Velocity",
            HOFFSET(IAS_SC_EPHEMERIS_RECORD, precision_ecef_velocity), vector);

    /* Check for insert errors */
    for (i = 0; i < NUM_EPHEMERIS_ITEMS; i++)
    {
        if (insert_status[i] < 0)
        {
            IAS_LOG_ERROR("Could not create IAS_SC_ATTITUDE_RECORD memory file "
                    "datatype -- one or more members failed to insert");
            H5Tclose(local_record);
            return ERROR;
        }
    }

    *record = local_record;

    return SUCCESS;
}

/*************************************************************************

NAME: ias_model_create_compound_ssm_type

PURPOSE: Local routine to create an HDF compound data type of
         IAS_SENSOR_SSM_RECORD. It is the caller's responsibility to close
         the ID when no longer needed.

RETURNS: SUCCESS or ERROR

**************************************************************************/
int ias_model_create_compound_ssm_type
(
    hid_t *record       /* O: Record created */
)
{
    herr_t insert_status[NUM_SSM_ITEMS];
    hid_t local_record;
    int i;

    /* Create a nested data structure for the attitude record */
    local_record = H5Tcreate(H5T_COMPOUND, sizeof(IAS_SENSOR_SSM_RECORD));
    if (local_record < 0)
    {
        IAS_LOG_ERROR("Cannot create IAS_SENSOR_SSM_RECORD memory datatype");
        return ERROR;
    }

    insert_status[0] = H5Tinsert(local_record, "Seconds From Epoch",
            HOFFSET(IAS_SENSOR_SSM_RECORD, seconds_from_epoch),
            H5T_NATIVE_DOUBLE);
    insert_status[1] = H5Tinsert(local_record, "Mirror Angle",
            HOFFSET(IAS_SENSOR_SSM_RECORD, mirror_angle), H5T_NATIVE_DOUBLE);

    /* Check for insert errors */
    for (i = 0; i < NUM_SSM_ITEMS; i++)
    {
        if (insert_status[i] < 0)
        {
            IAS_LOG_ERROR("Could not create IAS_SENSOR_SSM_RECORD memory file "
                    "datatype -- one or more members failed to insert");
            H5Tclose(local_record);
            return ERROR;
        }
    }

    *record = local_record;

    return SUCCESS;
}

/*************************************************************************

NAME: ias_model_close_the_fields

PURPOSE: Local routine to close any of the fields that were used

RETURNS: Nothing
**************************************************************************/
void ias_model_close_the_fields
(
    hid_t fields_to_close[NFIELDS] /* I: Array of field identifiers */
)
{
    int i;      /* Loop control */

    for (i = 0; i < NFIELDS; i++)
    {
        if (fields_to_close[i] >= 0)
        {
            if (H5Tclose(fields_to_close[i]) < 0)
                IAS_LOG_WARNING("Closing HDF fields");
        }
    }
}
