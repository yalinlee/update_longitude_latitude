/*******************************************************************************
NAME:           ias_math_invert_3x3_matrix

PURPOSE:        
Calculates the inverse of a 3 x 3 matrix

RETURN VALUE:
Type = int (SUCCESS or ERROR)

ALGORITHM REFERENCES:
        Applied Linear Statistical Models
        by John Neter and William Wasserman
        (Richard D. Irwin, INC 1974)
        page 234

         _     _              _     _
        | a b c |            | A B C |
    X=  | d e f |   (X)^-1 = | D E F | * 1 / Z
        | g h i |            | G H I |
         -     -              -     -
*******************************************************************************/
#include "ias_logging.h"
#include "ias_math.h"

#define LIMIT 1e-9

int  ias_math_invert_3x3_matrix
( 
    double inmatrix[3][3],     /* I Input matrix         */
    double outmatrix[3][3]     /* O Inverted matrix      */
)
{
    double A,                  /* Temp variables for calculations */
           B,
           C,
           D,
           E,
           F,
           G,
           H,
           K,
           Z;
          
    A = inmatrix[1][1] * inmatrix[2][2] - inmatrix[1][2] * inmatrix[2][1];
    B = inmatrix[0][2] * inmatrix[2][1] - inmatrix[0][1] * inmatrix[2][2];
    C = inmatrix[0][1] * inmatrix[1][2] - inmatrix[0][2] * inmatrix[1][1];
    D = inmatrix[1][2] * inmatrix[2][0] - inmatrix[1][0] * inmatrix[2][2];
    E = inmatrix[0][0] * inmatrix[2][2] - inmatrix[0][2] * inmatrix[2][0];
    F = inmatrix[0][2] * inmatrix[1][0] - inmatrix[0][0] * inmatrix[1][2];
    G = inmatrix[1][0] * inmatrix[2][1] - inmatrix[1][1] * inmatrix[2][0];
    H = inmatrix[0][1] * inmatrix[2][0] - inmatrix[0][0] * inmatrix[2][1];
    K = inmatrix[0][0] * inmatrix[1][1] - inmatrix[0][1] * inmatrix[1][0];
    Z = inmatrix[0][0] * A + inmatrix[0][1] * D + inmatrix[0][2] * G;

    if (Z <= LIMIT)     /* if Z is zero */
    {
        IAS_LOG_ERROR ("Error: calculations will result in divide by zero.");
        return ERROR;
    }

    outmatrix[0][0] = A / Z;
    outmatrix[0][1] = B / Z;
    outmatrix[0][2] = C / Z;

    outmatrix[1][0] = D / Z;
    outmatrix[1][1] = E / Z;
    outmatrix[1][2] = F / Z;

    outmatrix[2][0] = G / Z;
    outmatrix[2][1] = H / Z;
    outmatrix[2][2] = K / Z;

    return SUCCESS;
}
