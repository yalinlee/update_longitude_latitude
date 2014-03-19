/*****************************************************************************
NAME: ias_ancillary_preprocess_attitude

PURPOSE: Processes through the attitude data.

RETURNS: Type = int
    Value    Description
    -----    -----------
    SUCCESS  Successful completion
    ERROR    Operation failed

MEMORY MANAGEMENT:

    Memory Passed Back To Parent Routine:
        anc_attitude_data

*****************************************************************************/

#include <stdlib.h>

#include "ias_types.h"
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_cpf.h"
#include "ias_ancillary.h"
#include "ias_ancillary_private.h"

int ias_ancillary_preprocess_attitude
(
    IAS_CPF *cpf,                     /* I: CPF structure */
    const IAS_L0R_ATTITUDE *l0r_attitude, /* I: L0R attitude structure */
    int l0r_attitude_count,           /* I: Number of attitude records in L0R */
    const IAS_L0R_IMU *l0r_imu,       /* I: IMU data */
    int l0r_imu_count,                /* I: IMU record count */
    const double *interval_start_time,/* I: Interval start YEAR, DOY, SOD */
    const double *interval_stop_time, /* I: Interval stop YEAR, DOY, SOD */
    const IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data,
                                      /* I: Pointer to ephem data */
    IAS_ACQUISITION_TYPE acq_type,    /* I: Image acquisition type */
    IAS_ANC_ATTITUDE_DATA **anc_attitude_data, /* O: Pointer to ancillary
                                                     attitude data */
    int *invalid_attitude_count       /* O: Number of invalid attitude points
                                            detected */
)
{
    double imu_epoch[3];                 /* Year, DOY, SOD for start of IMU */
    double eph_start_time;               /* Ephemeris start time */
    double eph_stop_time;                /* Ephemeris stop time */
    int imu_count;                       /* Number of valid gryo records */
    int original_imu_count;              /* Original # of valid gryo records */
    IAS_VECTOR *imu_data = NULL;         /* Array of IMU records */
    double *imu_time_data = NULL;        /* Array of IMU time records */
    IAS_QUATERNION *quaternion_data = NULL; /* Array of valid quaternions */
    double *quaternion_time_data = NULL; /* Array of quaternion times */
    int valid_quaternion_count;          /* Number of valid quaternions */
    int *valid_imu_flag = NULL;          /* Array of IMU quality flag */
    int *valid_quaternion_flag = NULL;   /* Array of quaternion quality flag */
    int invalid_imu_count;               /* Number of invalid IMU samples */
    int invalid_quaternion_count;        /* Number of invalid quaternions */
    double imu_start_time;               /* Time of first IMU */
    double percentage_bad_attitude_points; /* Percentage of attitude points
                                            that are invalid */
    int imu_valid = 0;                   /* Flag for valid/invalid IMU data */
    int quaternion_array_count;          /* Count of entries allocated for the
                                            quaternion related arrays */
    int interpolate_quaternions_flag = 0;/* flag to indicate there were missing
                                            quaternions to interpolate using
                                            Kalman filtering */

    *anc_attitude_data = NULL;

    /* Set the count of entries to allocate for the quaternion arrays to
       include one additional record of samples found in the L0R data.  This
       is to support the potential for having a missing record that needs to
       be interpolated. */
    quaternion_array_count = l0r_attitude_count
        + IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD;

    /* Allocate structure that will contain valid quaternion data */
    quaternion_data = (IAS_QUATERNION *)malloc(sizeof(IAS_QUATERNION)
        * quaternion_array_count);
    if (quaternion_data == NULL)
    {
        IAS_LOG_ERROR("Allocating quaternions");
        return ERROR;
    }

    /* Allocate structure that will contain valid quaternion time data */
    quaternion_time_data = (double *)malloc( sizeof(double)
        * quaternion_array_count );
    if (quaternion_time_data == NULL)
    {
        IAS_LOG_ERROR("Allocating quaternion time");
        free(quaternion_data);
        return ERROR;
    }

    /* Allocate array for quaternion quality flags */
    valid_quaternion_flag = (int *)malloc(sizeof(int) * quaternion_array_count);
    if (valid_quaternion_flag == NULL)
    {
        IAS_LOG_ERROR("Allocating quaternion quality flag");
        free(quaternion_data);
        free(quaternion_time_data);
        return ERROR;
    }

    /* Identify quaternion outliers */
    if (ias_ancillary_identify_quaternion_outliers(
            cpf, l0r_attitude, l0r_attitude_count, quaternion_time_data,
            quaternion_data, valid_quaternion_flag, quaternion_array_count,
            &valid_quaternion_count, &interpolate_quaternions_flag)
            != SUCCESS)
    {
        IAS_LOG_ERROR("Removing quaternion outliers");
        free(quaternion_data);
        free(quaternion_time_data);
        free(valid_quaternion_flag);
        return ERROR;
    }

    /* Allocate structure that will contain valid IMU data */
    imu_data = (IAS_VECTOR *)malloc(sizeof(IAS_VECTOR)
            * l0r_imu_count * IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD);
    if (imu_data == NULL)
    {
        IAS_LOG_ERROR("Allocating memory for imu_data");
        free(quaternion_data);
        free(quaternion_time_data);
        free(valid_quaternion_flag);
        return ERROR;
    }

    /* allocate structure that will contain valid IMU time stamps */
    imu_time_data = (double *)malloc(sizeof(double)
            * l0r_imu_count * IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD);
    if (imu_time_data == NULL)
    {
        IAS_LOG_ERROR("Allocating memory for imu_time_data");
        free(quaternion_data);
        free(quaternion_time_data);
        free(valid_quaternion_flag);
        free(imu_data);
        return ERROR;
    }

    /* Allocate structure that will contain IMU quality flag. */
    valid_imu_flag = (int *)malloc(sizeof(int) * l0r_imu_count
            * IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD);
    if (valid_imu_flag == NULL)
    {
        IAS_LOG_ERROR("Allocating IMU quality flag");
        free(quaternion_data);
        free(quaternion_time_data);
        free(valid_quaternion_flag);
        free(imu_data);
        free(imu_time_data);
        return ERROR;
    }

    /* Rotate the IMU data to the attitude control system coordinates
         and converts from arc seconds to radians. */
    if (ias_ancillary_convert_imu_to_acs(
            cpf, l0r_imu, l0r_imu_count, valid_quaternion_count,
            quaternion_time_data, imu_data, imu_time_data, valid_imu_flag,
            &imu_valid, &imu_count) != SUCCESS)
    {
        IAS_LOG_ERROR("Converting IMU to ACS");
        free(quaternion_data);
        free(quaternion_time_data);
        free(valid_quaternion_flag);
        free(imu_data);
        free(imu_time_data);
        free(valid_imu_flag);
        return ERROR;
    }

    /* Window the IMU data so that the IMU falls inside the ephemeris data.
       Window the quaternion data so that it falls inside the IMU data. */
    if (ias_ancillary_extract_valid_imu_data_window(
            anc_ephemeris_data, imu_time_data, imu_data, valid_imu_flag,
            &imu_count, imu_epoch, &eph_start_time, &eph_stop_time,
            &invalid_imu_count) != SUCCESS)
    {
        IAS_LOG_ERROR("Computing IMU data window");
        free(quaternion_data);
        free(quaternion_time_data);
        free(valid_quaternion_flag);
        free(imu_data);
        free(imu_time_data);
        free(valid_imu_flag);
        return ERROR;
    }

    /* Extract the quaternion data window. */
    if (ias_ancillary_extract_valid_quaternion_window(
            imu_time_data, imu_count, quaternion_data,
            quaternion_time_data, valid_quaternion_flag,
            &valid_quaternion_count, &invalid_quaternion_count) != SUCCESS)
    {
        IAS_LOG_ERROR("Computing quaternion data window");
        free(quaternion_data);
        free(quaternion_time_data);
        free(valid_quaternion_flag);
        free(imu_data);
        free(imu_time_data);
        free(valid_imu_flag);
        return ERROR;
    }

    /* Compute the spacecraft attitude. */
    original_imu_count = imu_count;
    if (ias_ancillary_compute_spacecraft_attitude(
            anc_ephemeris_data, &imu_count, quaternion_data,
            valid_quaternion_count, eph_start_time, eph_stop_time,
            interval_start_time, interval_stop_time, acq_type, imu_valid,
            interpolate_quaternions_flag, imu_epoch, imu_time_data,
            quaternion_time_data, valid_imu_flag, valid_quaternion_flag,
            imu_data, &imu_start_time) != SUCCESS)
    {
        IAS_LOG_ERROR("Computing spacecraft attitude");
        free(quaternion_data);
        free(quaternion_time_data);
        free(valid_quaternion_flag);
        free(imu_data);
        free(imu_time_data);
        free(valid_imu_flag);
        return ERROR;
    }

    /* Allocate ancillary data structure */
    *anc_attitude_data = ias_ancillary_allocate_attitude(imu_count);
    if (*anc_attitude_data == NULL)
    {
        IAS_LOG_ERROR("Allocating ancillary attitude records");
        return ERROR;
    }

    /* Calculate satellite ECI and ECEF positions, and load into
       ancillary attitude structure. */
    if (ias_ancillary_convert_imu_to_attitude(cpf, acq_type,
            anc_ephemeris_data, imu_epoch, imu_data, imu_time_data,
            imu_count, imu_start_time, *anc_attitude_data) != SUCCESS)
    {
        IAS_LOG_ERROR("Converting IMU to attitude");
        free(quaternion_data);
        free(quaternion_time_data);
        free(valid_quaternion_flag);
        free(imu_data);
        free(imu_time_data);
        free(valid_imu_flag);
        return ERROR;
    }

    /* Report out the total number of invalid points found */
    *invalid_attitude_count = invalid_imu_count + invalid_quaternion_count;

    /* Give a warning if the percentage of bad attitude points is too high */
    percentage_bad_attitude_points = 100.0 *
        ((double)*invalid_attitude_count / original_imu_count);
    if (percentage_bad_attitude_points > 5.0)
    {
        IAS_LOG_WARNING("The percentage of bad attitude points %.1f is greater "
            "than threshold 5 percent (total points %d, bad IMU points %d, "
            "bad quaternion points %d, total bad points %d)",
            percentage_bad_attitude_points, original_imu_count,
            invalid_imu_count, invalid_quaternion_count,
            *invalid_attitude_count);
    }

    /* free allocated memory */
    free(quaternion_data);
    free(quaternion_time_data);
    free(valid_quaternion_flag);
    free(imu_data);
    free(imu_time_data);
    free(valid_imu_flag);

    return SUCCESS;
}
