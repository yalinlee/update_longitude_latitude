/******************************************************************************
Name:         ias_sc_model_correct_ephem_convert_ecef2eci

Purpose:
Convert the ephemeris points information (position and velocity) from
the Earth Centered Earth Fixed (ECEF) system, to the
Earth Centered Inertial (ECI:true_of_date) system and output the coefficients
of polynomial fit to the position (x, y, z) and velocity (vx, vy, vz) in
the ECI system. The velocity is simply the "inertial" velocity vector
rotated to the ECI system, there is no change in magnitude of the
vector. The time variable of the polynomial is referenced to the
Reference Ephemeris Time. Polynomial fit to ephemeris in ECI system
is also output, for the later use of correcting gyro measurements.

RETURNS: SUCCESS or ERROR

******************************************************************************/
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_geo.h"
#include "ias_math.h"
#include "ias_spacecraft_model.h"

int ias_sc_model_correct_ephem_convert_ecef2eci
(
    double xp,              /* I: True pole postion in the mean pole coords
                               system, x-axis */
    double yp,              /* I: True pole position in the mean pole coords
                               system, y-axis */
    double ut1_utc,         /* I: Earth rotation angle difference ut1 - utc */
    const double *image_epoch, /* I: Image epoch time (year/doy/sod array) */
    const IAS_SC_PRECISION_MODEL *prec_model,   /* I: Precision parameters */
    IAS_SC_EPHEMERIS_MODEL *ephem_model         /* IO: Ephemeris to convert */
)
{
    IAS_VECTOR Wx;      /* Unit vector in along-track direction */ 
    IAS_VECTOR Wy;      /* Unit vector in cross-track direction */
    IAS_VECTOR Wz;      /* Unit vector in negative radial direction */
    double dx_ob;       /* Orbit position correction for x direction */
    double dy_ob;       /* Orbit position correction for y direction */
    double dz_ob;       /* Orbit position correction for z direction */
    double dx;          /* Orbit position correction in Earth fixed sys */
    double dy;          /* Orbit position correction in Earth fixed sys */
    double dz;          /* Orbit position correction in Earth fixed sys */
    double dvx;         /* Orbit velocity correction in Earth fixed system */
    double dvy;         /* Orbit velocity correction in Earth fixed system */
    double dvz;         /* Orbit velocity correction in Earth fixed system */
    double dvx_ob;      /* Orbit velocity correction for x direction */
    double dvy_ob;      /* Orbit velocity correction for y direction */
    double dvz_ob;      /* Orbit velocity correction for z direction */
    double vector_length;
    double vector_length_inverse; /* 1/vec_len; use to optimize with *, not / */
    double ephem_time;  /* Ephemeris time */
    double ephem_epoch_time[3];    /* Ephemeris UTC epoch time */
    double seconds_from_image_epoch;  /* Time different from ephemeris epoch to
                                         image epoch */
    int i;              /* General looping counter */

    /* Check to determine if enough ephemeris information is known */
    if (ephem_model->sample_count < IAS_LOS_LEGENDRE_TERMS)
    {
        IAS_LOG_ERROR("Not enough ephemeris points to process");
        return ERROR;
    }

    dvx_ob = prec_model->x_corr[1];
    dvy_ob = prec_model->y_corr[1];
    dvz_ob = prec_model->z_corr[1];

    /* Correction model does not support higher order ephemeris corrections */
    if (prec_model->ephemeris_order >= 3)
    {
        IAS_LOG_ERROR("Too many precision ephemeris coefficients");
        return ERROR;
    }

    /* Calculate the time difference between the ephemeris epoch and the 
       image epoch */
    if (ias_math_get_time_difference(ephem_model->utc_epoch_time,
            image_epoch, &seconds_from_image_epoch) != SUCCESS)
    {
        IAS_LOG_ERROR("Calculating time difference between the ephemeris and "
            "image epoch times");
        return ERROR;
    }

    /* Process the ephemeris information */
    for (i = 0; i < ephem_model->sample_count; i++)
    {
        ephem_epoch_time[0] = ephem_model->utc_epoch_time[0];
        ephem_epoch_time[1] = ephem_model->utc_epoch_time[1];
        ephem_epoch_time[2] = ephem_model->utc_epoch_time[2];

        /* Convert the satellite position and velocity vectors in
           ECEF (Earth-fixed) cartesian coordinates to
           ECI (true-of-date) coordinates. */
        ias_math_add_seconds_to_year_doy_sod(
                ephem_model->sample_records[i].seconds_from_epoch,
                ephem_epoch_time);
        if (ias_geo_ecef2eci(xp, yp, ut1_utc,
                &ephem_model->sample_records[i].ecef_position, 
                &ephem_model->sample_records[i].ecef_velocity, ephem_epoch_time,
                &ephem_model->sample_records[i].eci_position, 
                &ephem_model->sample_records[i].eci_velocity) != SUCCESS)
        {
            IAS_LOG_ERROR("Converting ECEF coordinate to ECI");
            return ERROR;
        }

        /* Calculate the orbit position correction in Orbit System. */
        ephem_time = seconds_from_image_epoch
            + ephem_model->sample_records[i].seconds_from_epoch 
            - prec_model->seconds_from_image_epoch;

        dx_ob = prec_model->x_corr[0] + prec_model->x_corr[1] * ephem_time;
        dy_ob = prec_model->y_corr[0] + prec_model->y_corr[1] * ephem_time;
        dz_ob = prec_model->z_corr[0] + prec_model->z_corr[1] * ephem_time;

        /* Apply the precision correction to the ECEF coordinates */
        /* Calculate the unit vector for Wz in the negative radial dir.
           Wz = -X / X where X = {x, y, z} */
        vector_length = ias_math_compute_vector_length(
                &ephem_model->sample_records[i].ecef_position);
        if (vector_length == 0.0)
        {
            IAS_LOG_ERROR("Vector length zero; cannot divide by zero");
            return ERROR;
        }
        vector_length_inverse = 1.0 / vector_length;
        Wz.x = -ephem_model->sample_records[i].ecef_position.x
        * vector_length_inverse;
        Wz.y = -ephem_model->sample_records[i].ecef_position.y
        * vector_length_inverse;
        Wz.z = -ephem_model->sample_records[i].ecef_position.z
        * vector_length_inverse;

        /* Calculate the unit vector for Wy in the cross-track direction.
           Wy = Wz cross V / |Wz cross V| where V = {vx, vy, vz} */
        ias_math_compute_3dvec_cross(&Wz,
                &ephem_model->sample_records[i].ecef_velocity, &Wy);
        vector_length = ias_math_compute_vector_length(&Wy);
        if (vector_length == 0.0)
        {
            IAS_LOG_ERROR("Vector length zero; cannot divide by zero");
            return ERROR;
        }
        vector_length_inverse = 1.0 / vector_length;
        Wy.x = Wy.x * vector_length_inverse;
        Wy.y = Wy.y * vector_length_inverse;
        Wy.z = Wy.z * vector_length_inverse;

        /* Calculate the unit vector for Wx in the along-track direction.
           Wx = Wy cross Wz */
        ias_math_compute_3dvec_cross(&Wy, &Wz, &Wx);

        /* Calculate the orbit position correction in the Earth Fixed system */
        dx = dx_ob * Wx.x + dy_ob * Wy.x + dz_ob * Wz.x;
        dy = dx_ob * Wx.y + dy_ob * Wy.y + dz_ob * Wz.y;
        dz = dx_ob * Wx.z + dy_ob * Wy.z + dz_ob * Wz.z;

        /* Calculate the orbit velocity correction in the Earth Fixed system */
        dvx = dvx_ob * Wx.x + dvy_ob * Wy.x + dvz_ob * Wz.x;
        dvy = dvx_ob * Wx.y + dvy_ob * Wy.y + dvz_ob * Wz.y;
        dvz = dvx_ob * Wx.z + dvy_ob * Wy.z + dvz_ob * Wz.z;

        /* Load the ephemeris points in EF */
        ephem_model->sample_records[i].precision_ecef_position.x
            = ephem_model->sample_records[i].ecef_position.x + dx;
        ephem_model->sample_records[i].precision_ecef_position.y
            = ephem_model->sample_records[i].ecef_position.y + dy;
        ephem_model->sample_records[i].precision_ecef_position.z
            = ephem_model->sample_records[i].ecef_position.z + dz;
        ephem_model->sample_records[i].precision_ecef_velocity.x
            = ephem_model->sample_records[i].ecef_velocity.x + dvx;
        ephem_model->sample_records[i].precision_ecef_velocity.y
            = ephem_model->sample_records[i].ecef_velocity.y + dvy;
        ephem_model->sample_records[i].precision_ecef_velocity.z
            = ephem_model->sample_records[i].ecef_velocity.z + dvz;

        /* Apply the precision correction to the ECI coordinates */
        /* Calculate the unit vector for Wz in the negative radial dir.
           Wz = -X / X where X = {x, y, z} */
        vector_length = ias_math_compute_vector_length(
                &ephem_model->sample_records[i].eci_position);
        if (vector_length == 0.0)
        {
            IAS_LOG_ERROR("Vector length zero; cannot divide by zero");
            return ERROR;
        }
        vector_length_inverse = 1.0 / vector_length;
        Wz.x = -ephem_model->sample_records[i].eci_position.x
        * vector_length_inverse;
        Wz.y = -ephem_model->sample_records[i].eci_position.y
        * vector_length_inverse;
        Wz.z = -ephem_model->sample_records[i].eci_position.z
        * vector_length_inverse;

        /* Calculate the unit vector for Wy in the cross-track direction.
           Wy = Wz cross V / |Wz cross V| where V = {vx, vy, vz} */
        ias_math_compute_3dvec_cross(&Wz,
                &ephem_model->sample_records[i].eci_velocity, &Wy);
        vector_length = ias_math_compute_vector_length(&Wy);
        if (vector_length == 0.0)
        {
            IAS_LOG_ERROR("Vector length zero; cannot divide by zero");
            return ERROR;
        }
        vector_length_inverse = 1.0 / vector_length;
        Wy.x = Wy.x * vector_length_inverse;
        Wy.y = Wy.y * vector_length_inverse;
        Wy.z = Wy.z * vector_length_inverse;

        /* Calculate the unit vector for Wx in the along-track direction.
           Wx = Wy cross Wz */
        ias_math_compute_3dvec_cross(&Wy, &Wz, &Wx);

        /* Calculate the orbit position correction in the ECI system.  */
        dx = dx_ob * Wx.x + dy_ob * Wy.x + dz_ob * Wz.x;
        dy = dx_ob * Wx.y + dy_ob * Wy.y + dz_ob * Wz.y;
        dz = dx_ob * Wx.z + dy_ob * Wy.z + dz_ob * Wz.z;

        /* Calculate the orbit velocity correction in the ECI system.  */
        dvx = dvx_ob * Wx.x + dvy_ob * Wy.x + dvz_ob * Wz.x;
        dvy = dvx_ob * Wx.y + dvy_ob * Wy.y + dvz_ob * Wz.y;
        dvz = dvx_ob * Wx.z + dvy_ob * Wy.z + dvz_ob * Wz.z;

        /* Load the ephemeris points in ECI */
        ephem_model->sample_records[i].precision_eci_position.x
            = ephem_model->sample_records[i].eci_position.x + dx; 
        ephem_model->sample_records[i].precision_eci_position.y
            = ephem_model->sample_records[i].eci_position.y + dy; 
        ephem_model->sample_records[i].precision_eci_position.z
            = ephem_model->sample_records[i].eci_position.z + dz; 
        ephem_model->sample_records[i].precision_eci_velocity.x
            = ephem_model->sample_records[i].eci_velocity.x + dvx;
        ephem_model->sample_records[i].precision_eci_velocity.y
            = ephem_model->sample_records[i].eci_velocity.y + dvy;
        ephem_model->sample_records[i].precision_eci_velocity.z
            = ephem_model->sample_records[i].eci_velocity.z + dvz;
    }

    return SUCCESS;
}
