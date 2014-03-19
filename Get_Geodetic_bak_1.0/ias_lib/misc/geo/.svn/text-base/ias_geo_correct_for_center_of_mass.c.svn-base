/******************************************************************************
NAME:  ias_geo_correct_for_center_of_mass

PURPOSE:
Adjust observation position according to the spacecraft center of mass to
instrument offset.

RETURNS: void

******************************************************************************/
#include "ias_math.h"
#include "ias_geo.h"

void ias_geo_correct_for_center_of_mass
(
   double cm2oli[3],        /* I: sensor to center-of-mass to OLI vector */
   double orb2ecf[3][3],    /* I: Orbit to ECF transformation */
   double attpert[3][3],    /* I: Attitude perturbation matrix */
   const IAS_VECTOR *npos,  /* I: ECF position vector */
   IAS_VECTOR *cpos         /* O: ECF position vector adjusted for CM */
)
{
    IAS_VECTOR ocm2oli;          /* Orbital CM to OLI vector */
    IAS_VECTOR ecf2ocm;          /* ECF TO Orbital CM vector */

    /* Perturb (orbital) center of mass vector */ 
    ocm2oli.x = cm2oli[0] * attpert[0][0]
              + cm2oli[1] * attpert[0][1]
              + cm2oli[2] * attpert[0][2];
    ocm2oli.y = cm2oli[0] * attpert[1][0]
              + cm2oli[1] * attpert[1][1]
              + cm2oli[2] * attpert[1][2];
    ocm2oli.z = cm2oli[0] * attpert[2][0]
              + cm2oli[1] * attpert[2][1]
              + cm2oli[2] * attpert[2][2];

    /* Calculate ECF CM vector */
    ias_math_transform_3dvec(&ocm2oli, orb2ecf, &ecf2ocm);

    /* Calculate new ECF LOS vector adjusted for CM */ 
    cpos->x = npos->x + ecf2ocm.x; 
    cpos->y = npos->y + ecf2ocm.y;
    cpos->z = npos->z + ecf2ocm.z;
}
