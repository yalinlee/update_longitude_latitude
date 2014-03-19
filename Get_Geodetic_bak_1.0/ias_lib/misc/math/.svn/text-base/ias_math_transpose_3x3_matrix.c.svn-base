/******************************************************************************
NAME:           ias_math_transpose_3x3_matrix

PURPOSE:        Transpose a 3 X 3 matrix;

******************************************************************************/
#include "ias_math.h"

void ias_math_transpose_3x3_matrix
(
    double A[3][3]        /* I/O: the matrix to be transposed          */
)
{
    int i, j;             /* looping variable                          */
    double s;             /* intermediate variable                     */

    /* Transpose a 3 X 3 matrix. */
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < i; j++)
        {
            s = A[i][j];
            A[i][j] = A[j][i];
            A[j][i] = s;
        }
    }  
}
