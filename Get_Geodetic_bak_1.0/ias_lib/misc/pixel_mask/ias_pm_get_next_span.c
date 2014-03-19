/*----------------------------------------------------------------------------
NAME:    ias_pm_get_next_span

PURPOSE: Return the next span available in the pixel mask for the given
         type and mask_set.

RETURNS: 1 for true if a span is returned
         0 for false no span returned because end of iterator was reached

-----------------------------------------------------------------------------*/
#include "ias_const.h"      /* TRUE/FALSE definition */
#include "pm_local.h"
#include "ias_logging.h"


int ias_pm_get_next_span
(   
    IAS_PIXEL_MASK_ITERATOR *iterator, /* I: Pointer to the iterator */
    IAS_PIXEL_MASK_SPAN *span          /* I/O: Span structure */
)
{
    IAS_PIXEL_MASK_SPAN *it_span; /* span contained in the iterator */
    struct IAS_PIXEL_MASK *pixel_mask; /* convenience variable for mask */
    IAS_LINKED_LIST_NODE *base_node = NULL; /* base node of a linked list */

    /* These are local copies of the values to return in the span */
    PIXEL_MASK_TYPE mask_to_return;
    int detector_to_return = 0;
    int pixel_to_return = 0;
    int length_to_return = 0;

    int condition_not_met = TRUE; /* while loop control variable */

    /* Get the iterator span and pixel mask */
    pixel_mask = iterator->pixel_mask;
    if ( iterator->curr_node != NULL )
    {
        it_span = GET_OBJECT_OF( iterator->curr_node, IAS_PIXEL_MASK_SPAN,
            node );
        base_node = pixel_mask->detector_lut[iterator->next_detector];
    }
    else
    {
        it_span = NULL;
    }

    /* Loop until a mask is found that matches the correct contents
    for the iterator type. */
    do
    {
        mask_to_return = PM_NOVALUE;

        /* First stopping condition is: the detector number is greater than
        number of detectors in the image.  Detectors are considered to be
        zero-based for the pixel mask. */
        if ( iterator->next_detector >= pixel_mask->num_of_detectors )
            return 0;

        /* Set the detector to return to the iterator current detector */
        detector_to_return = iterator->next_detector;

        /* If the iterator current pixel is 0, set the current node to the
        initial node from the detector lookup table for the current detctor */
        if ( iterator->next_span_starting_pixel_index == 0 )
        {
            base_node = pixel_mask->detector_lut[iterator->next_detector];
            if ( base_node == NULL )
            {
                iterator->curr_node = NULL;
            }
            else
            {
                iterator->curr_node =
                    ias_linked_list_get_head_node( base_node );
                it_span = GET_OBJECT_OF( iterator->curr_node,
                    IAS_PIXEL_MASK_SPAN, node );
            }
        }

        /* Set the pixel to return to the iterator current pixel */
        pixel_to_return = iterator->next_span_starting_pixel_index;

        if ( iterator->curr_node == NULL )
        {
            length_to_return = pixel_mask->num_of_pixels -
                iterator->next_span_starting_pixel_index;
        }
        else
        {
            if ( iterator->next_span_starting_pixel_index <
                 it_span->starting_pixel_index )
            {
                length_to_return = it_span->starting_pixel_index -
                    iterator->next_span_starting_pixel_index;
            }
            else
            {
                /* Error check make sure the iterator current node start
                pixel is equal to the current pixel. */
                iterator->next_span_starting_pixel_index =
                    it_span->starting_pixel_index;

                length_to_return = it_span->length_of_span;
                mask_to_return= it_span->pixel_mask;
                iterator->curr_node = ias_linked_list_get_next_node( base_node,
                    iterator->curr_node );
                it_span = GET_OBJECT_OF( iterator->curr_node,
                    IAS_PIXEL_MASK_SPAN, node );
            }
        }  /* END else of if iterator->curr_node == NULL */

        /* Adjust the span starting pixel by adding the current length */
        iterator->next_span_starting_pixel_index += length_to_return;

        if ( iterator->next_span_starting_pixel_index >=
            pixel_mask->num_of_pixels )
        {
            iterator->next_span_starting_pixel_index = 0;
            iterator->next_detector++;
            /* Error check, make sure current node is null */
            iterator->curr_node = NULL;
        }

        /* Logic to check the other stopping conditions */
        switch ( iterator->iterator_type )
        {
            case IAS_PM_ALL :
                /* We return all spans for this type */
                condition_not_met = FALSE;
                break;

            case IAS_PM_INCLUDE :
                if ( mask_to_return & iterator->mask_set )
                    condition_not_met = FALSE;
                break;

            case IAS_PM_EXCLUDE :
                if ( mask_to_return & iterator->complemented_mask_set )
                    condition_not_met = FALSE;
                break;

            default :
                /* This should never happen */
                IAS_LOG_ERROR("Impossibility has occurred");
                return 0;
        }  /* END switch */

    } while ( condition_not_met );

    /* Set the return span values */
    span->node.prev = span->node.next = NULL;
    span->detector_index = detector_to_return;
    span->starting_pixel_index = pixel_to_return;
    span->length_of_span = length_to_return;
    span->pixel_mask = mask_to_return;
    return 1;

}  /* END ias_pm_get_next_span */
