/*************************************************************************
 NAME:        ias_pm_get_sca_number

 PURPOSE:     Accessor function to return the sca number for a pixel mask

 RETURNS:     Integer representing (1-based) sca number for the pixel mask
              ERROR (-1), if an error occurs

*************************************************************************/
#include "ias_logging.h"
#include "pm_local.h"
#include "ias_const.h"
int ias_pm_get_sca_number
(
    IAS_PIXEL_MASK *pm      /* I: Pixel mask */
)
{
    if (pm == NULL)
    {
        IAS_LOG_ERROR("Pixel mask pointer is null");
        return ERROR;
    }
    return pm->sca;
}
