/*----------------------------------------------------------------------------
NAME:    ias_pm_create

PURPOSE: Create a new pixel mask.

RETURNS: Pointer to pixel mask or NULL on error

-----------------------------------------------------------------------------*/
#include <stdlib.h>
#include "pm_local.h"
#include "ias_logging.h"


IAS_PIXEL_MASK *ias_pm_create
(
    int band,                   /* I: Band number               */
    int sca,                    /* I: SCA number                */
    int num_of_detectors,       /* I: Number of detectors       */
    int num_of_pixels           /* I: Number of pixels          */
)
{
    struct IAS_PIXEL_MASK *pixel_mask_ptr = NULL; /* pointer to pixel mask */

    /* Allocate the pixel mask */
    pixel_mask_ptr = malloc( sizeof( *pixel_mask_ptr ));
    if ( pixel_mask_ptr == NULL )
    {
        IAS_LOG_ERROR("Allocating internal pixel mask");
        return NULL;
    }

    /* Set the structure entries */
    pixel_mask_ptr->band = band;
    pixel_mask_ptr->sca = sca;
    pixel_mask_ptr->num_of_detectors = num_of_detectors;
    pixel_mask_ptr->num_of_pixels = num_of_pixels;

    /* Initialize the detector look up table */
    pixel_mask_ptr->detector_lut = calloc( num_of_detectors, 
        sizeof( IAS_LINKED_LIST_NODE ));
    if ( pixel_mask_ptr->detector_lut == NULL )
    {
        IAS_LOG_ERROR("Allocating the detector look up table");
        free( pixel_mask_ptr );
        return NULL;
    }

    return (IAS_PIXEL_MASK *)pixel_mask_ptr;
} /* END ias_pm_create */
