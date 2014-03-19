/*****************************************************************************
NAME: ias_geo_convert_rpy_to_matrix

PURPOSE: Convert the roll, pitch, and yaw vector into a matrix version.

RETURNS: Nothing
*****************************************************************************/

#include <math.h>

#include <ias_structures.h>

void ias_geo_convert_rpy_to_matrix
(
    const IAS_VECTOR *rpy,/* I: attitude vector to convert */
    double matrix[3][3]   /* O: resulting matrix */
)
{
    double cosine_roll;   /* trig functions */
    double cosine_pitch;
    double cosine_yaw;
    double sine_roll;
    double sine_pitch;
    double sine_yaw;

    cosine_roll  = cos(rpy->x);
    sine_roll    = sin(rpy->x);
    cosine_pitch = cos(rpy->y);
    sine_pitch   = sin(rpy->y);
    cosine_yaw   = cos(rpy->z);
    sine_yaw     = sin(rpy->z);

    /*****************************************************/
    matrix[0][0] = cosine_pitch * cosine_yaw;
    matrix[0][1] = cosine_yaw * sine_roll * sine_pitch
                   + cosine_roll * sine_yaw;
    matrix[0][2] = sine_roll * sine_yaw
                   - sine_pitch * cosine_roll * cosine_yaw;

    /*****************************************************/
    matrix[1][0] = -sine_yaw * cosine_pitch;
    matrix[1][1] = cosine_yaw * cosine_roll
                   - sine_yaw * sine_roll * sine_pitch;
    matrix[1][2] = sine_pitch * sine_yaw * cosine_roll
                   + cosine_yaw * sine_roll;

    /*****************************************************/
    matrix[2][0] = sine_pitch;
    matrix[2][1] = -sine_roll * cosine_pitch;
    matrix[2][2] = cosine_pitch * cosine_roll;
}
