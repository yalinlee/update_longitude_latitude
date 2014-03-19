/**********************************************************************
Name: ias_ancillary_compute_spacecraft_attitude

Purpose: Compute the spacecraft attitude.

Returns: Type = int
    Value    Description
    -----    -----------
    SUCCESS  Successful completion 
    ERROR    Operation failed
***********************************************************************/

#include <stdlib.h>
#include <math.h>

#include "ias_types.h"
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_structures.h"
#include "ias_math.h"
#include "ias_ancillary_io.h"
#include "ias_ancillary_private.h"

/* local routine prototype */
static int compute_spacecraft_attitude_at_time
(
    IAS_ACQUISITION_TYPE acq_type, /* I: Image acquisition type */
    const IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data,
                                   /* I: pointer to ephem data */
    const IAS_QUATERNION *quat,    /* I: Quaternion vector */
    double qtime,                  /* I: Quaternion time (seconds from J2000) */
    IAS_VECTOR *attitude           /* O: Roll, pitch, yaw */
);


int ias_ancillary_compute_spacecraft_attitude
(
    const IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data,
                                   /* I: Pointer to ephem data */
    int *imu_count,                /* I/O: Number of IMU records (size of
                                         IMU data arrays) */
    const IAS_QUATERNION *valid_quaternion_data,
                                   /* I: Array of valid quaternions */
    int valid_quaternion_count,    /* I: Number of valid quaternions (size
                                         of quaternion data arrays) */
    double eph_start_time,         /* I: Ephemeris start seconds from J2000 */
    double eph_stop_time,          /* I: Ephemeris stop seconds from J2000 */
    const double *interval_start_time, /* I: interval start YEAR, DOY, SOD */
    const double *interval_stop_time,  /* I: interval stop YEAR, DOY, SOD */
    IAS_ACQUISITION_TYPE acq_type, /* I: Image acquisition type */
    int imu_valid,                 /* I: Flag for valid/invalid IMU data */
    int interpolate_quaternions_flag,/* I: flag that indicates there are
                                           missing quaternions to interpolate
                                           using Kalman filtering */
    double *imu_epoch,             /* I/O: Year, DOY, SOD for start of IMU */
    double *imu_time_data,         /* I/O: Array of IMU seconds since j2000 
                                      on input, seconds since epoch on output */
    double *quaternion_time_data,  /* I/O: Array of quat seconds since j2000
                                      on input, seconds since epoch on output */
    int *valid_imu_flag,           /* I/O: Array of IMU data quality flags */
    int *valid_quaternion_flag,    /* I/O: Array of quaternion quality flags*/
    IAS_VECTOR *imu_data,          /* O: Array of IMU records */
    double *imu_start_time         /* O: Time of first quaternion and IMU */
)
{
    int index;                  /* loop variable */
    IAS_VECTOR mean_att_ref;    /* Mean attitude reference for detrending 
                                   IMU data */
    IAS_VECTOR *attitude_data;  /* Quaternions -> roll, pitch, yaw */
    int number_valid_points;    /* Number of valid attitude points */
    double delta_sample_time;   /* Time between samples */
    double pi = ias_math_get_pi(); /* PI */

#if DEBUG_GENERATE_DATA_FILES == 1
    FILE *fp;
#endif

    IAS_LOG_DEBUG("New bounds IMU seconds since j2000 start %f, end %f",
        imu_time_data[0], imu_time_data[*imu_count - 1]);
    IAS_LOG_DEBUG("New bounds quaternion seconds since j2000 start %f, end %f",
        quaternion_time_data[0],
        quaternion_time_data[valid_quaternion_count - 1]);

    IAS_LOG_DEBUG("Ephemeris UTC epoch time %f %f %f",
        anc_ephemeris_data->utc_epoch_time[0],
        anc_ephemeris_data->utc_epoch_time[1],
        anc_ephemeris_data->utc_epoch_time[2]);
    IAS_LOG_DEBUG("Ephemeris bounds start %f, end %f",
        anc_ephemeris_data->records[0].seconds_from_epoch,
        anc_ephemeris_data->
            records[anc_ephemeris_data->
                number_of_samples - 1].seconds_from_epoch);

    IAS_LOG_DEBUG("IMU epoch time %f %f %f",
        imu_epoch[0], imu_epoch[1], imu_epoch[2]);
    IAS_LOG_DEBUG("IMU time length in j2000 seconds %f",
        imu_time_data[*imu_count - 1] - imu_time_data[0]);

    /* One last IMU check */
    if (imu_time_data[0] < eph_start_time)
    {
        IAS_LOG_ERROR("Ephemeris (%f,%f) data does not cover IMU (%f,%f) -> "
                      "Start time",
                      eph_start_time, eph_stop_time,
                      imu_time_data[0],
                      imu_time_data[*imu_count - 1]);
        return ERROR;
    }

    if (imu_time_data[*imu_count - 1] > eph_stop_time)
    {
        IAS_LOG_ERROR("Ephemeris (%f,%f) data does not cover IMU (%f,%f) -> "
                      "Stop time",
                      eph_start_time, eph_stop_time,
                      imu_time_data[0],
                      imu_time_data[*imu_count - 1]);
        return ERROR;
    }

    /* check to see if there is enough ephemeris to process quaternion */
    if (quaternion_time_data[0] < eph_start_time ||
        quaternion_time_data[valid_quaternion_count - 1] > eph_stop_time)
    {
        IAS_LOG_ERROR("Ephemeris (%f,%f) data does not cover quaternion "
            "(%f,%f)", eph_start_time, eph_stop_time, quaternion_time_data[0],
            quaternion_time_data[valid_quaternion_count - 1]);
        return ERROR;
    }

    /* For Earth, Lunar, and Stellar collections make sure the quaternion and
       IMU data cover the imagery.  For other collections, it is OK if they 
       don't cover. */
    if ((acq_type == IAS_EARTH) || (acq_type == IAS_LUNAR)
        || (acq_type == IAS_STELLAR))
    {
        double interval_start;      /* start time of the interval */
        double interval_stop;       /* stop time of the interval */

        /* Find seconds from J2000 for the interval start time */
        if (ias_math_convert_year_doy_sod_to_j2000_seconds(
                interval_start_time, &interval_start) != SUCCESS)
        {
            IAS_LOG_ERROR("Converting Year, DOY, SOD format to J2000 seconds");
            return ERROR;
        }

        /* Find seconds from J2000 for the interval stop time */
        if (ias_math_convert_year_doy_sod_to_j2000_seconds(
                interval_stop_time, &interval_stop) != SUCCESS)
        {
            IAS_LOG_ERROR("Converting Year, DOY, SOD format to J2000 seconds");
            return ERROR;
        }

        /* Confirm the IMU data covers the imagery */
        if (imu_time_data[0] > interval_start)
        {
            IAS_LOG_ERROR("IMU start time %f occurs after the interval image "
                    "start time %f", imu_time_data[0], interval_start);
            return ERROR;
        }
        if (imu_time_data[*imu_count - 1] < interval_stop)
        {
            IAS_LOG_ERROR("IMU end time %f occurs before the interval image "
                    "end time %f", imu_time_data[*imu_count - 1], 
                    interval_stop);
            return ERROR;
        }

        /* Confirm the quaternion data covers the imagery */
        if (quaternion_time_data[0] > interval_start)
        {
            IAS_LOG_ERROR("Quaternion start time %f occurs after the interval "
                    "image start time %f", quaternion_time_data[0], 
                    interval_start);
            return ERROR;
        }
        if (quaternion_time_data[valid_quaternion_count - 1] < interval_stop)
        {
            IAS_LOG_ERROR("Quaternion end time %f occurs before the interval "
                    "image end time %f", 
                    quaternion_time_data[valid_quaternion_count - 1], 
                    interval_stop);
            return ERROR;
        }
    }

    /* Allocate structure that will contain attitude data */
    attitude_data = (IAS_VECTOR *)malloc(sizeof(IAS_VECTOR) 
        * valid_quaternion_count);
    if (attitude_data == NULL)
    {
        IAS_LOG_ERROR("Allocating attitude vectors.");
        return ERROR;
    }

    /* convert quaternions to angles */
    for (index = 0; index < valid_quaternion_count; index++)
    {
        if (compute_spacecraft_attitude_at_time(acq_type, anc_ephemeris_data,
                &valid_quaternion_data[index], quaternion_time_data[index], 
                &attitude_data[index]) != SUCCESS)
        {
            IAS_LOG_ERROR("Converting quaternions to r-p-y at valid "
                " at valid quaternion index %d and time %f.", index,  
                quaternion_time_data[index]);
            free(attitude_data);
            return ERROR;
        }

        /* Check for +/-pi rollover in angles.  This should happen rarely and
           likely only for non-earth looking data. */
        if (index > 0)
        {
            if ((attitude_data[index].x - attitude_data[index - 1].x) > pi)
            {
                attitude_data[index].x -= 2.0 * pi;
            }
            if ((attitude_data[index - 1].x - attitude_data[index].x) > pi)
            {
                attitude_data[index].x += 2.0 * pi;
            }
            if ((attitude_data[index].y - attitude_data[index - 1].y) > pi)
            {
                attitude_data[index].y -= 2.0 * pi;
            }
            if ((attitude_data[index - 1].y - attitude_data[index].y) > pi)
            {
                attitude_data[index].y += 2.0 * pi;
            }
            if ((attitude_data[index].z - attitude_data[index - 1].z) > pi)
            {
                attitude_data[index].z -= 2.0 * pi;
            }
            if ((attitude_data[index - 1].z - attitude_data[index].z) > pi)
            {
                attitude_data[index].z += 2.0 * pi;
            }
        }
    }

#if DEBUG_GENERATE_DATA_FILES == 1
    fp = fopen("anc.epa.dat", "w");

    for (index = 0; index < valid_quaternion_count; index++)
    {
        fprintf(fp, "%d %f %f %f %f %f %e %e %e\n",
                    index,quaternion_time_data[index],
                    valid_quaternion_data[index].vector.x,
                    valid_quaternion_data[index].vector.y,
                    valid_quaternion_data[index].vector.z,
                    valid_quaternion_data[index].scalar,
                    attitude_data[index].x, 
                    attitude_data[index].y, 
                    attitude_data[index].z);
    }

    fclose(fp);
#endif

    /* Replace the points flagged as bad. Calculate mean attitude reference. */
    mean_att_ref.x = 0.0;
    mean_att_ref.y = 0.0;
    mean_att_ref.z = 0.0;
    number_valid_points = 0;
    for (index = 0; index < valid_quaternion_count; index++)
    {
        if (valid_quaternion_flag[index] == 0)
        {
            if (index == 0 || index == valid_quaternion_count - 1)
            {
                attitude_data[index].x = 0.0;
                attitude_data[index].y = 0.0;
                attitude_data[index].z = 0.0;
            }
            else if (valid_quaternion_flag[index + 1] != 0)
            {
                attitude_data[index].x = (attitude_data[index - 1].x
                    + attitude_data[index + 1].x) / 2.0;
                attitude_data[index].y = (attitude_data[index - 1].y
                    + attitude_data[index + 1].y) / 2.0;
                attitude_data[index].z = (attitude_data[index - 1].z
                    + attitude_data[index + 1].z) / 2.0;
            }
            else
            {
                attitude_data[index].x = attitude_data[index - 1].x;
                attitude_data[index].y = attitude_data[index - 1].y;
                attitude_data[index].z = attitude_data[index - 1].z;
            }
        }
        else
        {
            number_valid_points++;
            mean_att_ref.x += attitude_data[index].x;
            mean_att_ref.y += attitude_data[index].y;
            mean_att_ref.z += attitude_data[index].z;
        }
    }

    /* Make sure there are valid points */
    if (number_valid_points == 0)
    {
        IAS_LOG_ERROR("No valid attitude points");
        free(attitude_data);
        return ERROR;
    }

    mean_att_ref.x /= (double)number_valid_points;
    mean_att_ref.y /= (double)number_valid_points;
    mean_att_ref.z /= (double)number_valid_points;

    /* change IMU to represent rates */
    for (index = 0; index < *imu_count; index++)
    {
        if (index > 0)
        {
            delta_sample_time = imu_time_data[index]
                                - imu_time_data[index - 1];
        }
        else
        {
            delta_sample_time = imu_time_data[index + 1]
                                - imu_time_data[index];
        }

        imu_data[index].x = imu_data[index].x / delta_sample_time;
        imu_data[index].y = imu_data[index].y / delta_sample_time;
        imu_data[index].z = imu_data[index].z / delta_sample_time;
    }

    /* Suppress IMU data for celestial acquisitions */
    if (acq_type == IAS_LUNAR || acq_type == IAS_STELLAR)
    {
        for (index = 0; index < *imu_count; index++)
            valid_imu_flag[index] = 0;
    }

    /* Check on orbital motion, removing if earth based acquisition. */
    if (ias_ancillary_correct_imu_orbital_motion(
            acq_type, anc_ephemeris_data, &mean_att_ref,
            *imu_count, imu_time_data, imu_data) != SUCCESS)
    {
        IAS_LOG_ERROR("Removing satellite motion from IMU");
        free(attitude_data);
        return ERROR;
    }

#if DEBUG_GENERATE_DATA_FILES == 1
    fp = fopen("anc.imu.sat.dat", "w");

    for (index = 0; index < *imu_count; index++)
    {
        fprintf(fp, "%d %f %e %e %e\n",
                index, imu_time_data[index],
                imu_data[index].x, imu_data[index].y, imu_data[index].z);
    }

    fclose(fp);
#endif

    /* Load windowed IMU from windowed quaternions if there is no IMU data */
    if (!imu_valid)
    {
        /* Copy as many quaternion values as will fit in the IMU array */
        if (*imu_count > valid_quaternion_count)
            *imu_count = valid_quaternion_count;
        for (index = 0; index < *imu_count; index++)
        {
           imu_time_data[index] = quaternion_time_data[index];
           imu_data[index].x = attitude_data[index].x;
           imu_data[index].y = attitude_data[index].y;
           imu_data[index].z = attitude_data[index].z;
        }
        ias_math_convert_j2000_seconds_to_year_doy_sod(imu_time_data[0], 
            imu_epoch);
    }

    /* Kalman filtering expects time increments in seconds
       from some relative epoch.  Adjust time accordingly */

    *imu_start_time = imu_time_data[0];

    IAS_LOG_DEBUG("Time adjustments %f", imu_time_data[0]);

    for (index = 0; index < *imu_count; index++)
    {
         imu_time_data[index] -= *imu_start_time;
    }

    for (index = 0; index < valid_quaternion_count; index++)
    {
         quaternion_time_data[index] -= *imu_start_time;
    }
 
    /* Combine quaternion and IMU data with Kalman smoothing filter. */
    /* May need to add IMU drift data if that turns out to be variable. */
    if (imu_valid || interpolate_quaternions_flag)
    {
        if (ias_ancillary_kalman_smooth_imu(
                valid_quaternion_count, *imu_count, quaternion_time_data, 
                attitude_data, valid_quaternion_flag, imu_time_data,
                imu_data, valid_imu_flag) != SUCCESS)
        {
            IAS_LOG_ERROR("Smoothing quaternion and IMU data");
            free(attitude_data);
            return ERROR;
        }
    }

#if DEBUG_GENERATE_DATA_FILES == 1
    fp = fopen("anc.imu.smo.dat", "w");

    for (index = 0; index < *imu_count; index++)
    {
        fprintf(fp, "%d %f %e %e %e\n",
                index, imu_time_data[index],
                imu_data[index].x, imu_data[index].y, imu_data[index].z);
    }

    fclose(fp);
#endif

    /* free locally allocated memory */
    free(attitude_data);

    return SUCCESS;
}

/****************************************************************************
NAME: compute_spacecraft_attitude

PURPOSE: Find the spacecrafts attitude at some reference time.

RETURN VALUE: Type = int
    Value    Description
    -----    -----------
    SUCCESS  Successful completion 
    ERROR    Operation failed

NOTE: The quaternions must be from the same major frame as the reference 
      attitude data and the reference ephemeris data (the first time of good 
      attitude and ephemeris).  Or we may have to interpolate sc_roll, 
      sc_pitch, and sc_yaw, so that sc_attitude is time sync.ed referenced to 
      tm_attitude.  The sc_attitude values are constant for entire image.

    -     -     -      -   -     -     -    -   -     -   -     -   -     -
   | Vector|   |Attitude| | Vector|   |      | |       | |       | | Vector|
   |   in  |   |Perturb | |   in  |   | Yaw  | | Pitch | | Roll  | |   in  |
   | Space | = |Matrix  | | Space | = | Rot. | | Rot.  | | Rot.  | | Space |
   |craft's|   |        | |craft's|   | Marix| | Matrix| | Matrix| |craft's|
   |  Body |   |        | |Orbit  |   |      | |       | |       | |Orbit  |
    -     -     -      -   -     -     -    -   -     -   -     -   -     -

                                                               -1
      -     -     -       -    -     -     -       -    -      -    -     -
     |       |   | Space   |  | Vector|   | Space   |  |Attitude|  | Vector|
     |Vector |   | craft's |  |  in   |   | craft's |  |Perturb |  |   in  |
     |  in   | = | Body    |  | Space | = | Orbit   |  |Matrix  |  | Space |
     |ECITOD |   |  to     |  |craft's|   |  to     |  |        |  |craft's|
     |       |   | ECITOD  |  | Body  |   | ECITOD  |  |        |  | Body  |
      -     -     -       -    -     -     -       -    -      -    -     -
*****************************************************************************/
static int compute_spacecraft_attitude_at_time
(
    IAS_ACQUISITION_TYPE acq_type, /* I: Image acquisition type */
    const IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data,
                                   /* I: pointer to ephem data */
    const IAS_QUATERNION *quat,    /* I: quaternion vector */
    double qtime,                  /* I: quaternion time (seconds from J2000) */
    IAS_VECTOR *attitude           /* O: Roll, pitch, yaw */
)
{
    int status;                             /* status of return from funtions */
    IAS_VECTOR spacecraft_velocity;         /* Spacecraft velocity vector */
    IAS_VECTOR spacecraft_orbital_x_vector; /* Spacecraft's orbital x-axis unit
                                               vector */
    IAS_VECTOR spacecraft_orbital_y_vector; /* Spacecraft's orbital y-axis unit
                                               vector */
    IAS_VECTOR spacecraft_orbital_z_vector; /* Spacecraft's orbital z-axis unit
                                               vector */ 

    double spacecraft_body_to_eci[3][3];    /* Spacecraft's body to ECITOD 
                                               matrix */
    double spacecraft_orbital_to_eci[3][3]; /* Spacecraft's orbital to ECITOD 
                                               matrix */
    double inv_spacecraft_orbital_to_eci[3][3]; /* Inverse of orbital to ECITOD
                                                   matrix */
    double inv_perturb[3][3]; /* Inverse of attitude perturb matrix */
    double magnitude_x;       /* Magnitude of the spacecraft x */
    double magnitude_y;       /* Magnitude of the spacecraft y */
    double magnitude_z;       /* Magnitude of the satellite position */
    IAS_QUATERNION acs2eci;   /* Conjugate of quaternion quat (eci2acs) */

    double quat_epoch;             /* quaternion time relative to ephem_time */
    double quat_time[3];      /* Quaternion time year, DOY, SOD. */

    /* Compute quat_epoch */
    status = ias_math_convert_j2000_seconds_to_year_doy_sod(qtime, quat_time);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Converting J2000 seconds %lf to Year, DOY, SOD format",
                      qtime);
        return ERROR;
    }
    if (ias_math_get_time_difference(quat_time,
        anc_ephemeris_data->utc_epoch_time, &quat_epoch) != SUCCESS)
    {
        IAS_LOG_ERROR("Computing delta epoch");
        return ERROR;
    }

    /* Set up the direction cosines matrix from the attitude control system 
       (ACS) reference axes to the ECITOD */
    ias_math_conjugate_quaternion(quat, &acs2eci);

    /* set up matrix for spacecraft's body to ECI */
    ias_math_convert_quaternion_to_euler(&acs2eci, spacecraft_body_to_eci);

    /* If not Earth view just calculate the attitude roll, pitch, and yaw */
    if (acq_type == IAS_STELLAR || acq_type == IAS_LUNAR)
    {
       attitude->x = -atan2(spacecraft_body_to_eci[2][1], 
           spacecraft_body_to_eci[2][2]);
       attitude->y =  asin(spacecraft_body_to_eci[2][0]);
       attitude->z = -atan2(spacecraft_body_to_eci[1][0], 
           spacecraft_body_to_eci[0][0]);
       return SUCCESS;
    }

    /* Calculate the spacecraft's position and velocity at a time from 
       ephemeris reference time */
    ias_ancillary_get_position_and_velocity_at_time(anc_ephemeris_data, 
        IAS_STELLAR, quat_epoch, &spacecraft_orbital_x_vector, 
        &spacecraft_velocity);

    spacecraft_orbital_z_vector.x = -spacecraft_orbital_x_vector.x;
    spacecraft_orbital_z_vector.y = -spacecraft_orbital_x_vector.y;
    spacecraft_orbital_z_vector.z = -spacecraft_orbital_x_vector.z;
    
    /* Calculate the cross products of the satellite ECI position and satellite
       velocity */
    ias_math_compute_3dvec_cross(&spacecraft_orbital_z_vector, 
        &spacecraft_velocity, &spacecraft_orbital_y_vector);
    ias_math_compute_3dvec_cross(&spacecraft_orbital_y_vector, 
        &spacecraft_orbital_z_vector, &spacecraft_orbital_x_vector);
    
    /* Calculate the magnitude of the vectors */
    magnitude_x = ias_math_compute_vector_length(&spacecraft_orbital_x_vector);
    magnitude_y = ias_math_compute_vector_length(&spacecraft_orbital_y_vector);
    magnitude_z = ias_math_compute_vector_length(&spacecraft_orbital_z_vector);
    
    /* Set up the second direction cosine matrix */
    if (magnitude_x == 0.0 || magnitude_y == 0.0 || magnitude_z == 0.0)
    {
        IAS_LOG_ERROR("Divide by zero with magnitude");
        return ERROR;
    }
    spacecraft_orbital_to_eci[0][0] = spacecraft_orbital_x_vector.x 
        / magnitude_x;
    spacecraft_orbital_to_eci[0][1] = spacecraft_orbital_y_vector.x 
        / magnitude_y;
    spacecraft_orbital_to_eci[0][2] = spacecraft_orbital_z_vector.x 
        / magnitude_z;
    
    spacecraft_orbital_to_eci[1][0] = spacecraft_orbital_x_vector.y 
        / magnitude_x;
    spacecraft_orbital_to_eci[1][1] = spacecraft_orbital_y_vector.y 
        / magnitude_y;
    spacecraft_orbital_to_eci[1][2] = spacecraft_orbital_z_vector.y 
        / magnitude_z;
    
    spacecraft_orbital_to_eci[2][0] = spacecraft_orbital_x_vector.z 
        / magnitude_x;
    spacecraft_orbital_to_eci[2][1] = spacecraft_orbital_y_vector.z 
        / magnitude_y;
    spacecraft_orbital_to_eci[2][2] = spacecraft_orbital_z_vector.z 
        / magnitude_z;
    
    /* Inverse the second direction cosine matrix */
    status = ias_math_invert_3x3_matrix(spacecraft_orbital_to_eci, 
        inv_spacecraft_orbital_to_eci);
    if (status != SUCCESS) 
    {
        IAS_LOG_ERROR("Error inverting spacecraft_orbital_to_eci matrix");
        return ERROR;
    }
    
    /* Multiply the inverse and direction cosine matrix
       Inverse perturbation matrix is the same as Body to Orbit (scb2sco) */
    ias_math_multiply_3x3_matrix(inv_spacecraft_orbital_to_eci, 
        spacecraft_body_to_eci, inv_perturb);
    
    /* Calculate the attitude roll, pitch, and yaw */
    attitude->x = -atan2(inv_perturb[2][1], inv_perturb[2][2]);
    attitude->y =  asin(inv_perturb[2][0]);
    attitude->z = -atan2(inv_perturb[1][0], inv_perturb[0][0]);

    IAS_LOG_DEBUG("Quaternion Attitude:  %e %e %e\n",
                  attitude->x, attitude->y, attitude->z);

    return SUCCESS;
}
