/*******************************************************************************
NAME:           ias_math_compute_3dvec_cross

PURPOSE:
Compute the cross product of two vectors

RETURN VALUE:
none

*******************************************************************************/
#include "ias_math.h"

void ias_math_compute_3dvec_cross
(
    const IAS_VECTOR *vec1,     /* I: Vector one       */
    const IAS_VECTOR *vec2,     /* I: Vector two       */
    IAS_VECTOR *vec3            /* O: Output vector (cross product) */
)
{
    vec3->x = vec1->y * vec2->z - vec1->z * vec2->y; 
    vec3->y = vec1->z * vec2->x - vec1->x * vec2->z;
    vec3->z = vec1->x * vec2->y - vec1->y * vec2->x;
}
