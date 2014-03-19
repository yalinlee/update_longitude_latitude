/*************************************************************************
 NAME:        ias_pm_get_number_of_pixels

 PURPOSE:     Accessor function to return the number of pixels for a
              pixel mask

 RETURNS:     Integer representing number of pixels for the pixel mask
              ERROR, if an error occurs

*************************************************************************/
#include "ias_logging.h"
#include "pm_local.h"
#include "ias_const.h"

int ias_pm_get_number_of_pixels
(
    IAS_PIXEL_MASK *pm      /* I: Pixel mask */
)
{
    if (pm == NULL)
    {
        IAS_LOG_ERROR("Pixel mask pointer is null");
        return ERROR;
    }
    return pm->num_of_pixels;
}
