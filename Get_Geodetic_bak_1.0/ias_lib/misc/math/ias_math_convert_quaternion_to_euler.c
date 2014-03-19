/******************************************************************************
NAME: ias_math_convert_quaternion_to_euler

PURPOSE: Converts a quaternion to a Euler angle sequence

NOTE:
   To represent the ACStoECI matrix needed by the model that is the
   transpose of what is traditionally defined as quaternion-to-euler
******************************************************************************/
#include "ias_math.h"                /* ias_math prototype */

void ias_math_convert_quaternion_to_euler
(
    const IAS_QUATERNION *quaternion, /* I: the quaternion to convert */
    double matrix[3][3]               /* O: the converted results */
)
{
    /* do most of the multiplies here */
    double xx = quaternion->vector.x * quaternion->vector.x;
    double xy = quaternion->vector.x * quaternion->vector.y;
    double xz = quaternion->vector.x * quaternion->vector.z;
    double xs = quaternion->vector.x * quaternion->scalar;

    double yy = quaternion->vector.y * quaternion->vector.y;
    double yz = quaternion->vector.y * quaternion->vector.z;
    double ys = quaternion->vector.y * quaternion->scalar;

    double zz = quaternion->vector.z * quaternion->vector.z;
    double zs = quaternion->vector.z * quaternion->scalar;

    double ss = quaternion->scalar * quaternion->scalar;

    /**********************************/
    matrix[0][0] = xx - yy - zz + ss;
    matrix[0][1] = 2.0 * (xy + zs);
    matrix[0][2] = 2.0 * (xz - ys);

    /**********************************/
    matrix[1][0] = 2.0 * (xy - zs);
    matrix[1][1] = yy - zz + ss - xx;
    matrix[1][2] = 2.0 * (yz + xs);

    /**********************************/
    matrix[2][0] = 2.0 * (xz + ys);
    matrix[2][1] = 2.0 * (yz - xs);
    matrix[2][2] = zz + ss - xx - yy;
}

