/*******************************************************************************
NAME:            ias_math_multiply_3x3_matrix

PURPOSE:      
Multiply two 3 x 3 matrices

RETURN VALUE:
none

*******************************************************************************/
#include "ias_math.h"

void  ias_math_multiply_3x3_matrix
(
    double matrix1[3][3],       /* I: Input matrix one */
    double matrix2[3][3],       /* I: Input matrix two */
    double outmatrix[3][3]      /* O: Multiplied matrix */
)
{
    double     sum;             /* Summary of multiplied items from old 
                                   matrices to form item in the new matrix */
    int        i, j, k;         /* Looping variables            */

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            sum = 0.0;
            for (k = 0; k < 3; k++)
                sum += (matrix1[i][k] * matrix2[k][j]);

            outmatrix[i][j] = sum; 
        }
    }
}
