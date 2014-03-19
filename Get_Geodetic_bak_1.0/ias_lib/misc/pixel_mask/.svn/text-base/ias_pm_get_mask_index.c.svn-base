/*************************************************************************
 NAME:                 ias_pm_get_mask_index

 PURPOSE:     Given an array of pixel masks and specified band and
              SCA numbers, returns the corresponding index of the
              mask

 RETURNS:     Integer representing (0-based) array index if successful,
              -1 if an error occurs

*************************************************************************/
#include "ias_logging.h"
#include "ias_pixel_mask.h"
#include "pm_local.h"
#include "ias_const.h"


int ias_pm_get_mask_index
(
    IAS_PIXEL_MASK **pm_array,      /* I: Pixel mask array to search */
    int number_of_masks,            /* I: Number of masks in array */
    int band_number,                /* I: Current 1-based band number */
    int sca_number                  /* I: Current 1-based SCA number */
)
{
    int index;                      /* Local loop counter for array
                                       indices */
    IAS_PIXEL_MASK *current_pm = NULL;    /* Pointer to current mask in
                                             array */


    /* Start with the first mask, and check the band and SCA numbers
       recorded for it against the specified band/SCA numbers */
    for (index = 0; index < number_of_masks; index++)
    {
        current_pm = (IAS_PIXEL_MASK *)pm_array[index];

        /* If there's a match, return the index */
        if ((current_pm->band == band_number)
                && (current_pm->sca == sca_number))
        {
            return index;
        }
    }

    /* If we didn't find the mask with the specified band/SCA, return a
       flag value of -1 (since array indices are assumed to be
       positive-valued).  In some contexts, it would be treated as an error;
       in others, however, it would indicate a missing band/SCA combination
       that shouldn't necessarily be treated as an error...  */
    return -1;
}
