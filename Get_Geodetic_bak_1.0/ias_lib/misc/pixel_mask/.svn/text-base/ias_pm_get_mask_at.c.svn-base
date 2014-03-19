/*----------------------------------------------------------------------------
NAME:    ias_pm_get_mask_at

PURPOSE: Return the pixel mask at a specific location

RETURNS: SUCCESS or ERROR

-----------------------------------------------------------------------------*/
#include "pm_local.h"
#include "ias_pixel_mask.h"
#include "ias_const.h"
#include "ias_logging.h"


int ias_pm_get_mask_at
(
    IAS_PIXEL_MASK *pixel_struct,   /* I: Pointer to a pixel mask structure */
    int detector_index,             /* I: Detectory number */
    int pixel_index,                /* I: Location of pixel */
    PIXEL_MASK_TYPE *mask           /* O: pixel mask */
)
{
    IAS_LINKED_LIST_NODE *base; /* Base node of detector linked list of spans */
    IAS_PIXEL_MASK_SPAN *span; /* pointer to each span of the linked list */
    int span_start;  /* starting pixel of span */
    int span_end;    /* ending pixel of span */

    /* If the detector, pixel location is outside the covered area */
    if ( detector_index >= pixel_struct->num_of_detectors
       || pixel_index >= pixel_struct->num_of_pixels )
    {
        IAS_LOG_ERROR("Attempting to get a mask that is not inside the "
            "covered area: detector %d pixel %d", detector_index, pixel_index);
        return ERROR;
    }

    /* Set the return value to none */
    *mask = PM_NOVALUE;

    /* Get the base node, if any, of the detector linked list */
    base = pixel_struct->detector_lut[detector_index];
    if ( base == NULL ) 
        return SUCCESS; /* Returned mask value already set */

    /* Search the linked list for the span that includes the requested
    pixel. */
    GET_OBJECT_FOR_EACH_ENTRY( span, base, IAS_PIXEL_MASK_SPAN, node )
    {
        span_start = span->starting_pixel_index;
        span_end = span_start + span->length_of_span - 1;

        if ( pixel_index >= span_start && pixel_index <= span_end )
        {
            *mask = span->pixel_mask;
            return SUCCESS;
        }
    }  /* END GET_OBJECT_FOR_EACH_ENTRY */

    return SUCCESS;
}  /* END ias_pm_get_mask_at */
