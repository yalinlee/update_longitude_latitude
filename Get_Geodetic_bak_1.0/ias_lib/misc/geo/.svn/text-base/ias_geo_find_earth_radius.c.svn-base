/*******************************************************************************
NAME: ias_geo_find_earth_radius

PURPOSE:
Finds the radius of the Earth at a specified latitude

RETURN VALUE:
Type = int (SUCCESS or ERROR)
Value           Description
-----           -----------
SUCCESS         Successful calculation of Earth radius
ERROR           Error calculating Earth radius (error in CPF eccentricity)

*******************************************************************************/
#include <math.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_geo.h"

int ias_geo_find_earth_radius
(
    double latc,       /* I: Lat to find the radius at (rad) */
    const IAS_EARTH_CHARACTERISTICS *earth, /* I: Earth constants */
    double *radius     /* O: Radius of Earth at given lat (m) */
)
{
    double tmp1;        /* temporary for checking calculation */
    double tmp2;        /* temporary for checking calculation */

    tmp1 = 1.0 - earth->eccentricity;
    tmp2 = 1.0 - earth->eccentricity * cos(latc) * cos(latc);
    if ((tmp1 < 0.0) || (tmp2 <= 0.0))
    {
        IAS_LOG_ERROR("Calculating Earth radius");
        return ERROR;
    }

    *radius = (earth->semi_major_axis * sqrt(tmp1)) / sqrt(tmp2);

    return SUCCESS;
}
