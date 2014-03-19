/******************************************************************************
NAME: ias_math_transform_3dvec

PURPOSE: Transform a vector matrix by multiplication.

RETURNS: nothing

ALGORITHM DESCRIPTION:
    Xnew = Trans * Xold;

******************************************************************************/
#include "ias_math.h"

void ias_math_transform_3dvec
(
    const IAS_VECTOR *Xold, /* I: vector in old system */
    double Trans[3][3],     /* I:transformation matrix from old to new system*/
    IAS_VECTOR *Xnew        /* O: vector in the new system */
)
{
    /* Calculate the new vector */
    Xnew->x  = Trans[0][0] * Xold->x ;
    Xnew->x  += Trans[0][1] * Xold->y ;
    Xnew->x  += Trans[0][2] * Xold->z ;

    Xnew->y  = Trans[1][0] * Xold->x ;
    Xnew->y  += Trans[1][1] * Xold->y ;
    Xnew->y  += Trans[1][2] * Xold->z ;

    Xnew->z  = Trans[2][0] * Xold->x ;
    Xnew->z  += Trans[2][1] * Xold->y ;
    Xnew->z  += Trans[2][2] * Xold->z ;
}
