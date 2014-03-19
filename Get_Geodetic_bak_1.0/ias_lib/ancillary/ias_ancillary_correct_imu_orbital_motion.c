/****************************************************************************
NAME: ias_ancillary_correct_imu_orbital_motion

PURPOSE: Remove the effects due to orbital motion.  This is done through
         several steps.

    1) Find spacecraft vector for current IMU time
    2) Calculate ECI to Orbit transformation matrix for current time
    3) Calculate "cummulative" ECI to Orbit transformation matrix
    4) Calculate delta r-p-y due to satellite motion using cummulative matrix
    5) Remove delta r-p-y from IMU r-p-y

RETURN VALUE: Type = int
    Value    Description
    -----    -----------
    SUCCESS  Successful completion 
    ERROR    Operation failed

NOTES:
    This routine assumes that the ephemeris arrays in the model structure
    have already been populated.

*****************************************************************************/
#include <math.h>

#include "ias_logging.h"
#include "ias_math.h"
#include "ias_geo.h"
#include "ias_ancillary_private.h"

int ias_ancillary_correct_imu_orbital_motion
(
    IAS_ACQUISITION_TYPE acq_type, /* I: Image acquisition type */
    const IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data,
                                   /* I: Pointer to ephem data */
    const IAS_VECTOR *attitude_ref,/* I: Roll-pitch-yaw mean offset */
    int imu_count,                 /* I: Number of IMU points (array size) */
    const double *imu_time_data,   /* I: Array of IMU seconds since J2000 */
    IAS_VECTOR *imu_data           /* I/O: Array of IMU data */
)
{
    double eci2ob0[3][3];  /* trans. matrix ECI to orbital at IMU ref. time */
    double obt2eci[3][3];  /* trans. matrix orbital to ECI at IMU time */
    double eci2obt[3][3];  /* trans. matrix ECI to orbital at IMU time */
    double obt2ob0[3][3];  /* trans. matrix orbital time to orbital ref. time */
    double orb2acs[3][3];  /* trans. matrix orbital to ACS (ref. r-p-y)  */

    double delta_roll;     /* delta Roll due to sat. motion, in radian */
    double delta_pitch;    /* delta Pitch due to sat. motion, in radian */
    double delta_yaw;      /* delta Yaw due to sat. motion, in radian */

    IAS_VECTOR sat_pos;    /* satellite position at interpolated time */
    IAS_VECTOR sat_vel;    /* satellite velocity at interpolated time */

    int i, j, k;           /* looping variables */
    int status;            /* function return status values */

    double ephemeris_time; /* time from ephemeris start */
    double delta_time;     /* time between samples */
    double j2000_seconds;  /* seconds since J2000 */

    double cosine_roll;    /* trig functions */
    double sine_roll;
    double cosine_pitch;
    double sine_pitch;
    double cosine_yaw;
    double sine_yaw;

    /* if acquisition is stellar leave IMU in inertial coordinates */
    if (acq_type == IAS_STELLAR || acq_type == IAS_LUNAR)
    {
        for (i = 0; i < imu_count; i++)
        {
            imu_data[i].x = -imu_data[i].x;
            imu_data[i].y = -imu_data[i].y;
            imu_data[i].z = -imu_data[i].z;
        }

        return SUCCESS;
    }

    /* calculate the satellite position and velocity at the first IMU data
       point, and compose the transformation matrix from ECI system to the
       orbit system at this point */
    status = ias_math_convert_year_doy_sod_to_j2000_seconds(
        anc_ephemeris_data->utc_epoch_time, &j2000_seconds);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Converting Year %f, DOY %f, SOD %f to J2000 seconds",
                anc_ephemeris_data->utc_epoch_time[0],
                anc_ephemeris_data->utc_epoch_time[1],
                anc_ephemeris_data->utc_epoch_time[2]);
        return ERROR;
    }
    ephemeris_time = imu_time_data[0] - j2000_seconds;

    ias_ancillary_get_position_and_velocity_at_time(anc_ephemeris_data, 
        IAS_ECI, ephemeris_time, &sat_pos, &sat_vel);

    status = ias_geo_create_transformation_matrix(&sat_pos, &sat_vel, eci2obt);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Creating transformational matrix");
        return ERROR;
    }

    /* construct orbital to acs transformation */
    cosine_roll  = cos(attitude_ref->x);
    sine_roll    = sin(attitude_ref->x);
    cosine_pitch = cos(attitude_ref->y);
    sine_pitch   = sin(attitude_ref->y);
    cosine_yaw   = cos(attitude_ref->z);
    sine_yaw     = sin(attitude_ref->z);

    orb2acs[0][0] = cosine_pitch * cosine_yaw;
    orb2acs[1][0] = cosine_yaw * sine_roll * sine_pitch + cosine_roll 
        * sine_yaw;
    orb2acs[2][0] = sine_roll * sine_yaw - sine_pitch * cosine_roll 
        * cosine_yaw;

    orb2acs[0][1] = -sine_yaw * cosine_pitch;
    orb2acs[1][1] = cosine_yaw * cosine_roll - sine_yaw * sine_roll 
        * sine_pitch;
    orb2acs[2][1] = sine_pitch * sine_yaw * cosine_roll + cosine_yaw 
        * sine_roll;

    orb2acs[0][2] = sine_pitch;
    orb2acs[1][2] = -sine_roll * cosine_pitch;
    orb2acs[2][2] = cosine_pitch * cosine_roll;

    ias_math_multiply_3x3_matrix(orb2acs, eci2obt, eci2ob0);

    /* Process all the IMU data points */

    for (i = 1; i < imu_count; i++)
    {
        /* recalculate ephemeris_time from imu_time_data[i] */
        ephemeris_time = imu_time_data[i] - j2000_seconds;

        ias_ancillary_get_position_and_velocity_at_time(anc_ephemeris_data, 
            IAS_ECI, ephemeris_time, &sat_pos, &sat_vel);

        /* compose the transfromation matrix from the orbit system at the time 
           to ECI, for the calculation of transformation from orbital this 
           time to orbital at IMU reference time */
        /* from ECI to orbital at time */
        status = ias_geo_create_transformation_matrix(&sat_pos, &sat_vel,
                        obt2eci); 
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Returned from ias_geo_create_transformation_matrix");
            return ERROR;
        }

        ias_math_multiply_3x3_matrix(orb2acs, obt2eci, eci2obt);

        status = ias_math_invert_3x3_matrix(eci2obt, obt2eci);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Inverting matrix: eci2obt");
            return ERROR;
        }

        /* calculate the transformation from orbital at the IMU time to the 
           orbital at the IMU reference time, for the calculation of 
           attitude change  */
        ias_math_multiply_3x3_matrix(eci2ob0, obt2eci, obt2ob0);

        /* copy the current eci2obt array into eci2ob0 for use with
           the next point.  */
        for (j = 0; j < 3; j++)
            for (k = 0; k < 3; k++)
                eci2ob0[j][k] = eci2obt[j][k];

        delta_time = imu_time_data[i]
                     - imu_time_data[i - 1];
        delta_roll = -atan2(obt2ob0[2][1], obt2ob0[2][2]) / delta_time;
        delta_pitch = asin(obt2ob0[2][0]) / delta_time;
        delta_yaw = -atan2(obt2ob0[1][0], obt2ob0[0][0]) / delta_time;

        /* Remove the attitude change due to the satellite motion during
           the time from the previous IMU time to the current IMU time.
           Also swap the sign on the IMU data to be body to orbit. */
        imu_data[i].x = -imu_data[i].x - delta_roll;
        imu_data[i].y = -imu_data[i].y - delta_pitch;
        imu_data[i].z = -imu_data[i].z - delta_yaw; 

        /* if this is point 1, go back and correct the initial point */
        if (i == 1)
        {
            imu_data[0].x = -imu_data[0].x - delta_roll;
            imu_data[0].y = -imu_data[0].y - delta_pitch;
            imu_data[0].z = -imu_data[0].z - delta_yaw; 

            IAS_LOG_DEBUG("Corrected IMU %03d: %lf %lf %lf %lf\n",
                i - 1, imu_time_data[0],
                imu_data[0].x, imu_data[0].y, imu_data[0].z );
        }

        IAS_LOG_DEBUG("Corrected IMU %03d: %lf %lf %lf %lf\n",
            i, imu_time_data[i],
            imu_data[i].x, imu_data[i].y, imu_data[i].z );
    }

    return SUCCESS;
}
