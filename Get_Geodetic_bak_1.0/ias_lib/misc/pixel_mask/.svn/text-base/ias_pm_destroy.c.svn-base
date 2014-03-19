/*----------------------------------------------------------------------------
NAME:    ias_pm_destroy

PURPOSE: Free the resources allocated to a pixel mask

RETURNS: nothing

-----------------------------------------------------------------------------*/
#include <stdlib.h>
#include "pm_local.h"


void ias_pm_destroy
(
    IAS_PIXEL_MASK *pixel_mask_ptr    /* I: Pointer to pixel mask */
)
{
    int i;  /* loop counter */

    /* Free any allocated detector LUT nodes */
    for ( i = 0; i < pixel_mask_ptr->num_of_detectors; i++ )
    {
        if ( pixel_mask_ptr->detector_lut[i] != NULL )
        {
            ias_linked_list_delete( pixel_mask_ptr->detector_lut[i],
                offsetof( IAS_PIXEL_MASK_SPAN, node ));
            free( pixel_mask_ptr->detector_lut[i] );
            pixel_mask_ptr->detector_lut[i] = NULL;
        }
    }

    /* Free the remainder of the memory */
    free( pixel_mask_ptr->detector_lut );
    free( pixel_mask_ptr );

}  /* END ias_pm_destroy */
