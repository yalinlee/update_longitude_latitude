/******************************************************************************
NAME: ias_geo_extract_window

PURPOSE: 
Extracts an image window around a specific GCP.
        
From the input image, a window of the specified size will be extracted around 
the GCP line and sample.  If the window is of odd size, the extra line and/or 
sample will be at the beginning of the imagery.  The data in the window 
representing portions outside the imagery will be filled with zeros.

There are 2 steps to the extraction:
 (1) data type conversion of whatever the 1G image is to float
 (2) setting the calculated window correctly into the buffer (even if the
     calculated window falls partially outside the image)
        
RETURN VALUE:
Type = int
Value Description
----- -----------
ERROR Error reading from the 1G image
SUCCESS Successfully read 1G window and placed into buffer
OUTSIDE GCP falls totally outside the image, no window extracted

NOTES:
--The calling function must allocate space for the output image buffer and 
  open the image file previous to calling this function.
--The predicted point may lie near the edge of the image, in which case the 
  window may fall out of the image.  Any window that falls out will have the 
  "outside" area zero-filled.

******************************************************************************/
#include <stdlib.h>                /* for malloc */
#include "ias_types.h"
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_l1g.h"
#include "ias_geo.h"               /* for OUTSIDE define */

int ias_geo_extract_window
(
    int image_nl,       /* I: # of lines in the 1G image */
    int image_ns,       /* I: # of samples in the 1G image */
    int sca,            /* I: SCA number to extract data */
    IAS_DATA_TYPE dtype,/* I: data type of the band  */
    double pred_line,   /* I: center line of window to extract */
    double pred_samp,   /* I: center sample of window to extract */
    const int win_size[2], /* I: size of window to extract */
    L1G_BAND_IO *l1g_band, /* I: L1G_BAND_IO info structure for the image */
    float *img_buf,     /* O: Buffer read from 1G image */
    int *upper_line,    /* O: The upper line in the image for the chip */
    int *left_sample    /* O: The left sample in the image for the chip */
)
{
    float *read_buf; /* buffer to read into, size of the actual window read */
    float *tmp_buf;  /* temporary pointer when copying into final buffer */
    void *img_buf_gen=NULL;           /* generic image buffer */
    unsigned char *img_buf_i8=NULL;   /* buffer depending on the data type of */
    short int *img_buf_i16=NULL;      /* the 1G image: BYTE (uint8), */
    unsigned short int *img_buf_ui16 = NULL; /* I2 (int16), UI2 (uint16), or */
    int *img_buf_i32=NULL;            /* I4 (int32) or R4 (float32) */
    float *img_buf_f32=NULL;          

    float offsetline;  /* line offset */
    float offsetsamp;  /* sample offset */
    int k;             /* looping variable */
    int uplineoff;     /* upper line offest */
    int leftsampoff;   /* left sample offest */
    int upline;        /* upper line */
    int lowline;       /* lower line */
    int leftsamp;      /* left sample */
    int rgtsamp;       /* right sample */
    int read_upline;   /* upper line of actual window to read */
    int read_lowline;  /* lower line of actual window to read */
    int read_leftsamp; /* left sample of actual window to read */
    int read_rgtsamp;  /* right sample of actual window to read */
    int read_nl;       /* actual # of lines to read */
    int read_ns;       /* actual # of samples to read */
    int line;          /* line looping variable */
    int sample;        /* sample looping variable */

    /* Find offsets */
    /* LINE: */
    offsetline = win_size[0] / 2.0;
    if (win_size[0] % 2) /* fractional offsetline */
        uplineoff = (int)offsetline + 1;
    else
        uplineoff = (int)offsetline;

    /* SAMPLE: */
    offsetsamp = win_size[1] / 2.0;
    if (win_size[1] % 2) /* fractional offsetsamp */
        leftsampoff = (int)offsetsamp + 1;
    else
        leftsampoff = (int)offsetsamp;

    /* Find starting line/samp (make all lines/samps 0-relative) */
    /* TOP LINE: */
    upline = (int)(pred_line - uplineoff + 1.0);
    if (upline < 0)
    {
       /* Set to start reading at the first line if top line falls above the 
          image */
        read_upline = 0;
    }
    else 
    {
        /* The first line of buffer will contain valid image data starting at 
           line upline */
        read_upline = upline;
    }
    if (upline >= image_nl)
        return OUTSIDE;

    /* LEFT SAMPLE: */
    leftsamp = (int)(pred_samp - leftsampoff + 1.0);
    if (leftsamp < 0)
    {
        /* Set to start reading at the first sample if left sample falls to 
           left of the image */
        read_leftsamp = 0;
    }
    else 
    {
        /* The first samps of buffer will contain valid image data
           starting at sample leftsamp */
        read_leftsamp = leftsamp;
    }
    if (leftsamp >= image_ns)
        return OUTSIDE;

    /* Find ending line/samp */
    /* BOTTOM LINE: */
    lowline = (int)(upline + win_size[0] - 1.0);
    /* lowline is 0-relative, image_nl is a 1-rel count */
    if (lowline >= image_nl) 
    {
        /* Set to stop reading at the last line if bottom line falls below
           the image */
        read_lowline = image_nl - 1;
    }
    else
    {
        /* Stop reading at the last line */
        read_lowline = lowline;
    }
    read_nl = read_lowline - read_upline + 1;
    if (read_nl <= 0)
        return OUTSIDE;

    /* RIGHT SAMPLE: */
    rgtsamp = (int)(leftsamp + win_size[1] - 1.0);
    /* rgtsamp is 0-rel, image_ns is a 1-rel count */
    if (rgtsamp >= image_ns) 
    {
        /* Set to stop reading at the last sample if bottom sample falls to the
           right of the image */
        read_rgtsamp = image_ns - 1;
    }
    else
    {
        /* The last samp of buffer will contain valid image data */
        read_rgtsamp = rgtsamp;
    }
    read_ns = read_rgtsamp - read_leftsamp + 1;
    if (read_ns <= 0)
        return OUTSIDE;

    /* Read the window */
    read_buf = malloc(read_nl * read_ns * sizeof(float));
    if (read_buf == NULL)
    {
        IAS_LOG_ERROR("Allocating memory for search window buffer");
        return ERROR;
    }

    if (dtype == IAS_BYTE) 
    {
        img_buf_i8  = malloc(read_nl * read_ns * sizeof(unsigned char));
        img_buf_gen = img_buf_i8;
    }
    else if (dtype == IAS_I2) 
    {
        img_buf_i16 = malloc(read_nl * read_ns * sizeof(short int));
        img_buf_gen = img_buf_i16;
    }
    else if (dtype == IAS_UI2) 
    {
        img_buf_ui16 = malloc(read_nl * read_ns * sizeof(unsigned short int));
        img_buf_gen = img_buf_ui16;
    }
    else if (dtype == IAS_I4) 
    {
        img_buf_i32 = malloc(read_nl * read_ns * sizeof(int));
        img_buf_gen = img_buf_i32;
    }
    else if (dtype == IAS_R4) 
    {
        img_buf_f32 = malloc(read_nl * read_ns * sizeof(float));
        img_buf_gen = img_buf_f32;
    }
    if (img_buf_gen == NULL)
    {
        IAS_LOG_ERROR("Allocating memory for search window buffer");
        free(read_buf);
        return ERROR;
    }

    /* Read the data from the image */
    if (ias_l1g_read_image(l1g_band, sca, read_upline, read_leftsamp,
        read_nl, read_ns, img_buf_gen) == ERROR)
    {
        IAS_LOG_ERROR("Reading 1g image at line %d, sample %d, line count %d, "
            "sample count %d\n",read_upline, read_leftsamp, read_nl, read_ns);
        free(read_buf);
        free(img_buf_gen);
        return ERROR;
    }

    /* Update the values passed back to the calling routine */
    *upper_line = read_upline;
    *left_sample = read_leftsamp;

    /* Copy from the image's data type into the temporary float buffer */
    if (dtype == IAS_BYTE) 
        for (line = 0,k=0; line < read_nl; line++)
            for (sample = 0; sample < read_ns; sample++,k++)
                read_buf[k] = (float)img_buf_i8[k];
    else if (dtype == IAS_I2) 
        for (line = 0,k=0; line < read_nl; line++)
            for (sample = 0; sample < read_ns; sample++,k++)
                read_buf[k] = (float)img_buf_i16[k];
    else if (dtype == IAS_UI2) 
        for (line = 0,k=0; line < read_nl; line++)
            for (sample = 0; sample < read_ns; sample++,k++)
                read_buf[k] = (float)img_buf_ui16[k];
    else if (dtype == IAS_I4) 
        for (line = 0,k=0; line < read_nl; line++)
            for (sample = 0; sample < read_ns; sample++,k++)
                read_buf[k] = (float)img_buf_i32[k];
    else if (dtype == IAS_R4) 
        for (line = 0,k=0; line < read_nl; line++)
            for (sample = 0; sample < read_ns; sample++,k++)
                read_buf[k] = (float)img_buf_f32[k];

    /* Copy from the temporary window into the final buffer
       in the correct place, zeroing out places where there was
       no data read */
    tmp_buf = img_buf;
    for (line = upline; line <= lowline; line++)
    {
        if ((line < read_upline) || (line > read_lowline)) 
        {
            /* didn't read any data at this line--zero fill */
            for (sample = 0; sample < win_size[1]; sample++)
                *(tmp_buf++) = 0.0;
        }
        else /* read data here--copy */
        {
            for (sample = leftsamp; sample <= rgtsamp; sample++)
            {
                /* didn't read any data here--zero fill */
                if (sample < read_leftsamp) 
                    *(tmp_buf++) = 0.0;
                else if (sample <= read_rgtsamp) /* read data here-copy */
                {
                    *(tmp_buf++) = read_buf[(line-read_upline) * read_ns 
                                            + sample-read_leftsamp];
                }
                else  
                {
                    /* outside right side--didn't read any data here
                       --zero fill */
                    *(tmp_buf++) = 0.0;
                }
            }
        }
    }

    free(read_buf);
    free(img_buf_gen);

    return SUCCESS;
}

