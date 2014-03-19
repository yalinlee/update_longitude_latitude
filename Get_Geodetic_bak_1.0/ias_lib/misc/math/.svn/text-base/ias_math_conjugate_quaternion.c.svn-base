/******************************************************************************
NAME: ias_math_conjugate_quaternion

PURPOSE: Conjugate a quaternion

******************************************************************************/
#include "ias_math.h"                /* ias_math prototype */

void ias_math_conjugate_quaternion
(
    const IAS_QUATERNION *quaternion, /* I: the quaternion to conjugate */
    IAS_QUATERNION *conjugated_quaternion /* O: the conjugated quaternion */
)
{
    /* Calculate the conjugate of the quaternion. */ 
    conjugated_quaternion->vector.x = -quaternion->vector.x;
    conjugated_quaternion->vector.y = -quaternion->vector.y;
    conjugated_quaternion->vector.z = -quaternion->vector.z;
    conjugated_quaternion->scalar = quaternion->scalar;
}

