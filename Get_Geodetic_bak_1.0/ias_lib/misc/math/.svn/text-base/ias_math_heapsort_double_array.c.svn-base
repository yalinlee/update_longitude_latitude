/******************************************************************************
NAME:           ias_math_heapsort_double_array

PURPOSE:  This routine will sort an array into numerical ascending order using
the Heapsort algorithm.

RETURN VALUE: none

******************************************************************************/
#include "ias_math.h"

void ias_math_heapsort_double_array
(
    int n,        /* I: number of elements        */
    double *ra    /* I/O: element values to sort  */
)
{
    int l;        /* array counters & indices     */
    int j;
    int ir;
    int i;
    double rra;   /* temp array holder for ra     */

    if (n <= 1)
        return;

    l = n/2;
    ir = n - 1;
    for (;;) 
    {
        if (l > 0)
        {
            l = l - 1;
            rra = ra[l];
        }
        else
        {
            rra = ra[ir];
            ra[ir] = ra[0];
            ir = ir - 1;
            if (ir == 0)
            {
                ra[0] = rra;
                return;
            }
        }
        i = l;
        j = l * 2 + 1;
        while (j <= ir)
        {
            if (j < ir && ra[j] < ra[j+1])
                ++j;
            if (rra < ra[j])
            {
                ra[i] = ra[j];
                i = j;
                j += i + 1;
            }
            else 
                j = ir + 1;
        }
        ra[i] = rra;
    }
}
