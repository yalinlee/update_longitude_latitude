/******************************************************************************
NAME:           ias_misc_read_dem

PURPOSE:        Read the digital elevation model and verify it is legal to
                use for the given source image 

RETURN VALUE: 
Type = SUCCESS/ERROR
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>    
#include "ias_const.h"
#include "ias_types.h"
#include "ias_logging.h"
#include "ias_miscellaneous.h"

int ias_misc_read_dem 
(
    const char *dem_name,                 /* I: name of DEM to read */
    const IAS_CORNERS *image_corners_ptr, /* I: Corners of image */
    int lines_in_image,                   /* I: Lines in image */
    int samples_per_image_line,           /* I: Samples in an image line */
    double image_pixsize,                 /* I: Input image pixel size */
    IAS_IMAGE *dem_ptr                    /* O: Populated from the L1G file */
)
{
    int status;                 /* Return status */
    double max_corner_diff;     /* Maximum allowed difference in the DEM and
                                   image corner points */
    int scaled_lines_in_image;  /* Number of lines in the image after the
                                   lines_in_image is scaled by the ratio of
                                   the image pixel size to DEM pixel size */
    int scaled_samps_in_image;  /* Samples across the image after the
                                   samples_per_image_line is scaled by the ratio
                                   of the image pixel size to DEM pixel size */
    IAS_DATA_TYPE dem_datatype; /* Datatype of DEM as read from the L1G */

    status = ias_misc_read_single_band_l1g(dem_name, 1, &dem_datatype, dem_ptr);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Reading L1G DEM image file");
        return ERROR;
    }

    /* Set the maximum difference between the DEM and image corner points
       to one-half of the pixsize */
    max_corner_diff = dem_ptr->pixel_size_x * 0.5;
    
    /* Verify the DEM is valid */
    /* Make sure the corners of the image are within 0.5 pixels */
    if ((fabs(image_corners_ptr->upleft.y - dem_ptr->corners.upleft.y)
                > max_corner_diff)
            || (fabs(image_corners_ptr->upleft.x - dem_ptr->corners.upleft.x)
                > max_corner_diff)
            || (fabs(image_corners_ptr->upright.y - dem_ptr->corners.upright.y)
                > max_corner_diff)
            || (fabs(image_corners_ptr->upright.x - dem_ptr->corners.upright.x)
                > max_corner_diff)
            || (fabs(image_corners_ptr->loleft.y - dem_ptr->corners.loleft.y)
                > max_corner_diff)
            || (fabs(image_corners_ptr->loleft.x - dem_ptr->corners.loleft.x)
                > max_corner_diff)
            || (fabs(image_corners_ptr->loright.y - dem_ptr->corners.loright.y)
                > max_corner_diff)
            || (fabs(image_corners_ptr->loright.x - dem_ptr->corners.loright.x)
                > max_corner_diff)
       )
    {
        IAS_LOG_ERROR("DEM and image corners do not match");
        return ERROR;
    }

    /* Scale the number of lines and samples in the input image using this:
       lines_in_higher_resolution
         = pixel_size_lower_resolution / pixel_size_higher_resolution
             * lines_in_lower_resolution
             - ((pixel_size_lower_resolution - pixel_size_higher_resolution)
                 / pixel_size_higher_resolution)
    */
    scaled_lines_in_image = (int)(image_pixsize / dem_ptr->pixel_size_x
            * lines_in_image - ((image_pixsize - dem_ptr->pixel_size_x)
                / dem_ptr->pixel_size_x) + 0.5);
    scaled_samps_in_image = (int)(image_pixsize / dem_ptr->pixel_size_y
            * samples_per_image_line
            - ((image_pixsize - dem_ptr->pixel_size_y)
                / dem_ptr->pixel_size_y) + 0.5);

    /* Make sure the number of lines and sample agree between the DEM and the 
       image */
    if ((dem_ptr->ns != scaled_samps_in_image )
            || (dem_ptr->nl != scaled_lines_in_image))
    {
        IAS_LOG_ERROR("DEM and image sizes do not match. DEM has %d, %d. "
            "Expected %d, %d (line, sample)", dem_ptr->nl, dem_ptr->ns,
            scaled_lines_in_image, scaled_samps_in_image);
        return ERROR;
    }

    /* Verify data type is short */
    if (dem_datatype != IAS_I2)
    {
        IAS_LOG_ERROR("DEM does not contain short integer data");
        return ERROR;        
    }

    return SUCCESS;
}
