/******************************************************************************
NAME: ias_math_convert_euler_to_quaternion

PURPOSE: Converts a Euler angle sequence to a quaternion

RETURN VALUE:
    Type = int
    Value    Description
    -----    -----------
    SUCCESS  Successful completion
    ERROR    Operation failed

******************************************************************************/
#include <math.h>

#include "ias_const.h"
#include "ias_logging.h"
#include "ias_math.h"                /* ias_math prototype */

int ias_math_convert_euler_to_quaternion
(
    double tolerance,          /* I: error tolerance for the conversion */
    double matrix[3][3],       /* I: matrix to convert */
    IAS_QUATERNION *quaternion /* O: converted results */
)
{
    double d1, d2, d3, d4;
    double q1, q2, q3, q4;

    d1 = 1.0 + matrix[0][0] - matrix[1][1] - matrix[2][2];
    if (d1 < 0.0) 
        d1 = 0.0;
    else 
        d1 = sqrt(d1) / 2.0;

    d2 = 1.0 - matrix[0][0] + matrix[1][1] - matrix[2][2];
    if (d2 < 0.0) 
        d2 = 0.0;
    else 
        d2 = sqrt(d2) / 2.0;

    d3 = 1.0 - matrix[0][0] - matrix[1][1] + matrix[2][2];
    if (d3 < 0.0) 
        d3 = 0.0;
    else 
        d3 = sqrt(d3) / 2.0;

    d4 = 1.0 + matrix[0][0] + matrix[1][1] + matrix[2][2];
    if (d4 < 0.0) 
        d4 = 0.0;
    else 
        d4 = sqrt(d4) / 2.0;

    if (d1 > d2 && d1 > d3 && d1 > d4)
    {
        q1 = d1;
        q2 = (matrix[0][1] + matrix[1][0]) / (4.0 * q1);
        q3 = (matrix[0][2] + matrix[2][0]) / (4.0 * q1);
        q4 = (matrix[1][2] - matrix[2][1]) / (4.0 * q1);

        if (fabs(d2 - fabs(q2)) > tolerance
            || fabs(d3 - fabs(q3)) > tolerance
            || fabs(d4 - fabs(q4)) > tolerance)
        {
            IAS_LOG_ERROR("Converting Euler to quaternions");
            return ERROR;
        }
    }
    else if (d2 > d1 && d2 > d3 && d2 > d4)
    {
        q2 = d2;
        q1 = (matrix[0][1] + matrix[1][0]) / (4.0 * q2);
        q3 = (matrix[1][2] + matrix[2][1]) / (4.0 * q2);
        q4 = (matrix[2][0] - matrix[0][2]) / (4.0 * q2);

        if (fabs(d1 - fabs(q1)) > tolerance
            || fabs(d3 - fabs(q3)) > tolerance
            || fabs(d4 - fabs(q4)) > tolerance)
        {
            IAS_LOG_ERROR("Converting Euler to quaternions");
            return ERROR;
        }
    }
    else if (d3 > d1 && d3 > d2 && d3 > d4)
    {
        q3 = d3;
        q1 = (matrix[0][2] + matrix[2][0]) / (4.0 * q3);
        q2 = (matrix[1][2] + matrix[2][1]) / (4.0 * q3);
        q4 = (matrix[0][1] - matrix[1][0]) / (4.0 * q3);

        if (fabs(d1 - fabs(q1)) > tolerance
            || fabs(d2 - fabs(q2)) > tolerance
            || fabs(d4 - fabs(q4)) > tolerance)
        {
          IAS_LOG_ERROR("Converting Euler to quaternions");
          return ERROR;
        }
    }
    else
    {
        q4 = d4;
        q1 = (matrix[1][2] - matrix[2][1]) / (4.0 * q4);
        q2 = (matrix[2][0] - matrix[0][2]) / (4.0 * q4);
        q3 = (matrix[0][1] - matrix[1][0]) / (4.0 * q4);

        if (fabs(d1 - fabs(q1)) > tolerance
            || fabs(d2 - fabs(q2)) > tolerance
            || fabs(d3 - fabs(q3)) > tolerance)
        {
            IAS_LOG_ERROR("Converting Euler to quaternions");
            return ERROR;
        }
    }

    /* Make sure the q4 component is positive to make it better match the
       spacecraft quaternions */
    if ( q4 < 0.0 )
    {
        quaternion->vector.x = -q1;
        quaternion->vector.y = -q2;
        quaternion->vector.z = -q3;
        quaternion->scalar = -q4;
    }
    else
    {
        quaternion->vector.x = q1;
        quaternion->vector.y = q2;
        quaternion->vector.z = q3;
        quaternion->scalar = q4;
    }

    return SUCCESS;
}

