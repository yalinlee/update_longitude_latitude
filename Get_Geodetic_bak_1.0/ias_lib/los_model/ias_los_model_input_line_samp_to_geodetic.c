/****************************************************************************
Name: ias_los_model_input_line_samp_to_geodetic

Purpose: Main routine for calculating the geodetic coordinates for the L0Rp
    line and sample location. Calculations are done based on sensor, scene, and
    target specific information stored in the line-of-sight model.  Module does
    the following:
    1) Determines time associated with L0rp line and sample location.
    2) Calculates line-of-sight (LOS) for detector/SCA/Band location.
    3) Calculates roll-pitch-yaw for time.
    4) Calculates satellite position and velocity for time.
    5) Uses position/velocity/attitude information to project LOS in Earth 
       fixed coordinates.
    6) Adjusts LOS for center-of-mass.
    7) Adjusts LOS for velocity aberration.
    8) Finds target location for LOS.  Location in cartesian coordinates.
    9) Corrects target LOS for light travel time.
    10) Calculates geodetic coordinates for target position.

Returns:
    SUCCESS or ERROR

Note: For stellar and lunar collects no ajustments are done for light travel
    time and there are no geodetic coordinates calculated.

*******************************************************************************
                        Property of the U.S. Government
                             USGS EROS Data Center
*****************************************************************************/
#include <math.h>
#include "ias_los_model.h"
#include "ias_geo.h"
#include "ias_math.h"
#include "logging_channel.h" /* define the debug logging channel */
#include "ias_logging.h"

/* The corrections within this routine dependant on these macro definitions
   are not optional. The macros are defined for CalVal for ease of testing. */
#define CENTER_OF_MASS 1
#define VELOCITY_ABERR 1
#define LIGHT_TRAVEL 1

int ias_los_model_input_line_samp_to_geodetic
(
//    double line,                        /* I: Input line number */
	long long image_time, 					/* I: Generation time of the image */
    double sample,                      /* I: Input sample number */
    int band_index,                     /* I: Input band index */
    int sca_index,                      /* I: SCA index */
    double target_elev,                 /* I: Target elevation
                                              (at line/sample) */
    const IAS_LOS_MODEL *model,         /* I: LOS model information */
//    const IAS_LOS_MODEL *model1,
    IAS_SENSOR_DETECTOR_TYPE dettype,   /* I: Detector type to project NOMINAL,
                                              ACTUAL, MAXIMUM or EXACT */
    double *attitude_variance,          /* I: Attitude perterbation array
                                              (r,p,y) (in radians) used in
                                              determining the jitter
                                              sensitivity coefficients or
                                              NULL to factor with no variance */
    double *target_latd,                /* O: Target latitude */
    double *target_long                 /* O: Target longitude */
)
{
    IAS_VECTOR sensor_los;      /* Sensor LOS vector */
    IAS_VECTOR new_los;         /* Earth or stellar LOS */
    IAS_VECTOR pert_los;        /* Perturbed LOS vector */
    double delta_eph_time;      /* Delta time for ephemeris of image time */

    double roll = 0;                /* Spacecraft roll */
    double pitch = 0;               /* Spacecraft pitch */
    double yaw = 0;                 /* Spacecraft yaw */

    IAS_VECTOR satpos;          /* Spacecraft position */
    IAS_VECTOR satvel;          /* Spacecraft velocity */

    double orb2ecf[3][3];       /* Orbit to ECEF transformation */
    double attpert[3][3];    /* Attitude pertubration matrix */

    IAS_VECTOR vel_aberr_los;   /* LOS vector adjusted for velocity aberration*/

    double target_latc;         /* Target geocentric latitude (radians)*/
    double target_earth_radius; /* Radius of the earth at the target  */
    double target_height;       /* Height of the target in geodetic coords */
    IAS_VECTOR target_vec;      /* Target vector */

    const IAS_SENSOR_BAND_MODEL *band = &model->sensor.bands[band_index];
    int status;
//    IAS_LOG_DEBUG("%f %f %d %d %f", line, sample, sca_index, band_index,
//            target_elev);

//    if(ias_sensor_find_time(line, sample, band_index, sca_index, &model->sensor,
//                dettype, &image_time) != SUCCESS)
//    {
//        IAS_LOG_ERROR("Finding time of SCA %d, Band index %d, L1R line %.8e,"
//                "L1R samp %.8e", sca_index, band_index, line, sample);
//        return ERROR;
//    }
//
//    IAS_LOG_DEBUG("   Time %.8e", image_time);

    status = ias_sensor_find_los_vector(sca_index, sample, dettype, band,
            &sensor_los);
    if(status != SUCCESS)
    {
        IAS_LOG_ERROR("Finding the LOS vector for SCA %d, Band index %d, "
                "Detector %.8e", sca_index, band_index, sample);
        return ERROR;
    }

    IAS_LOG_DEBUG("   LOS %.8e,%.8e,%.8e", sensor_los.x, sensor_los.y,
            sensor_los.z);




//    double utc_epoch_time[3] = {2014, 62, 16397.029 };
//    image_time = 353.109;
    /* Added by LYL, convert time_t to double utc_epoch_time[3] */
    double utc_epoch_time[3];
    struct tm imageTime;
    long image_time_seconds = image_time / 1000;
    gmtime_r(&image_time_seconds,&imageTime);
    utc_epoch_time[0] = imageTime.tm_year + 1900;
    utc_epoch_time[1] = imageTime.tm_yday + 1;
    utc_epoch_time[2] = (imageTime.tm_hour*60+imageTime.tm_min)*60
    					+ imageTime.tm_sec + (double)(image_time%1000)/1000;

    /* Get the delta time relative to the ephemeris epoch */
    if (ias_math_get_time_difference(utc_epoch_time,
            model->spacecraft.ephemeris.utc_epoch_time, &delta_eph_time)
            != SUCCESS)
    {
        IAS_LOG_ERROR("Calculating time difference between the ephemeris and "
            "image epoch times");
        return ERROR;
    }

    IAS_LOG_DEBUG("   Delta ephemeris time %.8e", delta_eph_time);

    ias_sc_model_get_position_and_velocity_at_time(&model->spacecraft.ephemeris,
            model->acquisition_type, delta_eph_time, &satpos, &satvel);

    IAS_LOG_DEBUG("   Spacecraft ephemeris Pos %.8e,%.8e,%.8e "
            "Vel %.8e,%.8e,%.8e", satpos.x,satpos.y,satpos.z,satvel.x,satvel.y,
            satvel.z);



    if(ias_geo_convert_sensor_los_to_spacecraft(band->sensor->sensor2acs,
                model->acquisition_type, &satpos, &satvel, &sensor_los, roll,
                pitch, yaw, orb2ecf, attpert, &pert_los, &new_los) != SUCCESS )
    {
        IAS_LOG_ERROR("Finding perturb or new LOS for SCA %d, Band index %d, "
                "L1R samp %f", sca_index, band_index,sample);
        return ERROR;
    }

    IAS_LOG_DEBUG("   Perturbed LOS %.8e,%.8e,%.8e", pert_los.x, pert_los.y,
            pert_los.z);
    IAS_LOG_DEBUG("   New LOS %.8e,%.8e,%.8e", new_los.x, new_los.y, new_los.z);

//    if (model->acquisition_type == IAS_EARTH)
//    {
//        if (CENTER_OF_MASS)
//        {
//            IAS_VECTOR offpos;
//
//            ias_geo_correct_for_center_of_mass(
//                        band->sensor->center_mass2sensor_offset, orb2ecf,
//                        attpert, &satpos, &offpos);
//            satpos.x = offpos.x;
//            satpos.y = offpos.y;
//            satpos.z = offpos.z;
//        }
//    }
//    IAS_LOG_DEBUG("   CM offset position %.8e, %.8e, %.8e", satpos.x, satpos.y,
//            satpos.z);
//
    if(VELOCITY_ABERR)
    {
        if(ias_geo_correct_for_velocity_aberration(&satpos, &satvel,
                    model->acquisition_type, &model->earth, &new_los,
                    &vel_aberr_los) != SUCCESS)
        {
            IAS_LOG_ERROR("Adjusting for vel. aberr. for SCA %d, Band index %d,"
                    "L1R samp %f", sca_index, band_index,sample);
            return ERROR;
        }
    }
    else
    {
        vel_aberr_los.x = new_los.x;
        vel_aberr_los.y = new_los.y;
        vel_aberr_los.z = new_los.z;
    }
    IAS_LOG_DEBUG("   Velocity Aberration LOS %.8e,%.8e,%.8e", vel_aberr_los.x,
            vel_aberr_los.y, vel_aberr_los.z );


    if (model->acquisition_type == IAS_EARTH)
    {
        if (ias_geo_find_target_position(&satpos, &vel_aberr_los, &model->earth,
                    target_elev, &target_vec, &target_latc, target_long,
                    &target_earth_radius) != SUCCESS)
        {
            IAS_LOG_ERROR("Targeting Earth");
            return ERROR;
        }

        if(LIGHT_TRAVEL)
        {
            IAS_VECTOR ltarvec;     /* Light travel time target vector */

            if(ias_geo_correct_for_light_travel_time(&satpos, &model->earth,
                        &target_vec, &ltarvec, &target_latc, target_long,
                        &target_earth_radius) != SUCCESS) 
            {
                IAS_LOG_ERROR("Correcting light travel time");
                return ERROR;
            }
            IAS_LOG_DEBUG("   Org target %.8e,%.8e,%.8e Light travel "
                    "%.8e,%.8e,%.8e", target_vec.x, target_vec.y, target_vec.z,
                    ltarvec.x, ltarvec.y, ltarvec.z);
        }

        /* Calculate the geodetic latitude and height of the target */
        if (ias_geo_convert_geocentric_height_to_geodetic(target_latc,
                    target_earth_radius, &model->earth, target_latd,
                    &target_height) != SUCCESS)
        {
            IAS_LOG_ERROR("Converting geocentric lat/height to geodetic");
            return ERROR;
        }
        IAS_LOG_DEBUG("   Earth based target rad/m %.8e,%.8e,%.8e "
                "deg/m %.8e,%.8e,%.8e", *target_latd, *target_long,
                target_earth_radius, (*target_latd) *
                    ias_math_get_degrees_per_radian(),
                (*target_long) * ias_math_get_degrees_per_radian(),
                    target_earth_radius);
    }
    else if (model->acquisition_type == IAS_STELLAR 
            || model->acquisition_type == IAS_LUNAR)
    {
        if (ias_geo_convert_cart2sph(&vel_aberr_los, target_latd, target_long,
                    &target_height)
                != SUCCESS)
        {
            IAS_LOG_ERROR("Converting stellar/lunar LOS to spherical"
                    " coordinates");
            return ERROR;
        }
    }

    return SUCCESS;
}
