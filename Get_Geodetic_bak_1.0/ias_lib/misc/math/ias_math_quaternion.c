/*******************************************************************************
NAME: ias_math_quaterion

PURPOSE: Contains various quaternion math routines.  The individual routines
    are all small and do not warrant being in separate files.

*******************************************************************************/
#include <math.h>
#include "ias_math.h"

/*******************************************************************************
NAME: ias_math_compute_quaternion_magnitude

PURPOSE: Find the magnitude of a quaternion

RETURN VALUE:
    TYPE = double
    Calculated magnitude of the quaternion
*******************************************************************************/
double ias_math_compute_quaternion_magnitude
(
    const IAS_QUATERNION *quat /* I: Quaternion to find the magnitude of */
)
{
    /* Calculate the magnitude */
    return (sqrt(quat->vector.x * quat->vector.x +
                 quat->vector.y * quat->vector.y +
                 quat->vector.z * quat->vector.z +
                 quat->scalar * quat->scalar));
}

/*******************************************************************************
NAME: ias_math_multiply_quaternions

PURPOSE: Multiply two quaternions

RETURNS: nothing
*******************************************************************************/
void ias_math_multiply_quaternions
(
    const IAS_QUATERNION *q1,       /* I: first quaternion */
    const IAS_QUATERNION *q2,       /* I: second quaternion */
    IAS_QUATERNION *result          /* O: multiplication result */
)
{
    double d;
    IAS_VECTOR t;

    d = ias_math_compute_3dvec_dot(&q1->vector, &q2->vector);
    ias_math_compute_3dvec_cross(&q1->vector, &q2->vector, &t);

    result->scalar = q1->scalar * q2->scalar - d;
    result->vector.x = q2->scalar * q1->vector.x + q1->scalar * q2->vector.x
        - t.x;
    result->vector.y = q2->scalar * q1->vector.y + q1->scalar * q2->vector.y
        - t.y;
    result->vector.z = q2->scalar * q1->vector.z + q1->scalar * q2->vector.z
        - t.z;
}

/*******************************************************************************
NAME: ias_math_convert_quaternion2rpy

PURPOSE: Convert a quaternion to roll-pitch-yaw

RETURNS: nothing
*******************************************************************************/
void ias_math_convert_quaternion2rpy
(
     const IAS_QUATERNION *quat,    /* I: quaternion to convert */
     IAS_VECTOR *att                /* O: output roll-pitch-yaw */
)
{
    double m21;
    double m22;
    double m20;
    double m10;
    double m00;

    m21 = 2.0 * (quat->vector.y * quat->vector.z
        - quat->vector.x * quat->scalar);
    m22 = quat->vector.z * quat->vector.z + quat->scalar * quat->scalar
        - quat->vector.x * quat->vector.x - quat->vector.y * quat->vector.y;
    m20 = 2.0 * (quat->vector.x * quat->vector.z
        + quat->vector.y * quat->scalar);
    m10 = 2.0 * (quat->vector.x * quat->vector.y
        - quat->vector.z * quat->scalar);
    m00 = quat->vector.x * quat->vector.x - quat->vector.y * quat->vector.y
        - quat->vector.z * quat->vector.z + quat->scalar * quat->scalar;

    /* Calculate the attitude roll, pitch, and yaw */
    att->x = -atan2(m21, m22);
    att->y =  asin(m20);
    att->z = -atan2(m10, m00);
}
