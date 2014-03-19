/******************************************************************************
NAME: ias_los_model_get_moon_position_at_location

PURPOSE: Given a L0R line and sample location calculate the right ascension,
    declination, and distance to the moon relative to the satellite.

RETURNS: SUCCESS or ERROR

******************************************************************************/
#include "logging_channel.h" /* define the debug logging channel */
#include "ias_logging.h"
#include "ias_types.h"
#include "ias_structures.h"
#include "ias_geo.h"
#include "ias_math.h"
#include "ias_los_model.h"

int ias_los_model_get_moon_position_at_location
(
    const IAS_LOS_MODEL *model, /*I: model structure */
    int band_index,         /*I: Band index in the image */
    int sca_index,          /*I: SCA index in the image */
    double line,            /*I: Line of interest (target) */
    double sample,          /*I: Sample of interest (target) */
    IAS_SENSOR_DETECTOR_TYPE dettype, /*I: Detector type */
    double *right_ascension,/*O: Right ascension lunar position (in radians)
                                 relative to spacecraft */
    double *declination,    /*O: Declination lunar position (in radians) 
                                 relative to spacecraft */
    double *distance        /*O: Lunar distance relative to spacecraft
                                 (in meters) */
)
{
    int month;            /* Month of acquisition */
    int day;              /* Day of acquisition */
    double julian_day;    /* Julian day of acquisition */
    double seconds_from_image_start; /* Time of the target from image start */
    IAS_VECTOR moon_tod;  /* Moon's position Earth fixed true-of-date */
    IAS_VECTOR moon_eci;  /* Moon's position in inertial J2000 */
    double image_time[3]; /* year/doy/sod image time */
    double ephem_time;    /* Time from ephem_time to image start */
    IAS_VECTOR satpos;    /* Satellite position in ECI J2000 */
    IAS_VECTOR satvel;    /* Satellite velocity in ECI J2000 */
    double rasc_from_earth;/* Right ascension of moon relative to Earth */
    double dec_from_earth; /* Declination of moon relative to Earth */
    double moon_earth_dist;/* Distance to moon */
    IAS_VECTOR moon_sat;   /* Satellite to moon LOS */
    double radians_per_degree = ias_math_get_radians_per_degree();
    double hrs2rad = 15.0 * radians_per_degree;

    /* Find the time from start of image for the requested line/sample */
    if (ias_sensor_find_time(line, sample, band_index, sca_index, 
            &model->sensor, dettype, &seconds_from_image_start) != SUCCESS)
    {
        IAS_LOG_ERROR("Finding time of SCA %d, Band index %d, L1R line %.8e,"
                "L1R sample %.8e", sca_index, band_index, line, sample);
        return ERROR;
    }

    /* Combine the seconds from the image start with the epoch time to get
       the image time in UTC */
    image_time[0] = model->sensor.bands[band_index].utc_epoch_time[0];
    image_time[1] = model->sensor.bands[band_index].utc_epoch_time[1];
    image_time[2] = model->sensor.bands[band_index].utc_epoch_time[2];
    ias_math_add_seconds_to_year_doy_sod(seconds_from_image_start, image_time);

    /* Get the month and day for the image time */
    if (ias_math_convert_doy_to_month_day(image_time[1], image_time[0],
            &month, &day) != SUCCESS)
    {
        IAS_LOG_ERROR("Converting Year %f, DOY %f to a month and day",
            image_time[0], image_time[1]);
        return ERROR;
    }

    /* Get Julian day needed for calculating the Moon's position */
    julian_day = ias_math_compute_full_julian_date(image_time[0], month, day,
            image_time[2]);

    /* Get the Moon's position relative to the Earth */
    if (ias_geo_compute_moon_position(julian_day, &rasc_from_earth,
            &dec_from_earth, &moon_earth_dist) != SUCCESS)
    {
        IAS_LOG_ERROR("Calculating the Moon's position for julian date %f",
            julian_day);
        return ERROR;
    }

    /* Convert units on the Moon's position to radians and meters */
    dec_from_earth *= radians_per_degree;
    rasc_from_earth *= hrs2rad;
    moon_earth_dist *= 1000.0;

    /* Convert declination, right ascension, and distance to a vector */
    ias_geo_convert_sph2cart(dec_from_earth, rasc_from_earth, moon_earth_dist,
        &moon_tod);

    /* Convert the lunar vector from ECI true-of-date to ECIJ2000 since the
       Moon vector is in true-of-date, no sidereal or polar motion, and the 
       geometric model is in inertial J2000. */
    if (ias_geo_transform_tod2j2k(model->earth.ut1_utc_correction, &moon_tod,
        image_time, &moon_eci) != SUCCESS)
    {
        IAS_LOG_ERROR("Converting the Moon's position to ECI J2000");
        return ERROR;
    }

    /* Get the time relative to the ephemeris epoch */
    if (ias_math_get_time_difference(
            model->sensor.bands[band_index].utc_epoch_time,
            model->spacecraft.ephemeris.utc_epoch_time, &ephem_time)
            != SUCCESS)
    {
        IAS_LOG_ERROR("Calculating time difference between the ephemeris and "
            "image epoch times");
        return ERROR;
    }
    ephem_time += seconds_from_image_start;

    /* Find satellite position in Inertial J2000 */
    ias_sc_model_get_position_and_velocity_at_time(&model->spacecraft.ephemeris,
            model->acquisition_type, ephem_time, &satpos, &satvel);

    /* Get the Moon's position relative to the satellite */
    moon_sat.x = moon_eci.x - satpos.x;
    moon_sat.y = moon_eci.y - satpos.y;
    moon_sat.z = moon_eci.z - satpos.z;

    /* Convert the position to spherical coordinates */
    if (ias_geo_convert_cart2sph(&moon_sat, declination, right_ascension,
            distance) != SUCCESS)
    {
        IAS_LOG_ERROR("Converting moon-satellite vector to spherical coords");
        return ERROR;
    }

    IAS_LOG_DEBUG("Band index %d SCA %d Time %f %f %f (%f from image start) "
                  "JD %f Moon %f %f %f\n", band_index, sca_index,
                  image_time[0], image_time[1], image_time[2],
                  seconds_from_image_start, julian_day, *right_ascension,
                  *declination, *distance);

    return SUCCESS;
}
