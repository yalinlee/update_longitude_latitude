/****************************************************************************
NAME:        ias_math_find_median_unsigned

PURPOSE:  Find the median value of 16-bit unsigned integer array

RETURN VALUE:
Median value

ALGORITHM REFERENCES:
1.0  LAS 4.0 GREYCORR and EDGECORR by R. White 6/83

*****************************************************************************/
#include <string.h>
#include "ias_math.h"                /* ias_math prototype */

/****************************************************************************
NAME: sort

PURPOSE: Sort the integer array from low to high
*****************************************************************************/
static void sort
(
    int N,                      /* I: Number of elements in the array */
    unsigned short int z[]      /* I: An integer array to sort */
) 
{
    int i, j;
   
    for (i = 0; i < N-1; i++) 
    {
        for (j = i+1; j < N; j++) 
        {
            if (z[j] < z[i]) 
            {
                unsigned short int tmp;
                tmp = z[j]; 
                z[j] = z[i]; 
                z[i] = tmp;
            } 
       }
    } 
}

/****************************************************************************
NAME:        ias_math_find_median_unsigned

PURPOSE:  Find the median value of integer array

RETURN VALUE:
Median value
*****************************************************************************/
unsigned short int ias_math_find_median_unsigned
(
    int N,                      /* I: Number of elements in the array */
    const unsigned short int z[]/* I: An integer array to find median from */
) 
{
    unsigned short int q[N];
    unsigned short int med;

    memcpy(q, z, sizeof(unsigned short int) * N);
    sort(N, q);
    med = q[N/2];
    return med;

}

