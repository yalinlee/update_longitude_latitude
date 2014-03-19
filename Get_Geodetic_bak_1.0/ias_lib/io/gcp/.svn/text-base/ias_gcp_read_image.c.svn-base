/******************************************************************************
NAME:        ias_gcp_read_image

PURPOSE:    Read the specified image chip

RETURN VALUE: 
Type = int
Value           Description
-----           -----------
SUCCESS         Successfully extracted chip
ERROR           Error; chip not extracted

NOTES:          The image chip is always assumed to be a flat binary file 
                containing chip_size[0] (lines) x chip_size[1] (samples)
                with data type chip_data_type.
                Supports all IAS_DATA_TYPE's except IAS_I4, IAS_UI4, IAS_I8.
                This is because the chip image data is ultimately stored
                in memory in a 32-bit floating point buffer type, so that
                prevents us from being able to support those noted types.

******************************************************************************/
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "config.h"
#include "ias_gcp.h"
#include "local_defines.h"          /* byte-swapping macro routines */

int ias_gcp_read_image 
(
    const char *chip_name,          /* I: Name of the image chip file */
    /* Chip lines and samples are doubles because that's how
       they're defined in the GCP Lib file */
    double chip_size_lines,         /* I: Expected number of chip lines */
    double chip_size_samples,       /* I: Expected number of chip samples */
    IAS_DATA_TYPE chip_data_type,   /* I: Image chip data type */
    IAS_R4_TYPE *chip_buffer        /* O: Buffer of chip image data converted
                                          to float (R*4) */
)
{
    FILE *chip_fp;                  /* Chip file pointer */
    int filedes;                    /* Integer file descriptor for chip_fp */
    struct stat file_stat;          /* Fstat file status structure */
    void           *max_buf;        /* Max sized buffer pointer */
    int data_size;                  /* The size of the chip data */
    int i;
    int chip_int_lines = (int)chip_size_lines;
    int chip_int_samples = (int)chip_size_samples;
    int chip_total_samples = chip_int_lines * chip_int_samples;

    /* Get the size of the data */
    switch(chip_data_type)
    {
        case IAS_BYTE:
            data_size = sizeof(IAS_BYTE_TYPE);
            break;
        case IAS_I2:
            data_size = sizeof(IAS_I2_TYPE);
            break;
        case IAS_UI2:
            data_size = sizeof(IAS_UI2_TYPE);
            break;
        case IAS_R4:
            data_size = sizeof(IAS_R4_TYPE);
            break;
        default:
            IAS_LOG_ERROR("Unsupported GCP chip data type %d", chip_data_type);
            return ERROR;
    }

    /* Allocate the max size (R*4) data buffer */
    max_buf = malloc(chip_total_samples * sizeof(IAS_R4_TYPE));
    if (max_buf == NULL)
    {
        IAS_LOG_ERROR("Allocating memory for reading chip");
        return ERROR;
    }

    /* Open the chip image */
    chip_fp = fopen(chip_name, "r");
    if (chip_fp == NULL)
    {
        free(max_buf);
        IAS_LOG_ERROR("Accessing image chip %s", chip_name);
        return ERROR;
    }

    /* Check to make sure the chip size is the same as reported by the chip */
    filedes = fileno(chip_fp);
    if (filedes == -1)
    {
        free(max_buf);
        fclose(chip_fp);
        IAS_LOG_ERROR("Getting descriptor for image chip file stream");
        return ERROR;
    }
    if (fstat(filedes, &file_stat) < 0)
    {
        free(max_buf);
        fclose(chip_fp);
        IAS_LOG_ERROR("Getting image chip file status");
        return ERROR;
    }
    if (file_stat.st_size != (chip_total_samples * data_size))
    {
        free(max_buf);
        fclose(chip_fp);
        IAS_LOG_ERROR("Image chip was %ld bytes, expected %d bytes",
                file_stat.st_size, data_size);
        return ERROR;
    }

    /* Read the full chip into max size buffer */
    if (fread(max_buf, data_size, chip_total_samples, chip_fp)
            != (chip_total_samples))
    {
        free(max_buf);
        fclose(chip_fp);
        IAS_LOG_ERROR("Reading image chip %s", chip_name);
        return ERROR;
    }

    /* Convert to float for output buffer */
    if (chip_data_type == IAS_BYTE)
    {
        /* Create a byte data type buffer pointer to point at the
           max sized data buffer */
        IAS_BYTE_TYPE *buf_byte = max_buf;

        for (i = 0; i < (chip_total_samples); i++)
            chip_buffer[i] = buf_byte[i];
    }
    else if (chip_data_type == IAS_I2)
    {
        /* Create a signed 16-bit int data type buffer pointer to point
           at the max sized data buffer */
        IAS_I2_TYPE *buf_i2 = max_buf;

        for (i = 0; i < (chip_total_samples); i++)
        {
            IAS_I2_TYPE data_value = buf_i2[i];
            /* Swap byte order of the buffer since GCP chips on disk are
               in little endian format, in case on big endian system */
            SWAP_BYTES_16(data_value);
            chip_buffer[i] = data_value;
        }
    }
    else if (chip_data_type == IAS_UI2)
    {
        /* Create an unsigned 16-bit int data type buffer pointer to point
           at the max sized data buffer */
        IAS_UI2_TYPE *buf_ui2 = max_buf;

        for (i = 0; i < (chip_total_samples); i++)
        {
            IAS_UI2_TYPE data_value = buf_ui2[i];
            /* Swap byte order of the buffer since GCP chips on disk are
               in little endian format, in case on big endian system */
            SWAP_BYTES_16(data_value);
            chip_buffer[i] = data_value;
        }
    }
    else /* (chip_data_type == IAS_R4) */
    {
        /* Create a 4-byte floating point data type buffer pointer to point
           at the max sized data buffer */
        IAS_R4_TYPE *buf_r4 = max_buf;

        for (i = 0; i < (chip_total_samples); i++)
        {
            /* Swap byte order of the buffer since GCP chips on disk are
               in little endian format, in case on big endian system */
            /* Doing a bit different than 16 bit cases, as using the local
               variable with the floating point causes a compile warning.
               Swapping directly in the buffer avoids a compile warning, so
               do this even though it's slightly less efficient, since this
               is such an unlikely case. */
            chip_buffer[i] = buf_r4[i];
            SWAP_BYTES_32(chip_buffer[i]);
        }
    }

    /* Close the chip */
    if (fclose(chip_fp) == EOF)
    {
        free(max_buf);
        IAS_LOG_ERROR("Closing image chip %s", chip_name);
        return ERROR;
    }

    free(max_buf);

    return SUCCESS;
}
