/******************************************************************************
NAME:                 ias_geo_compute_earth2orbit_transform

PURPOSE: Compose the transformation matrix from the Earth-fixed Cartesian
    system to the orbit-oriented Cartesian system.

RETURN VALUE: SUCCESS or ERROR

ALGORITHM REFERENCES:
                C. Wivell, ATTITUDE in ELIPS;
                G. Rosborough, et al., IEEE 32.3, 1994; 
                Wz, unit vector in negative radial direction, pointing to the
                Earth center, if the satellite attitude is defined by star
                sensor like Landsat, then;
                Wz = - X / |X|;
                (if Wz to be defined as the negative ellipsoid normal
                direction by using horizon sensor, like AVHRR, then
                call cart2geod(X, ae, flat, &lat, &lon, &height)
                Wz = { -cos(lat)*cos(lon), -cos(lat)*sin(lon), -sin(lat) })
                Wy, unit vector in negative normal (- angular momentum);
                Wy = Wz cross V / |Wz cross V|;
                Wx, unit vector in transverse direction;
                Wx = Wy cross Wz;
                transf_matrix(1,1) = { Wx(1), Wx(2), Wx(3) };
                transf_matrix(2,1) = { Wy(1), Wy(2), Wy(3) };
                transf_matrix(3,1) = { Wz(1), Wz(2), Wz(3) }.

******************************************************************************/
#include <math.h>
#include "ias_logging.h"
#include "ias_math.h"
#include "ias_geo.h"

int ias_geo_compute_earth2orbit_transform
(
    const IAS_VECTOR *satpos, /* I: satellite position vector in Earth-fixed
                                    system, in meters */
    const IAS_VECTOR *satvel, /* I: inertial satellite velocity vector in
                                    Earth-fixed system, in m/sec */
    double transf_matrix[3][3]/* O: 3 X 3 transformation matrix, from the
                                    Earth-fixed system to the orbit-oriented
                                    Cartesian system. */
)
{
    double magx = 0.0;            /* magnitude of vector */
    IAS_VECTOR Wz, Wy, Wx;        /* unit vector of the orbit system */

    /* Calculate Unit Z vector */
    magx = ias_math_compute_vector_length(satpos);

    /* Test for divide by zero error */
    if (magx == 0.0)
    {
        IAS_LOG_ERROR("Attempting to divide by zero");
        return ERROR;
    }
    Wz.x = -satpos->x / magx;
    Wz.y = -satpos->y / magx;
    Wz.z = -satpos->z / magx;

    /* Calculate Unit Y vector */
    ias_math_compute_3dvec_cross(&Wz, satvel, &Wy);
    magx = ias_math_compute_vector_length(&Wy);

    /* Test for divide by zero error */
    if (magx == 0.0)
    {
        IAS_LOG_ERROR("Attempting to divide by zero");
        return ERROR;
    }
    Wy.x  = Wy.x / magx;
    Wy.y  = Wy.y / magx;
    Wy.z  = Wy.z / magx;

    /* Calculate unit X vector as a cross product of the other axes */
    ias_math_compute_3dvec_cross(&Wy, &Wz, &Wx);

    /* Pack the unit vectors into the transf_matrix matrix */
    transf_matrix[0][0] = Wx.x;
    transf_matrix[1][0] = Wy.x;
    transf_matrix[2][0] = Wz.x;
 
    transf_matrix[0][1] = Wx.y;
    transf_matrix[1][1] = Wy.y;
    transf_matrix[2][1] = Wz.y;
 
    transf_matrix[0][2] = Wx.z;
    transf_matrix[1][2] = Wy.z;
    transf_matrix[2][2] = Wz.z;

    return SUCCESS;
}
