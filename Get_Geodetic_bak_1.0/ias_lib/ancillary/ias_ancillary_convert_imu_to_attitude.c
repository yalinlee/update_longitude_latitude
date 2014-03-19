/**********************************************************************
Name: ias_ancillary_convert_imu_to_attitude

Purpose: Calculate satellite ECI and ECEF positions, and load into
         ancillary attitude structure.

Returns: SUCCESS or ERROR
***********************************************************************/

#include "ias_types.h"
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_structures.h"
#include "ias_math.h"
#include "ias_cpf.h"
#include "ias_geo.h"
#include "ias_ancillary_io.h"
#include "ias_ancillary_private.h"

int ias_ancillary_convert_imu_to_attitude
(
    IAS_CPF *cpf,                    /* I: CPF structure */
    IAS_ACQUISITION_TYPE acq_type,   /* I: Image acquisition type */
    const IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data,
                                     /* I: Pointer to ephem data */
    const double *imu_epoch,         /* I: Year, DOY, SOD for start of IMU */
    const IAS_VECTOR *imu_data,      /* I: Array of valid IMU records */
    const double *imu_time_data,     /* I: Array of IMU seconds since j2000 */
    int imu_count,                   /* I: Number of IMU samples (size of
                                           IMU data arrays) */
    double imu_start_time,           /* I: Time of first quaternion and IMU */
    IAS_ANC_ATTITUDE_DATA *anc_attitude_data /* O: Ancillary attitude data */
)
{
    double current_time;              /* Current time of data field */
    int field;                        /* loop control */
    double euler[3][3];               /* Euler rotational matrix */
    double quat_tolerance;            /* tolerance from the CPF */
    IAS_QUATERNION euler_quat;        /* Euler matrix in quaternion */
    double delta_time;                /* time difference for interpolation */
    double utc_epoch_time_secs_J2000; /* ephemeris time for interpolation */
    int status;                       /* return value */

    const struct IAS_CPF_ANCILLARY_QA_THRESHOLDS *ancil_data;

    /* Read CPF ancillary data */
    ancil_data = ias_cpf_get_ancil_qa_thresholds(cpf);
    if (ancil_data == NULL)
    {
        IAS_LOG_ERROR("Reading CPF ancillary data");
        return ERROR;
    }
    quat_tolerance = ancil_data->quaternion_normalization_outlier_threshold;

    /* Convert the utc_epoch_time from YEAR, DOY, SOD to J2000 seconds */
    status = ias_math_convert_year_doy_sod_to_j2000_seconds(
        anc_ephemeris_data->utc_epoch_time, &utc_epoch_time_secs_J2000);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Converting Year, DOY, SOD %lf, %lf, %lf to J2000"
                " seconds format",
                anc_ephemeris_data->utc_epoch_time[0],
                anc_ephemeris_data->utc_epoch_time[1],
                anc_ephemeris_data->utc_epoch_time[2]);
        return ERROR;
    }

    /* Load ephemeris data into ancillary data structure */
    anc_attitude_data->number_of_samples = imu_count;
    anc_attitude_data->utc_epoch_time[0] = imu_epoch[0];
    anc_attitude_data->utc_epoch_time[1] = imu_epoch[1];
    anc_attitude_data->utc_epoch_time[2] = imu_epoch[2];
    current_time = imu_time_data[0];
    for (field=0; field < imu_count; field++)
    {
        /* Note that Kalman filtering resampled attitude so that
           samples are at integer locations of IAS_ANCILLARY_IMU_TIME */
        anc_attitude_data->records[field].seconds_from_epoch = current_time;
        anc_attitude_data->records[field].roll  = imu_data[field].x;
        anc_attitude_data->records[field].pitch = imu_data[field].y;
        anc_attitude_data->records[field].yaw   = imu_data[field].z;
        if (field < imu_count - 1)
        {
            IAS_VECTOR att_rate;
            IAS_QUATERNION temp_quat;
            IAS_QUATERNION rate_quat;

            /* Convert the current attitude point to a quaternion */
            ias_geo_convert_rpy_to_matrix(&imu_data[field], euler);
            if (ias_math_convert_euler_to_quaternion(quat_tolerance, euler,
                &temp_quat) != SUCCESS)
            {
                IAS_LOG_ERROR("Error calculating quaterion");
                return ERROR;
            }

            /* Invert the sign of the quaternion by conjugation */
            ias_math_conjugate_quaternion(&temp_quat, &euler_quat);

            /* Convert the next attitude point to a quaternion */
            ias_geo_convert_rpy_to_matrix(&imu_data[field + 1], euler);
            if (ias_math_convert_euler_to_quaternion(quat_tolerance, euler,
                &temp_quat) != SUCCESS)
            {
                IAS_LOG_ERROR("Error calculating quaterion");
                return ERROR;
            }

            /* Compute the attitude change as a quaternion */
            ias_math_multiply_quaternions(&euler_quat, &temp_quat, &rate_quat);

            /* Convert the attitude change quaternion to roll-pitch-yaw 
               angles */
            ias_math_convert_quaternion2rpy(&rate_quat, &att_rate);

            /* Compute the angular rate from the angle difference */
            anc_attitude_data->records[field].roll_rate
                = att_rate.x/IAS_ANCILLARY_IMU_TIME;
            anc_attitude_data->records[field].pitch_rate
                = att_rate.y/IAS_ANCILLARY_IMU_TIME;
            anc_attitude_data->records[field].yaw_rate
                = att_rate.z/IAS_ANCILLARY_IMU_TIME;
        }
        else
        {
            anc_attitude_data->records[field].roll_rate  
                = anc_attitude_data->records[field - 1].roll_rate;
            anc_attitude_data->records[field].pitch_rate 
                = anc_attitude_data->records[field - 1].pitch_rate;
            anc_attitude_data->records[field].yaw_rate   
                = anc_attitude_data->records[field - 1].yaw_rate;
        }

        ias_geo_convert_rpy_to_matrix(&imu_data[field], euler);

        /* Compute the difference between the UTC epoch time and the current
           record */
        delta_time = imu_time_data[field]
                          + imu_start_time - utc_epoch_time_secs_J2000;

        /* Get the satellite ECI quaternions.  Note that the euler matrix will
           get updated for lunar and stellar acquisitions. */
        if (ias_ancillary_get_quaternion_at_time(anc_ephemeris_data, acq_type,
                IAS_ECI, delta_time, quat_tolerance,
                euler, &euler_quat) != SUCCESS)
        {
            IAS_LOG_ERROR("Determining ECI quaternion values");
            return ERROR;
        }

        anc_attitude_data->records[field].eci_quaternion[0] 
            = euler_quat.vector.x;
        anc_attitude_data->records[field].eci_quaternion[1] 
            = euler_quat.vector.y;
        anc_attitude_data->records[field].eci_quaternion[2] 
            = euler_quat.vector.z;
        anc_attitude_data->records[field].eci_quaternion[3] 
            = euler_quat.scalar;

        /* get the satellite ECEF quaternions */
        if (ias_ancillary_get_quaternion_at_time(anc_ephemeris_data, acq_type,
                IAS_ECEF, delta_time, quat_tolerance,
                euler, &euler_quat) != SUCCESS)
        {
            IAS_LOG_ERROR("Determining ECEF quaternion values");
            return ERROR;
        }

        anc_attitude_data->records[field].ecef_quaternion[0] 
            = euler_quat.vector.x;
        anc_attitude_data->records[field].ecef_quaternion[1] 
            = euler_quat.vector.y;
        anc_attitude_data->records[field].ecef_quaternion[2] 
            = euler_quat.vector.z;
        anc_attitude_data->records[field].ecef_quaternion[3] 
            = euler_quat.scalar;

        current_time += IAS_ANCILLARY_IMU_TIME;
    }

    return SUCCESS;
}
