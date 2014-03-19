/******************************************************************************
NAME: ias_geo_transform_precession_j2k2mod

PURPOSE: Transformation from J2000.0 system to mean-of-date system
 through precession angles at a specified Barycentric Dynamical Time (TDB).

RETURN VALUE: SUCCESS or ERROR

*******************************************************************************/
#include "ias_math.h"
#include "ias_logging.h"
#include "ias_geo.h"
#include "local_novas_wrapper.h"

int ias_geo_transform_precession_j2k2mod
(
    const IAS_VECTOR *r_old,/* I: coordinates (x, y, z) in the J2000.0 system */
    double jd_tdb,    /* I: Julian date (Barycentric) for conversion */
    IAS_VECTOR *r_new /* O: coordinates in the mean-of-date equator and equinox 
                         sys. */
)
{
    double in_vec[3];       /* input vector as an array for NOVAS routine */
    double out_vec[3];      /* output vector as an array for NOVAS routine */
    int status;

    /* Copy the input vector to the array */
    in_vec[0] = r_old->x;
    in_vec[1] = r_old->y;
    in_vec[2] = r_old->z;

    /* Do the precession using NOVAS */
    status = NOVAS_PRECESSION(IAS_EPOCH_2000, in_vec, jd_tdb, out_vec);
    if (status != NOVAS_SUCCESS)
    {
        /* This error is very unlikely to happen since it can only happen
           if IAS_EPOCH_2000 isn't passed to the NOVAS precession routine as
           one of the parameters */
        IAS_LOG_ERROR("NOVAS precession routine returned error code of %d",
            status);
        return ERROR;
    }

    /* Copy the output to the output vector */
    r_new->x = out_vec[0];
    r_new->y = out_vec[1];
    r_new->z = out_vec[2];

    return SUCCESS;
}

/******************************************************************************
NAME: ias_geo_transform_precession_mod2j2k

PURPOSE: Transformation from mean-of-date system to J2000.0 system through 
    precession angles at a specified Barycentric Dynamical Time (TDB).

RETURN VALUE: SUCCESS or ERROR

*******************************************************************************/
int ias_geo_transform_precession_mod2j2k
(
    const IAS_VECTOR *r_old,/* I: coordinates (x, y, z) in the mean-of-date
                                  system */
    double jd_tdb,    /* I: Julian date (Barycentric) for conversion */
    IAS_VECTOR *r_new /* O: coordinates in the J2000.0 system */
)
{
    double in_vec[3];       /* input vector as an array for NOVAS routine */
    double out_vec[3];      /* output vector as an array for NOVAS routine */
    int status;

    /* Copy the input vector to the array */
    in_vec[0] = r_old->x;
    in_vec[1] = r_old->y;
    in_vec[2] = r_old->z;

    /* Do the precession using NOVAS */
    status = NOVAS_PRECESSION(jd_tdb, in_vec, IAS_EPOCH_2000, out_vec);
    if (status != NOVAS_SUCCESS)
    {
        /* This error is very unlikely to happen since it can only happen
           if IAS_EPOCH_2000 isn't passed to the NOVAS precession routine as
           one of the parameters */
        IAS_LOG_ERROR("NOVAS precession routine returned error code of %d",
            status);
        return ERROR;
    }

    /* Copy the output to the output vector */
    r_new->x = out_vec[0];
    r_new->y = out_vec[1];
    r_new->z = out_vec[2];

    return SUCCESS;
}
