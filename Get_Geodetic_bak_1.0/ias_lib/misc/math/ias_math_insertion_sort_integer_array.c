/******************************************************************************
NAME:           ias_math_insertion_sort_integer_array

PURPOSE:  This routine will sort an array of integers into numerical 
ascending order using the insertion sort algorithm.  It should only be
used for small inputs.


RETURN VALUE:   Type = void

ALGORITHM REFERENCES:

******************************************************************************/
#include "ias_math.h"

void ias_math_insertion_sort_integer_array
(
    int num_values,             /* I: Number of values to sort        */
    int sort_array[]            /* I/O: Array of integers to sort     */
)
{
    int i, j;                   /* Loop counters                      */
    int array_element;          /* single array element               */

    /* Sort each value one by one, scanning through the array. */
    for (j = 1; j < num_values; j++)
    {
        array_element = sort_array[j];
        i = j - 1;

        /* Find where this element belongs in the list sorted so far. */
        while (i >= 0 && sort_array[i] > array_element)
        {
            /* Shift the sorted values greater than this element to
               the right. */
            sort_array[i+1] = sort_array[i];
            i--;
        }

        /* Put this element where it belongs in the sorted list. */
        sort_array[i+1] = array_element;
    }
}

