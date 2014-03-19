/**************************************************************************
 NAME:                  ias_pm_is_detector_flagged

 PURPOSE:   Checks the detector LUT in a pixel mask to see if the
            specified detector has spans(s) including the specified mask
            artifact(s)

 RETURNS:   1 for true, if at least one span was found with the given
            artifact(s)
            0 for false, if:
              - no span was found with the given artifact(s), or
              - the detector LUT has no spans for the detector (iow the
                base pointer is NULL)

 NOTES:     If a "cursory" survey of the pixel mask is desired, i.e.
            just see if anything was flagged rather than checking to see
            if specific artifact(s) were flagged, set the input mask
            to include all known artifacts

***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ias_linked_list.h"
#include "ias_pixel_mask.h"
#include "pm_local.h"
#include "ias_logging.h"

int ias_pm_is_detector_flagged
(
    IAS_PIXEL_MASK  *pm,        /* I: Pixel mask for the current
                                   band/SCA */
    int   detector_index,       /* I: (0-based) index for current
                                   detector */
    PIXEL_MASK_TYPE mask        /* I: Mask of artifacts to look for
                                   (individual or composite)  */
)
{
    IAS_LINKED_LIST_NODE     *base = NULL;       /* Base node for detector's
                                                    linked list of spans */
    IAS_PIXEL_MASK_SPAN      *span = NULL;       /* Current span */

    PIXEL_MASK_TYPE          full_mask;          /* Pixel mask with all
                                                    known artifacts */


    /* Set the "found" flag to be false (not found) */
    int is_in_mask = 0;

    /* Create a "full" mask with all known artifacts */
    full_mask = ~PM_NOVALUE;

    /* Get the base node, if any, of the detector linked list */
    base      = pm->detector_lut[detector_index];

    if (base != NULL)  /* Something's been flagged--now we have to
                          check in further detail */
    {
        /* Get each node */
        GET_OBJECT_FOR_EACH_ENTRY(span, base, IAS_PIXEL_MASK_SPAN,
                                  node)
        {
            /* Test to see if the masked artifact(s) are part of the
               artifact(s) in the span.  If they are, for the specified
               detector, set is_in_mask to true and stop.

               If we don't care what specifically is flagged, the
               input mask should be a bit-wise OR of all known
               artifacts.  If it isn't, and the specified artifact(s)
               are not in any of the spans, the routine will return
               false  */
            if (mask & span->pixel_mask)
            {
                is_in_mask = 1;
                break;
            }
        }

        /* Something was flagged, but not what was specified in the input
           mask.  Issue a warning--the user might be interested to know...*/
        if ((is_in_mask == 0) && (mask != full_mask))
        {
            IAS_LOG_DEBUG("Detector %d has flagged artifact(s) not "
                            "found with current input mask",
                            (detector_index + 1));
        }
    }

    /* If base is NULL, the detector was not flagged for anything */

    /* Return is_in_mask flag */
    return is_in_mask;
}
