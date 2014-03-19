/******************************************************************************
NAME:    ias_los_model_get_satellite_state_vector_at_location.c

PURPOSE: Finds the satellite position, velocity, and time using the
         forward model.

RETURNS: SUCCESS or ERROR

******************************************************************************/
#include <math.h>
#include "ias_los_model.h"
#include "ias_geo.h"
#include "ias_math.h"
#include "ias_logging.h"

#define         CENTER_OF_MASS          1

int ias_los_model_get_satellite_state_vector_at_location
(
    double line,                        /* I: Line of interest (target) */
    double sample,                      /* I: Sample of interest (target) */
    int band_index,                     /* I: Band the line/sample are in */
    int sca_index,                      /* I: SCA the line/sample are in */
    IAS_SENSOR_DETECTOR_TYPE dettype,   /* I: Detector type */
    const IAS_LOS_MODEL *model,         /* I: LOS model information */
    IAS_VECTOR *satpos,                 /* O: Satellite position (meters) in
                                              Earth Centered Inertial (ECI)
                                              coordinates */
    IAS_VECTOR *satvel,                 /* O: Satellite velocity in ECI
                                              (meters/millisec) */
    IAS_VECTOR *satatt,                 /* O: Satellite attitude (roll, pitch,
                                              yaw) in radians */
    double *image_time,                 /* O: Time of the target image point
                                              (seconds of day) */
    int  *year,                         /* O: Year of image point */
    int  *day                           /* O: Day of image point */
)
{
    double att_time;             /* Delta time for attitude of image time */
    double eph_time;             /* Delta time for ephemeris of image time */
    double roll;                 /* Spacecraft roll */
    double pitch;                /* Spacecraft pitch */
    double yaw;                  /* Spacecraft yaw */
    double orb2ecf[3][3];        /* Orbit to ECEF transformation */
    double attpert[3][3];        /* Attitude pertubration matrix */
    const IAS_SENSOR_BAND_MODEL *band = &model->sensor.bands[band_index];

    /* Compute image time for input line/sample */
    if (ias_sensor_find_time(line, sample, band_index, sca_index,
                &model->sensor, dettype, image_time) != SUCCESS)
    {
        IAS_LOG_ERROR("Finding time of SCA %d, Band %d, L1R line %f, "
                "L1R samp %f", sca_index, band_index, line, sample);
        return ERROR;
    }

    /* Calculate corresponding attitude time reference */
    if (ias_math_get_time_difference(band->utc_epoch_time,
            model->spacecraft.attitude.utc_epoch_time, &att_time)
            != SUCCESS)
    {
        IAS_LOG_ERROR("Calculating time difference between the attitude and "
            "image epoch times");
        return ERROR;
    }
    att_time += *image_time;

    /* Calculate attitude at current time (needed for OLI-to-CM offset
       computation) */
    if (ias_sc_model_find_attitude_at_time(&model->spacecraft.attitude,
                att_time, &roll, &pitch, &yaw) != SUCCESS)
    {
        IAS_LOG_ERROR("Finding attitude at time %f", att_time);
        return ERROR;
    }
    satatt->x = roll;
    satatt->y = pitch;
    satatt->z = yaw;

    /* Calculate ephemeris time reference */
    if (ias_math_get_time_difference(band->utc_epoch_time,
            model->spacecraft.ephemeris.utc_epoch_time, &eph_time)
            != SUCCESS)
    {
        IAS_LOG_ERROR("Calculating time difference between the ephemeris and "
            "image epoch times");
        return ERROR;
    }
    eph_time += *image_time;

    /* Calculate ephemeris at current time */
    ias_sc_model_get_position_and_velocity_at_time(&model->spacecraft.ephemeris,
            model->acquisition_type, eph_time, satpos, satvel);

    /* Compute ACS2ORB and ORB2ECF transformation matrices */
    if (ias_geo_compute_orientation_matrices(satpos, satvel, roll,
                pitch, yaw, orb2ecf, attpert) != SUCCESS)
    {
        IAS_LOG_ERROR("Finding ACS2ORB for SCA %d, Band %d, L1R line %f, "
                "L1R samp %f", sca_index, band_index, line, sample);
        return ERROR;
    }

    /* Compute sensor to spacecraft CM offset adjustment */
    if (model->acquisition_type == IAS_EARTH)
    {
        if (CENTER_OF_MASS)
        {
            IAS_VECTOR offpos;

            ias_geo_correct_for_center_of_mass(
                        band->sensor->center_mass2sensor_offset, orb2ecf,
                        attpert, satpos, &offpos);
            satpos->x = offpos.x;
            satpos->y = offpos.y;
            satpos->z = offpos.z;
        }
    }

    /* Load the time values */
    *year = (int)band->utc_epoch_time[0];
    *day = (int)band->utc_epoch_time[1];
    *image_time += band->utc_epoch_time[2];

    return SUCCESS;
}
