/*******************************************************************************
NAME: ias_math_rotate_3dvec_around_x

PURPOSE:        
Transformation of coordinates through rotation around the x-axis and
specified angle.

RETURN VALUE:
None

*******************************************************************************/
#include <math.h>
#include "ias_math.h"

void  ias_math_rotate_3dvec_around_x
(
    const IAS_VECTOR *r_old, /* I: coordinates (x, y, z) in the old system */
    double angle,        /* I: angle to be rotated, in radian, positive
                               anticlockwise                           */
    IAS_VECTOR *r_new    /* O: coordinates in the new system           */
)
{
    double ca, sa;                /* cosine and sine of angle          */

    /* Compute the sine and cosine of the angle. */
    ca = cos(angle);
    sa = sin(angle);

    /* Calculate the new coords using the following:
    * new x coord = old x coord
    * new y coord = cos * old y coord + sine * old z coord
    * new z coord = cos * old z coord - sine * old y coord
    */
    r_new->x = r_old->x;
    r_new->y = ca * r_old->y + sa * r_old->z;
    r_new->z = ca * r_old->z - sa * r_old->y;
}


/******************************************************************************
NAME: ias_math_rotate_3dvec_around_y(r_old, angle, r_new)

PURPOSE: Transformation of coordinates through rotation around the
         y-axis and specified angle.

RETURN VALUE:
None

******************************************************************************/
void  ias_math_rotate_3dvec_around_y
(
    const IAS_VECTOR *r_old, /* I: coordinates (x, y, z) in the old system */
    double angle,     /* I: angle to be rotated, in radian, positive
                            anticlockwise                           */
    IAS_VECTOR *r_new /* O: coordinates in the new system          */
)
{
    double ca, sa;       /* cosine and sine of angle                */

    /* Compute the sine and cosine of the angle. */
    ca = cos(angle);
    sa = sin(angle);

    /* Calculate the new coords using the following:
     * new x coord = cosine * old x coord - sine * old z coord
     * new y coord = old y coord
     * new z coord = cosine * old z coord + sine * old x coord
     */
    r_new->x = ca * r_old->x - sa * r_old->z;
    r_new->y = r_old->y;
    r_new->z = ca * r_old->z + sa * r_old->x;
}


/******************************************************************************
NAME: ias_math_rotate_3dvec_around_z

PURPOSE: Transformation of coordinates through rotation around the
         Z-axis and specified angle.

RETURN VALUE:
None

******************************************************************************/
void  ias_math_rotate_3dvec_around_z
(
    const IAS_VECTOR *r_old, /* I: coordinates (x, y, z) in the old system */
    double angle,        /* I: angle to be rotated, in radian, positive
                                  anticlockwise                        */
    IAS_VECTOR *r_new    /* O: coordinates in the new system           */
)
{
    double ca, sa;         /* cosine and sine of angle                 */

    /* Compute the sine and cosine of the angle. */
    ca = cos(angle);
    sa = sin(angle);

    /* Calculate the new coords using the following:
     * new x coord = cosine * old x coord + sine * old y coord
     * new y coord = cosine * old y coord - sine * old x coord
     * new z coord = old z coord */
    r_new->x = ca * r_old->x + sa * r_old->y;
    r_new->y = ca * r_old->y - sa * r_old->x;
    r_new->z = r_old->z;
}
