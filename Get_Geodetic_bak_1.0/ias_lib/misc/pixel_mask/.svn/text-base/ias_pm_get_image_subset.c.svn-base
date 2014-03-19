/*----------------------------------------------------------------------------
NAME:    ias_pm_get_image_subset

PURPOSE: Return a subset of a pixel mask.  The pixel mask is detector based
         so the returned image is also detector based: image_mask_subset[0]
         contains the masks for detector 0.  In other words, the image is
         transposed.

RETURNS: SUCCESS or ERROR

-----------------------------------------------------------------------------*/
#include <string.h>
#include "pm_local.h"
#include "ias_logging.h"
#include "ias_const.h"

static int get_overlap_extents
(
    int span_start_pixel_index, /* I: Start of span */
    int span_number_pixels,     /* I: Number of pixels in span */
    int sub_start_pixel_index,  /* I: Starting pixel of the subset */
    int sub_last_pixel,         /* I: Last pixel of the subset */
    int *overlap_start,         /* O: Index of starting pixel in overlap */
    int *overlap_num_pixels     /* O: Number of pixels in overlap */
);

int ias_pm_get_image_subset
(
    IAS_PIXEL_MASK *pixel_mask_ptr,     /* I: Pixel mask */
    int start_detector,                 /* I: Starting detector 0-based */
    int start_pixel_index,              /* I: Starting pixel number 0-based */
    int number_of_detectors,            /* I: Number of detectors */
    int number_of_pixels,               /* I: Number of pixels */
    PIXEL_MASK_TYPE *image_mask_subset  /* O: Image subset */
)
{
    IAS_PIXEL_MASK_ITERATOR *it; /* Interator structure */
    IAS_PIXEL_MASK_SPAN span;    /* Span structure */
    unsigned int array_offset;   /* Offset into array for detector start */
    int end_pixel = start_pixel_index + number_of_pixels - 1;
    int end_detector = start_detector + number_of_detectors - 1;
    int sub_image_start_index;
    int sub_image_npixels;

    /* Clear the memory so that an iterator of type IAS_PM_INCLUDE can be
       used.  A pixel mask of the complement of 0 will include all spans with
       a mask set. */
    memset( image_mask_subset, 0, number_of_pixels * number_of_detectors
        * sizeof(*image_mask_subset));

    it = ias_pm_get_iterator_at_detector( pixel_mask_ptr, IAS_PM_INCLUDE,
        ~0x0, start_detector );
    if ( !it )
    {
        IAS_LOG_ERROR("Unable to create a pixel mask iterator");
        return ERROR;
    }

    /* Get the first span.  Note that if no span is returned it is assumed
       the there are no artifacts and the subset is correctly set */
    if ( !ias_pm_get_next_span( it, &span ))
    {
        ias_pm_destroy_iterator( it );
        return SUCCESS;
    }

    /* The span returned should be set for the starting detector or greater.
       If it's less, it's considered an error. */
    if ( span.detector_index < start_detector )
    {
        IAS_LOG_ERROR("Unexpected pixel span detector %d, should be %d",
            span.detector_index, start_detector );
        ias_pm_destroy_iterator( it );
        return ERROR;
    }

    /* While the span is within the specified pixel extents */
    while ( span.detector_index <= end_detector )
    {
        if ( get_overlap_extents( span.starting_pixel_index,
                 span.length_of_span, start_pixel_index, end_pixel,
                 &sub_image_start_index, &sub_image_npixels ))
        {
            array_offset = number_of_pixels
                * ( span.detector_index - start_detector );
            /* Note: if the PIXEL_MASK_TYPE is changed to something other
               than an 8-byte value this memset will no longer work
               properly. */
            memset( image_mask_subset + array_offset +
                sub_image_start_index, span.pixel_mask,
                sub_image_npixels);
        }

        /* Get the next span */
        if ( !ias_pm_get_next_span( it, &span ))
            break;
    }

    ias_pm_destroy_iterator( it );
    return SUCCESS;
}  /* END ias_pm_get_image_subset */

/*--------------------------------------------------------------------------
   PURPOSE: Local routine to determine the overlapping portion of a span
            and an image subset.
   RETURNS: 1 if there is any overlap
            0 if no overlap.
--------------------------------------------------------------------------*/
static int get_overlap_extents
(
    int span_start_pixel_index, /* I: Start of span */
    int span_number_pixels,     /* I: Number of pixels in span */
    int sub_start_pixel_index,  /* I: Starting pixel of the subset */
    int sub_last_pixel,         /* I: Last pixel of the subset */
    int *overlap_start,         /* O: Index of starting pixel in overlap */
    int *overlap_num_pixels     /* O: Number of pixels in overlap */
)
{
    int span_end_pixel = span_start_pixel_index + span_number_pixels - 1;

    /* If the span completely overlaps the sub-image set the start and end
       indices and return. */
    if ( span_start_pixel_index < sub_start_pixel_index &&
         span_end_pixel > sub_last_pixel )
    {
        *overlap_start = 0;
        *overlap_num_pixels = sub_last_pixel - sub_start_pixel_index + 1;
        return 1;
    }

    /* See if the end of the span falls within the subimage */
    if ( span_end_pixel >= sub_start_pixel_index && span_end_pixel <=
         sub_last_pixel )
    {
        /* If so adjust the start and number of pixels */
        if ( span_start_pixel_index < sub_start_pixel_index )
        {
            *overlap_start = 0;
            *overlap_num_pixels = span_start_pixel_index + span_number_pixels
                - sub_start_pixel_index; }
        else
        {
            *overlap_start = span_start_pixel_index - sub_start_pixel_index;
            *overlap_num_pixels = span_number_pixels;
        }
        return 1;
    }

    /* See if the start of the span falls within the subimage */
    if ( span_start_pixel_index >= sub_start_pixel_index &&
         span_start_pixel_index <= sub_last_pixel )
    {
        /* If so adjust the start and number of pixels */
        *overlap_start = span_start_pixel_index - sub_start_pixel_index;
        if ( span_end_pixel <= sub_last_pixel )
            *overlap_num_pixels = span_number_pixels;
        else
            *overlap_num_pixels = sub_last_pixel - span_start_pixel_index + 1;
        return 1;
    }

    return 0;
}  /* END get_overlap_extents */
