/******************************************************************************
NAME:        ias_gcp_write_image

PURPOSE:    Write the specified image chip

RETURN VALUE: 
Type = int
Value           Description
-----           -----------
SUCCESS         Successfully wrote chip
ERROR           Error; chip not written

NOTES:          The image chip is always assumed to be a flat binary file 
                containing chip_size[0] (lines) x chip_size[1] (samples)
                with data type chip_data_type.
                Supports all IAS_DATA_TYPE's except IAS_I4, IAS_UI4, IAS_I8.
                This is because the chip image data is ultimately stored
                in memory in a 32-bit floating point buffer type, so that
                prevents us from being able to support those noted types.

******************************************************************************/
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "config.h"
#include "ias_gcp.h"
#include "local_defines.h"          /* byte-swapping macro routines */

int ias_gcp_write_image 
(
    const char *chip_name,      /* I: Name of the image chip file */
    int chip_size_lines,        /* I: Number of chip lines in data buffer */
    int chip_size_samples,      /* I: Number of chip samples in data buffer */
    IAS_DATA_TYPE dtype,        /* I: Data type for byte swapping if needed */
    const void *buf             /* I: Image chip data buffer to write to file */
)
{
    int fd;                         /* File descriptor */
    int bytes;
    int size = chip_size_lines * chip_size_samples;
    const void *local_buf;

    /* Calculate the number of bytes in the GCP chip data buffer */
    switch (dtype)
    {
        case IAS_BYTE: bytes = size * sizeof(IAS_BYTE_TYPE); break;
        case IAS_I2:   bytes = size * sizeof(IAS_I2_TYPE);   break;
        case IAS_UI2:  bytes = size * sizeof(IAS_UI2_TYPE);  break;
        case IAS_R4:   bytes = size * sizeof(IAS_R4_TYPE);   break;
        default:
            IAS_LOG_ERROR("Unsupported GCP chip data type %d", dtype);
            return ERROR;
    }

#ifndef HAVE_LITTLE_ENDIAN
    /* Current machine is big endian, so swap byte order before writing
       buffer to the file, to keep GCP chips on disk in little endian format */
    void *tmp_buf;
    int i;

    /* Allocate internal buffer for byte swapping */
    tmp_buf = malloc(bytes);
    if (tmp_buf == NULL)
    {
        IAS_LOG_ERROR("Allocating memory for endianness byte swapping");
        return ERROR;
    }

    if (dtype == IAS_BYTE)
    {
        /* byte swapping unnecessary for byte data */
        memcpy(tmp_buf, buf, bytes);
    }
    else if ((dtype == IAS_I2) || (dtype == IAS_UI2))
    {
        /* swap byte order in 16 bit data case */
        const IAS_UI2_TYPE *in_buf_16 = buf;
        IAS_UI2_TYPE *out_buf_16 = tmp_buf;
        int num_words = bytes / sizeof(IAS_UI2_TYPE);
        for (i = 0; i < num_words; i++)
        {
            IAS_UI2_TYPE data_value = in_buf_16[i];
            SWAP_BYTES_16(data_value);
            out_buf_16[i] = data_value;
        }
    }
    else if (dtype == IAS_R4)
    {
        /* swap byte order in 32 bit data case */
        const IAS_R4_TYPE *in_buf_32 = buf;
        IAS_R4_TYPE *out_buf_32 = tmp_buf;
        int num_words = bytes / sizeof(IAS_R4_TYPE);
        for (i = 0; i < num_words; i++)
        {
            /* Doing a bit different than 16 bit case, as using the local
               variable with the floating point causes a compile warning.
               Swapping directly in the buffer avoids a compile warning, so
               do this even though it's slightly less efficient, since this
               is such an unlikely case. */
            out_buf_32[i] = in_buf_32[i];
            SWAP_BYTES_32(out_buf_32[i]);
        }
    }
    else
    {
        IAS_LOG_ERROR("Unsupported GCP chip data type %d", dtype);
        return ERROR;
    }
    local_buf = tmp_buf;
#else
    local_buf = buf;
#endif

    /* Prepare the output file */
    fd = creat(chip_name, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0)
    {
        const char *err_str = strerror(errno);
        IAS_LOG_ERROR("Creat encounted error: %s", err_str);
        IAS_LOG_ERROR("Opening output GCP chip for writing: %s", chip_name);
        return ERROR;
    }

    /* Write out output buffer */
    if (write(fd, local_buf, bytes) != bytes)
    {
        IAS_LOG_ERROR("Writing output GCP chip data to %s", chip_name);
        close(fd);
        return ERROR;
    }

    /* Close the file */
    if (close(fd) < 0)
    {
        IAS_LOG_ERROR("Closing output GCP chip %s", chip_name);
        return ERROR;
    }

#ifndef HAVE_LITTLE_ENDIAN
    free(tmp_buf);
    local_buf = NULL;
#endif

    return SUCCESS;
}
