/******************************************************************************
NAME:           ias_misc_read_single_band_l1g

PURPOSE:        Read the specified band of the L1G image

RETURN VALUE: 
Type = SUCCESS/ERROR

NOTES:          This routine allocates memory for the image data and returns
                a pointer to the data. It also passes back the size (nl, ns)
                and data type of the data that was read.
                SCA-separated images are not supported by this routine.
                The caller is responsible for freeing this memory.
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
#include "ias_l1g.h"
#include "ias_miscellaneous.h"

int ias_misc_read_single_band_l1g 
(
    const char *l1g_filename,    /* I: Name of the L1G image file */
    int band_number,             /* I: Band number for band to read */
    IAS_DATA_TYPE *data_type,    /* O: Data type of the read out band */
    IAS_IMAGE *image_ptr         /* O: Populated from the L1G image file */
)
{
    L1GIO *l1g_file;             /* File IO ID for the L1G image */
    L1G_BAND_IO *l1g_band;       /* L1G IO band structure */
    int status;                  /* Function return status */
    int size;                    /* Size of data (1 pixel) */
    int number_of_scas;          /* Number of SCAs */
    int lines;
    int samples;
    int num_scas;
    IAS_L1G_BAND_METADATA band_metadata;

    /* Open the L1G image */
    l1g_file = ias_l1g_open_image(l1g_filename, IAS_READ);
    if (l1g_file == NULL)
    {
        IAS_LOG_ERROR("Accessing L1G image %s", l1g_filename);
        return ERROR;
    }

    /* Checking the band is present */
    status = ias_l1g_is_band_present(l1g_file, band_number);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("The band is not present in the image %s", l1g_filename);
        ias_l1g_close_image(l1g_file);
        return ERROR;
    }

    status = ias_l1g_get_band_size(l1g_file, band_number, data_type,
            &number_of_scas, &lines, &samples);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Getting band size information");
        ias_l1g_close_image(l1g_file);
        return ERROR;
    }

    /* Set up the band data */
    l1g_band = ias_l1g_open_band(l1g_file, band_number, data_type, &num_scas, 
            &lines, &samples);
    if (l1g_band == NULL)
    {
        IAS_LOG_ERROR("Opening L1G image band for information");
        ias_l1g_close_image(l1g_file);
        return ERROR;
    }
    if (num_scas != 1)
    {
        IAS_LOG_ERROR("SCA-separated image not supported");
        ias_l1g_close_image(l1g_file);
        return ERROR;
    }

    switch (*data_type)
    {
        case IAS_BYTE:
            size = sizeof(uint8_t);
            break;
        case IAS_I2:
            size = sizeof(int16_t);
            break;
        case IAS_UI2:
            size = sizeof(uint16_t);
            break;
        case IAS_I4:
            size = sizeof(int32_t);
            break;
        case IAS_UI4:
            size = sizeof(uint32_t);
            break;
        case IAS_R4:
            size = sizeof(float);
            break;
        case IAS_R8:
            size = sizeof(double);
            break;
        default:
            {
                IAS_LOG_ERROR("L1G file %s contains invalid data type", 
                        l1g_filename);
                ias_l1g_close_band(l1g_band);
                ias_l1g_close_image(l1g_file);
                return ERROR;
            }
    }

    /* Read the band metadata */
    status = ias_l1g_get_band_metadata(l1g_file, band_number, &band_metadata);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Getting band metadata");
        ias_l1g_close_band(l1g_band);
        ias_l1g_close_image(l1g_file);
        return ERROR;
    }

    /* Copy the pixel size from the band metadata */
    image_ptr->pixel_size_x = band_metadata.projection_distance_x;
    image_ptr->pixel_size_y = band_metadata.projection_distance_y;

    /* Get the L1G size from the band metadata */
    image_ptr->ns = samples;
    image_ptr->nl = lines;

    /* Get the IAS_CORNERS values in IAS_IMAGE structure */
    image_ptr->corners.upleft.x = band_metadata.upper_left_x;
    image_ptr->corners.upleft.y = band_metadata.upper_left_y;
    image_ptr->corners.upright.x = band_metadata.upper_right_x;
    image_ptr->corners.upright.y = band_metadata.upper_right_y;
    image_ptr->corners.loleft.x = band_metadata.lower_left_x;
    image_ptr->corners.loleft.y = band_metadata.lower_left_y;
    image_ptr->corners.loright.x = band_metadata.lower_right_x;
    image_ptr->corners.loright.y = band_metadata.lower_right_y;

    /* Allocate space for the image data */
    image_ptr->data = malloc(lines * samples * size);
    if (image_ptr->data == NULL)
    {
        IAS_LOG_ERROR("Allocating memory for the data");
        ias_l1g_close_band(l1g_band);
        ias_l1g_close_image(l1g_file);
        return ERROR;
    }

    /* Read the full L1G band into the buffer */
    status = ias_l1g_read_image(l1g_band, 0, 0, 0, lines, samples,
            image_ptr->data);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Reading L1G image %s", l1g_filename);
        free(image_ptr->data);
        ias_l1g_close_band(l1g_band);
        ias_l1g_close_image(l1g_file);
        return ERROR;
    }

    /* Close the band */
    status = ias_l1g_close_band(l1g_band);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Closing the band %s", l1g_filename);
        free(image_ptr->data);
        return ERROR;
    }

    /* Close the L1G */
    status = ias_l1g_close_image(l1g_file);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Closing L1G image %s", l1g_filename);
        free(image_ptr->data);
        return ERROR;
    }

    return SUCCESS;
}
