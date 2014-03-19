/*----------------------------------------------------------------------------
NAME:    ias_pm_get_iterator
         ias_pm_get_iterator_at_detector

PURPOSE: Creates an iterator for a pixel mask.  The iterator accepts a mask
         to indicate which pixel masks should be included in the values
         returned.  For example, if the iterator should step through the
         spans that have the saturated pixel bit set, the mask passed in
         would include that bit.

         The distinction between the two routines is what the next detector
         member gets set to.  The normal case is to call ias_pm_get_iterator
         which sets the next detector to zero (first detector).

RETURNS: Pointer to iterator
         NULL on error

-----------------------------------------------------------------------------*/
#include <stdlib.h>
#include "pm_local.h"
#include "ias_logging.h"


IAS_PIXEL_MASK_ITERATOR *ias_pm_get_iterator
(
    IAS_PIXEL_MASK *pixel_mask_ptr,/* I: Pointer to a pixel mask structure */
    IAS_PIXEL_MASK_ITERATOR_TYPE iterator_type, /* I: Type of iterator to
                                                      create */
    PIXEL_MASK_TYPE mask_set       /* I: Pixel mask set */
)
{
    return ias_pm_get_iterator_at_detector( pixel_mask_ptr, iterator_type,
        mask_set, 0 );
}  /* END ias_pm_get_iterator */

IAS_PIXEL_MASK_ITERATOR *ias_pm_get_iterator_at_detector
(
    IAS_PIXEL_MASK *pixel_mask_ptr,/* I: Pointer to a pixel mask structure */
    IAS_PIXEL_MASK_ITERATOR_TYPE iterator_type, /* I: Type of iterator to
                                                      create */
    PIXEL_MASK_TYPE mask_set,      /* I: Pixel mask set */
    int detector                   /* I: Detector (0-based) to set the iterator
                                      next detector value */
)
{
    struct IAS_PIXEL_MASK_ITERATOR *iterator; /* Internal iterator */
    
    /* Make sure the detector isn't out of range */
    if ( detector >= pixel_mask_ptr->num_of_detectors || detector < 0 )
    {
        IAS_LOG_ERROR("Attempting to set detector %d which is out of the "
            "valid range: 0 to %d", detector,
            pixel_mask_ptr->num_of_detectors );
        return NULL;
    }

    /* Allocate the iterator */
    iterator = malloc( sizeof( *iterator ));
    if ( !iterator )
    {
        IAS_LOG_ERROR("Allocating pixel mask iterator");
        return NULL;
    }

    iterator->pixel_mask = pixel_mask_ptr;
    iterator->next_detector = detector;
    iterator->next_span_starting_pixel_index = 0;
    iterator->curr_node = NULL;
    iterator->iterator_type = iterator_type;
    iterator->mask_set = mask_set;
    iterator->complemented_mask_set = ~mask_set;

    return (IAS_PIXEL_MASK_ITERATOR *)iterator;

}  /* END ias_pm_get_iterator_at_detector */
