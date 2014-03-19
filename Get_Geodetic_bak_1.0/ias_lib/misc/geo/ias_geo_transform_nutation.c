/*******************************************************************************
NAME: ias_geo_transform_nutation
 
PURPOSE: 
Transformation from the mean-of-date system to true-of-date system through
nutation angles at a specified Barycentric Dynamical Time (TDB).

RETURN VALUE: None

*******************************************************************************/
#include "ias_math.h"
#include "ias_geo.h"
#include "local_novas_wrapper.h"

void ias_geo_transform_nutation_mod2tod
(
    const IAS_VECTOR *r_old,/* I: coordinates (x, y, z) in the mean-of-date
                                  system */
    double jd_tdb,     /* I: Julian date (Barycentric) for conversion */
    IAS_VECTOR *r_new  /* O: coordinates in the true-of-date equator and 
                             equinox sys. */
)
{
    double in_vec[3];       /* input vector as an array for NOVAS routine */
    double out_vec[3];      /* output vector as an array for NOVAS routine */

    /* Copy the input vector to the array */
    in_vec[0] = r_old->x;
    in_vec[1] = r_old->y;
    in_vec[2] = r_old->z;

    /* Do the nutation using NOVAS */
    NOVAS_NUTATION(jd_tdb, NOVAS_MEAN_TO_TRUE_DIRECTION, NOVAS_FULL_ACCURACY,
        in_vec, out_vec);

    /* Copy the output to the output vector */
    r_new->x = out_vec[0];
    r_new->y = out_vec[1];
    r_new->z = out_vec[2];
}

/*******************************************************************************
NAME: ias_geo_transform_nutation_tod2mod
 
PURPOSE: 
Transformation from the true-of-date system to mean-of-date system through
nutation angles at a specified Barycentric Dynamical Time (TDB).

RETURN VALUE:
None

*******************************************************************************/
void ias_geo_transform_nutation_tod2mod
(
    const IAS_VECTOR *r_old,/* I: coordinates (x, y, z) in the true-of-date
                                  system */
    double jd_tdb,     /* I: Julian date (Barycentric) for conversion */
    IAS_VECTOR *r_new  /* O: coordinates in the mean-of-date system */
)
{
    double in_vec[3];       /* input vector as an array for NOVAS routine */
    double out_vec[3];      /* output vector as an array for NOVAS routine */

    /* Copy the input vector to the array */
    in_vec[0] = r_old->x;
    in_vec[1] = r_old->y;
    in_vec[2] = r_old->z;

    /* Do the nutation using NOVAS */
    NOVAS_NUTATION(jd_tdb, NOVAS_TRUE_TO_MEAN_DIRECTION, NOVAS_FULL_ACCURACY,
        in_vec, out_vec);

    /* Copy the output to the output vector */
    r_new->x = out_vec[0];
    r_new->y = out_vec[1];
    r_new->z = out_vec[2];
}
