
/******************************************************************************
NAME: ias_math_check_pixels_in_range

PURPOSE:  Check whether the input window's percentage of data outside a 
          specified range is over a threshold.

RETURN VALUE:   Type = int
Value    Description
-----    -----------
  1      Out of range data was not over the specified amount.
  0      Out of range data was over the specified amount.

NOTES:
This routine is used to determine if there is too much out of range data in 
a window.  If too much out of range data exists, then the window might not be 
good for correlating.  This may be used, for example, to exclude windows in
lunar images that have too much deep space background. 

******************************************************************************/
#include "ias_math.h"

int ias_math_check_pixels_in_range
(
    const float *window,         /* I: Image window                          */
    int  size,                   /* I: Size of the window (nlines * nsamps)  */
    float invalid_thresh,        /* I: Threshold for out of range data       */
    float valid_image_max,       /* I: Upper bound defining valid vs invalid */
    float valid_image_min        /* I: Lower bound defining valid vs invalid */
)
{
    int i;                       /* Loop variable                            */
    int count = 0;               /* Count of out of range pixels             */
    float out_of_range;          /* Fraction of out of range data            */

    /* If the size is <= 0, then don't process. */
    if (size <= 0)
        return 1;

    /* Count the number of out of bounds pixels in the window. */
    for (i = 0; i < size; i++)
        if ((window[i] > valid_image_max) || (window[i] < valid_image_min))
            count++;

    /* Determine the amount of out of bounds data. */
    out_of_range = (float) count / (float) size;

    /* If the percent out of range is > the specified threshold, return 0. */
    if (out_of_range > invalid_thresh)
        return 0;

    /* Otherwise return 1. */
    return 1;
}
