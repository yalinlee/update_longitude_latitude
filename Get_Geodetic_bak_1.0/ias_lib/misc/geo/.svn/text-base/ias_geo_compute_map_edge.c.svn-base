/****************************************************************************
NAME: ias_geo_compute_map_edge

PURPOSE:  Calculates the minimum and maximum projection coordinates for
          given upper left and lower right latitude, longitude coordinates.

RETURN VALUE:   Type = int
Value    Description
-----    -----------
SUCCESS  Successful completion
ERROR    Operation failed

ALGORITHM DESCRIPTION:
        Initialize variables
        Calculate min/max coordinates along east edge of output area
        Calculate min/max coordinates along west edge of output area
        Calculate min/max coordinates along south edge of output area
        Calculate min/max coordinates along north edge of output area
        Return

*****************************************************************************/
#include "gctp.h"          /* DEGREE and DMS defines */
#include "ias_logging.h"
#include "ias_geo.h"

int ias_geo_compute_map_edge
(
    const IAS_GEO_PROJ_TRANSFORMATION *projection_transformation,
                      /* I: geographic degrees to projection transformation */
    double ul_lon,    /* I: Upper left longitude coordinate (in degrees) */
    double lr_lon,    /* I: Lower right longitude coordinate (in degrees) */
    double ul_lat,    /* I: Upper left latitude coordinate (in degrees) */
    double lr_lat,    /* I: Lower right latitude coordinate (in degrees) */
    double *pxmin,    /* O: Projection minimum in X */
    double *pxmax,    /* O: Projection maximum in X */
    double *pymin,    /* O: Projection minimum in Y */
    double *pymax     /* O: Projection maximum in Y */
)
{
    double delta_lon;   /* Longitude increment */
    double delta_lat;   /* Latitude increment */
    double inx, iny;    /* Input space coordinates given to proj() */
    double outx, outy;  /* Output space coordinates given to proj() */
    int  i;             /* Loop counter */
    int  size = 75;     /* Interval of increment to check for min/max */

    /* Initialization of variables.  */
    delta_lon = (lr_lon - ul_lon) / (size - 1);
    delta_lat = (lr_lat - ul_lat) / (size - 1);

    *pxmin = 1000000000.0;
    *pymin = 1000000000.0;
    *pxmax = -1000000000.0;
    *pymax = -1000000000.0;

    /* Calculate the minimum and maximum coordinates along the east side of 
       the output area */
    for (i = 0; i < size; i++)
    {
        inx = lr_lon;
        iny = lr_lat - (delta_lat * i);
        if (ias_geo_transform_coordinate(projection_transformation, 
            inx, iny, &outx, &outy) != SUCCESS)
        {
            IAS_LOG_ERROR("Failed to convert right edge to projection "
                          "coordinates");
            return ERROR;
        }

        if (outx < *pxmin)
            *pxmin = outx;
        if (outx > *pxmax)
            *pxmax = outx;
        if (outy < *pymin)
            *pymin = outy;
        if (outy > *pymax)
            *pymax = outy;
    }

    /* Calculate the minimum and maximum coordinates along the west side of 
       the output area */
    for (i = 0; i < size; i++)
    {
        inx = lr_lon - (delta_lon * (size - 1));
        iny = lr_lat - (delta_lat * i);
        if (ias_geo_transform_coordinate(projection_transformation,
            inx, iny, &outx, &outy) != SUCCESS)
        {
            IAS_LOG_ERROR("Failed to convert left edge to projection "
                          "coordinates");
            return ERROR;
        }
     
        if (outx < *pxmin)
            *pxmin = outx;
        if (outx > *pxmax)
            *pxmax = outx;
        if (outy < *pymin)
            *pymin = outy;
        if (outy > *pymax)
            *pymax = outy;
    }


    /* Calculate the minimum and maximum coordinates along the south side of 
       the output area */
    for (i = 0; i < size; i++)
    {
        inx = lr_lon - (delta_lon * i);
        iny = lr_lat - (delta_lat * (size - 1));
        if (ias_geo_transform_coordinate(projection_transformation, 
            inx, iny, &outx, &outy) != SUCCESS)
        {
            IAS_LOG_ERROR("Failed to convert lower edge to projection "
                          "coordinates");
            return ERROR;
        }
     
        if (outx < *pxmin)
            *pxmin = outx;
        if (outx > *pxmax)
            *pxmax = outx;
        if (outy < *pymin)
            *pymin = outy;
        if (outy > *pymax)
            *pymax = outy;
    }


    /* Calculate the minimum and maximum coordinates along the north side of 
       the output area */
    for (i = 0; i < size; i++)
    {
        inx = lr_lon - (delta_lon * i);
        iny = lr_lat;
        if (ias_geo_transform_coordinate(projection_transformation, 
            inx, iny, &outx, &outy) != SUCCESS)
        {
            IAS_LOG_ERROR("Failed to convert upper edge to projection "
                          "coordinates");
            return ERROR;
        }
     
        if (outx < *pxmin)
            *pxmin = outx;
        if (outx > *pxmax)
            *pxmax = outx;
        if (outy < *pymin)
            *pymin = outy;
        if (outy > *pymax)
            *pymax = outy;
    }

    return SUCCESS;
}
