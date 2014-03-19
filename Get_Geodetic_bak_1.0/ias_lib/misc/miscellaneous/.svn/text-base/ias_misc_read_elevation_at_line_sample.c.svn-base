/*******************************************************************************
NAME:    ias_misc_read_elevation_at_line_sample   

PURPOSE: 
        Get the elevation associated with point (oline,osamp). DEM is assumed
        to be coregisted and a multiple pixel size to calling function. 

NOTES:
- A DEM to the output image is assumed (need not be same pixel size/resolution)
- The line/sample parameters passed cannot be negative
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ias_logging.h"
#include "ias_const.h"      /* BandNumber, SUCCESS/ERROR definitions */
#include "ias_l1g.h"
#include "ias_miscellaneous.h"

/* Note that when calling ROUND you should cast the result to an integer type */
#define POSITIVE_ROUND(x) ((x)+0.5)

int ias_misc_read_elevation_at_line_sample
(
    const IAS_IMAGE *dem,       /* I: DEM image */
    double line,                /* I: Line in input space (0-rel) */
    double samp,                /* I: Sample in input space (0-rel) */
    double pixel_size_y,        /* I: Line in output space (0-rel) */
    double pixel_size_x,        /* I: Sample in output space (0-rel) */
    double *elevation           /* O: Terrain table & related info */
)
{
    int oline;              /* Integral output space line (0-rel) */
    int osamp;              /* Integral output space sample (0-rel) */
    int dem_line;           /* DEM line corresponding to output line */
    int dem_samp;           /* DEM samp corresponding to output line */
    int int_midsamp;        /* Truncated between-pixel DEM line */
    int int_midline;        /* Truncated between-pixel DEM sample */
    double midline;         /* Between-pixel DEM line */
    double midsamp;         /* Between-pixel DEM sample */
    double new_z, t, u;     /* Bilinear interpolation variables */
    double subsamp_factor_line;
    double subsamp_factor_samp;
    short *dem_data_ptr;

    /* Verify the calling routine is not passing an illegal output line/sample
       pair (negative). */
    if (line < 0.0 || samp < 0.0)
    {
        IAS_LOG_ERROR("Illegal line/sample pair");
        return ERROR;
    }

    /* Elevation index. Calculate the elevation for this point (assumes a
       co-registered DEM) */
    subsamp_factor_line = pixel_size_y / dem->pixel_size_y;
    subsamp_factor_samp = pixel_size_x / dem->pixel_size_x;

    if ((pixel_size_x == dem->pixel_size_x)
            && (pixel_size_y == dem->pixel_size_y))
    {
        /* Convert input lines/samples to integers to use as array indices */
        oline = (int)POSITIVE_ROUND(line);
        osamp = (int)POSITIVE_ROUND(samp);

        if (oline >= dem->nl) oline = dem->nl - 1;
        if (osamp >= dem->ns) osamp = dem->ns - 1;

        /* Directly use the DEM; it matches the output size */
        *elevation = dem->data[oline * dem->ns + osamp];
    }
    else if ((subsamp_factor_line < 1.0) && (subsamp_factor_samp < 1.0))
    {
        /* Output image pixel size smaller than DEM means use */
        /* Bilinear interpolation to calculate an "in-between" DEM value */

        /* Actual line, sample in DEM to use */
        midline = subsamp_factor_line * line;
        midsamp = subsamp_factor_samp * samp;

        /* Nearest line, sample in DEM to interp between */
        int_midline = (int)midline;
        if (int_midline > dem->nl-2) 
            int_midline = dem->nl-2;

        int_midsamp = (int)midsamp;
        if (int_midsamp > dem->ns-2) 
            int_midsamp = dem->ns-2;

        /* Elevations at the posts */
        dem_data_ptr = &dem->data[int_midline * dem->ns + int_midsamp];

        /* Perform the bilinear interpolation */
        t = (midsamp - (double)int_midsamp);
        u = (midline - (double)int_midline);

        new_z = (1.0 - t) * (1.0 - u) * dem_data_ptr[0]
            + t * (1.0 - u) * dem_data_ptr[1]
            + (1.0 - t) * u * dem_data_ptr[dem->ns]
            + t * u * dem_data_ptr[dem->ns + 1];

        *elevation = new_z;
    }
    else if ((subsamp_factor_line > 1.0) && (subsamp_factor_samp > 1.0))
    {
        /* Output image pixel size larger than DEM means take */
        /* Every n'th DEM pixel for every output image pixel */
        dem_line = (int)POSITIVE_ROUND(subsamp_factor_line * line);
        dem_samp = (int)POSITIVE_ROUND(subsamp_factor_line * samp);

        if (dem_line >= dem->nl) 
            dem_line = dem->nl-1;
        if (dem_samp >= dem->ns) 
            dem_samp = dem->ns-1;

        *elevation = dem->data[dem_line * dem->ns + dem_samp];
    }
    else
    {
        IAS_LOG_ERROR("Mixed pixel sub-sampling in read_elevation");
        return ERROR;
    }

    return SUCCESS;
}
