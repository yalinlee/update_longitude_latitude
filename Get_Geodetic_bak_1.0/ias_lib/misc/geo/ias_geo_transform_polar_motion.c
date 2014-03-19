/*******************************************************************************
NAME: ias_geo_transform_polar_motion_true_pole_to_mean

PURPOSE: 
Transform from Earth's true spin axis (old system) to the mean
pole (CIO) (new system).

RETURNS: void

*******************************************************************************/
#include "ias_structures.h"
#include "ias_const.h"
#include "ias_geo.h"
#include "local_novas_wrapper.h"

void ias_geo_transform_polar_motion_true_pole_to_mean
(
    const IAS_VECTOR *r_old, /* I: coordinates (x, y, z) in the old system */
    double xp,    /* I: true pole position in the mean pole coords system, 
                     x-axis pointing along Greenwich meridian; in arc seconds */
    double yp,    /* I: true pole position in the mean pole coords system, 
                     y-axis pointing along west 90 degree meridian; in arc 
                     seconds */
    double jd_tdb,/* I: Julian date (Barycentric) */
    IAS_VECTOR *r_new /* O: coordinates in the new system */
)
{
    double in_vec[3];   /* Intermediate vector for NOVAS */
    double out_vec[3];  /* Intermediate vector for NOVAS */

    in_vec[0] = r_old->x;
    in_vec[1] = r_old->y;
    in_vec[2] = r_old->z;

    /* Calculate the wobble using NOVAS */
    NOVAS_WOBBLE(jd_tdb, NOVAS_TRUE_TO_MEAN_DIRECTION, xp, yp, in_vec, out_vec);

    r_new->x = out_vec[0];
    r_new->y = out_vec[1];
    r_new->z = out_vec[2];
}

/*******************************************************************************
NAME: ias_geo_transform_polar_motion_mean_pole_to_true

PURPOSE: 
Transform from mean pole (CIO) to Earth's true spin axis.

RETURNS: void

*******************************************************************************/
void ias_geo_transform_polar_motion_mean_pole_to_true
(
    const IAS_VECTOR *r_old, /* I: coordinates (x, y, z) in the old system */
    double xp,    /* I: mean pole position in the true pole coords system, 
                     x-axis pointing along Greenwich meridian; in arc seconds */
    double yp,    /* I: mean pole position in the true pole coords system, 
                     y-axis pointing along west 90 degree meridian; in arc 
                     seconds */
    double jd_tdb,/* I: Julian date (Barycentric) */
    IAS_VECTOR *r_new /* O: coordinates in the new system */
)
{
    double in_vec[3];   /* Intermediate vector for NOVAS */
    double out_vec[3];  /* Intermediate vector for NOVAS */

    in_vec[0] = r_old->x;
    in_vec[1] = r_old->y;
    in_vec[2] = r_old->z;

    /* Calculate the wobble using NOVAS */
    NOVAS_WOBBLE(jd_tdb,  NOVAS_MEAN_TO_TRUE_DIRECTION, xp, yp, in_vec,
        out_vec);

    r_new->x = out_vec[0];
    r_new->y = out_vec[1];
    r_new->z = out_vec[2];
}

