/*----------------------------------------------------------------------------
NAME:    ias_pm_get_image

PURPOSE: Convert the pixel mask into an image suitable for writing to a
         file.  It is advised to only use this for a utility to write out
         the mask as an image for viewing by the user.  To walk through
         an image, the iterator functionality should be used since it is
         more efficient.

RETURNS: SUCCESS or ERROR

-----------------------------------------------------------------------------*/
#include <string.h>
#include "pm_local.h"
#include "ias_logging.h"
#include "ias_const.h"

int ias_pm_get_image
(
    IAS_PIXEL_MASK *pixel_mask_ptr,/* I: Pointer to a pixel mask structure */
    PIXEL_MASK_TYPE *image_data    /* I/O: Pointer to allocated array for
                                      image data */
)
{
    IAS_PIXEL_MASK_ITERATOR *it; /* Interator structure */
    IAS_PIXEL_MASK_SPAN span;    /* Span structure */
    PIXEL_MASK_TYPE *image_ptr;
    int number_detectors = pixel_mask_ptr->num_of_detectors;
    int pixel;
    int i;

    /* Clear the memory so that an iterator of type IAS_PM_INCLUDE can be
       used.  A pixel mask of the complement of 0 will include all spans with
       a mask set. */
    memset( image_data, 0, pixel_mask_ptr->num_of_pixels *
        number_detectors * sizeof(*image_data));

    it = ias_pm_get_iterator( pixel_mask_ptr, IAS_PM_INCLUDE, ~0x0 );
    if ( !it )
    {
        IAS_LOG_ERROR("Unable to create a pixel mask iterator");
        return ERROR;
    }

    /* Get all the masked spans and set the array memory to the mask value */
    while ( ias_pm_get_next_span( it, &span ))
    {
        image_ptr = &image_data[span.detector_index];

        for ( i = 0, pixel = span.starting_pixel_index;
            i < span.length_of_span; i++, pixel++ )
        {
            image_ptr[pixel * number_detectors] = span.pixel_mask;
        }
    }

    ias_pm_destroy_iterator( it );
    return SUCCESS;
}  /* END ias_pm_get_image */
