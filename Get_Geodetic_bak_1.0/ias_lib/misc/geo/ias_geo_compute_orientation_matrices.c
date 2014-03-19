/******************************************************************************
NAME: ias_geo_compute_orientation_matrices.c

PURPOSE:
Calculate the orbital-to-ECF and body-to-orbital orientation matrices.

Returns:
    SUCCESS or ERROR

******************************************************************************/
#include <math.h>
#include "ias_const.h"
#include "ias_structures.h"
#include "ias_math.h"
#include "ias_logging.h"
#include "ias_geo.h"

int ias_geo_compute_orientation_matrices
(
    const IAS_VECTOR *satpos,   /* I: Satellite position (meters) */
    const IAS_VECTOR *satvel,   /* I: Satellite velocity (m/s) */
    double roll,                /* I: Corrected roll (radians) */
    double pitch,               /* I: Corrected pitch (radians) */
    double yaw,                 /* I: Corrected yaw (radians) */
    double orb2ecf[3][3],       /* O: Orbit to ECEF transformation */
    double attpert[3][3]        /* O: Attitude perturbation matrix */
)
{
    IAS_VECTOR vector_v;     /* Velocity vector (unperturbed ECEF) */
    IAS_VECTOR vector_x;     /* Vector in direction of geocentric nadir */
    IAS_VECTOR vector_y;     /* Vector in direction of angular momentum */
    IAS_VECTOR vector_z;     /* Vector in direction of circular velocity
                                (ECEF coords) */
    double vector_mag[3];    /* Magnitude of x/y/z vectors */
    double roll_cos;         /* Cosine of roll */
    double roll_sin;         /* Sine of roll */
    double pitch_cos;        /* Cosine of pitch */
    double pitch_sin;        /* Sine of pitch */
    double yaw_cos;          /* Cosine of yaw */
    double yaw_sin;          /* Sine of yaw */

    /* Find unperturbed spacecraft coordinates in terms of the ECEF coords */
    vector_z.x = -satpos->x;
    vector_z.y = -satpos->y;
    vector_z.z = -satpos->z;

    vector_v.x = satvel->x;
    vector_v.y = satvel->y;
    vector_v.z = satvel->z;

    /* Find vector in the direction of the angular momentum */
    ias_math_compute_3dvec_cross(&vector_z, &vector_v, &vector_y);

    /* Find vector in the direction of the velocity */
    ias_math_compute_3dvec_cross(&vector_y, &vector_z, &vector_x);

    /* Calculate the magnitude of each vector */
    vector_mag[0] = ias_math_compute_vector_length(&vector_x);
    vector_mag[1] = ias_math_compute_vector_length(&vector_y);
    vector_mag[2] = ias_math_compute_vector_length(&vector_z);

    /* Construct the Orbital to ECEF transformation */
    if (vector_mag[0] == 0.0 || vector_mag[1] == 0.0 || vector_mag[2] == 0.0)
    {
        IAS_LOG_ERROR("Can not divide by zero");
        return ERROR;
    }
    orb2ecf[0][0] = vector_x.x / vector_mag[0];
    orb2ecf[0][1] = vector_y.x / vector_mag[1];
    orb2ecf[0][2] = vector_z.x / vector_mag[2];

    orb2ecf[1][0] = vector_x.y / vector_mag[0];
    orb2ecf[1][1] = vector_y.y / vector_mag[1];
    orb2ecf[1][2] = vector_z.y / vector_mag[2];

    orb2ecf[2][0] = vector_x.z / vector_mag[0];
    orb2ecf[2][1] = vector_y.z / vector_mag[1];
    orb2ecf[2][2] = vector_z.z / vector_mag[2];

    /* Body (ACS) to Orbital Attitude Perturbation Matrix
                              -     -    -     -    -     -    
                             |       |  |       |  |       |  
       perturbation matrix = |  yaw  |  | pitch |  | roll  | 
                             |       |  |       |  |       |
                              -     -    -     -    -     - 
       Perturb LOS
        -   -     -       -    -       -    -       -    -      -
       | ECEF|   |         |  |         |  |         |  | Sensor |
       | LOS | = | ORB2ECF |  | ACS2ORB |  | OLI2ACS |  |   LOS  |
       |     |   |         |  |(perturb)|  |         |  |        |
        -   -     -       -    -       -    -       -    -      -
     */
//    roll_cos = cos(roll);
//    roll_sin = sin(roll);
//    pitch_cos = cos(pitch);
//    pitch_sin = sin(pitch);
//    yaw_cos = cos(yaw);
//    yaw_sin = sin(yaw);
//
//    attpert[0][0] = pitch_cos * yaw_cos;
//    attpert[0][1] = yaw_cos * roll_sin * pitch_sin + roll_cos * yaw_sin;
//    attpert[0][2] = roll_sin * yaw_sin - pitch_sin * roll_cos * yaw_cos;
//
//    attpert[1][0] = -yaw_sin * pitch_cos;
//    attpert[1][1] = yaw_cos * roll_cos - yaw_sin * roll_sin * pitch_sin;
//    attpert[1][2] = pitch_sin * yaw_sin * roll_cos + yaw_cos * roll_sin;
//
//    attpert[2][0] = pitch_sin;
//    attpert[2][1] = -roll_sin * pitch_cos;
//    attpert[2][2] = pitch_cos * roll_cos;

    attpert[0][0] = 1;
    attpert[0][1] = 0;
    attpert[0][2] = 0;

    attpert[1][0] = 0;
    attpert[1][1] = 1;
    attpert[1][2] = 0;

    attpert[2][0] = 0;
    attpert[2][1] = 0;
    attpert[2][2] = 1;


    return SUCCESS;
}
