/****************************************************************************
NAME: IAS_MATH_COMPUTE_GREY_CROSS_SAME_SIZE

PURPOSE:  Compute the unnormalized (raw) sum of pixel-by-pixel cross products 
          between the reference and search images for every combination of 
          horizontal and vertical offsets of the reference relative to the 
          search image for windows of the same size (in one dimension at least)

RETURN VALUE: SUCCESS or ERROR

NOTES:
This routine requires search and reference dimensions to be the same.

*****************************************************************************/
#include <stdlib.h>                 /* for malloc */
#include <math.h>
#include "ias_math.h"               /* correlation prototypes & defines */
#include "ias_logging.h"

int ias_math_compute_grey_cross_same_size
(
    const float *images,  /* I: Search subimage */
    const float *imager,  /* I: Reference subimage */
    const int *win_size,  /* I: Actual size of subimage windows: samps/lines */
    int max_off,          /* I: Maximum offset to search for */
    double *unormc        /* O: Array of unnormalized (raw) counts of edge 
                                coincidences for each alignment of reference
                                image relative to search image */
)
{
    int memptr;            /* Loop index:  pointer into AP transfer buffers  */
    int i,j;               /* Loop counters                                   */
    int is, il;            /* Image loop indices                          */
    int cs, cl;            /* Correlation loop indices                    */
    double tmpref,tmpsrch; /* Temp reference and search pixel values */
    double srchsum,srchsqr;/* Search subimage sum & sum of squares          */
    double refsum,refsqr;  /* Reference subimage sum & sum of squares */
    double crossum;        /* Sum of reference and search cross terms */
    double size;           /* Double version of correlation window size */
    int ref_size[2];       /* Reference subimage dimensions  */
    int ref_off[2];        /* Offset to first reference subimage  */
    int cor_size[2];       /* Correlation surface dimensions */
    double *csurf;         /* Correlation surface array */

    /* Compute the size of the windows to be compared from the input window
       size and the maximum offset */
    ref_size[0] = win_size[0] - 2 * (max_off + 1);
    ref_size[1] = win_size[1] - 2 * (max_off + 1);
    size = (double)(ref_size[0] * ref_size[1]);

    /* Compute the line and sample offsets to the first reference subwindow */
    ref_off[0] = max_off + 1;
    ref_off[1] = max_off + 1;

    /* Compute the size of the output correlation surface */
    cor_size[0] = win_size[0] - ref_size[0] + 1;
    cor_size[1] = win_size[1] - ref_size[1] + 1;

    /* Allocate the correlation array */
    csurf = malloc(cor_size[0] * cor_size[1] * sizeof(*csurf));
    if (csurf == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        return ERROR;
    }

    /* Initialize the reference window stats */
    refsum = 0.0;
    refsqr = 0.0;
    for (il = 0; il < ref_size[1]; il++)
    {
        for (is = 0; is < ref_size[0]; is++)
        {
            tmpref = imager[(ref_off[1] + il) * win_size[0] + ref_off[0] + is];
            refsum += tmpref;
            refsqr += tmpref * tmpref;
        }
    }
    /* Loop through correlation surface (offsets) */
    for (cl = 0; cl < cor_size[1]; cl++)
    {
        /* Initialize correlation subwindow stats */
        srchsum = 0.0;
        srchsqr = 0.0;
        for (il = 0; il < ref_size[1]; il++)
        {
            for (is = 0; is < ref_size[0]-1; is++)
            {
                 tmpsrch = images[(il + cl) * win_size[0] + is];
                 srchsum += tmpsrch;
                 srchsqr += tmpsrch * tmpsrch;
            }
        }
        for (cs = 0; cs < cor_size[0]; cs++)
        {
            /* Compute the cross terms */
            crossum = 0.0;
            for (il = 0; il < ref_size[1]; il++)
            {
                for (is = 0; is < ref_size[0]; is++)
                {
                    tmpref = imager[(ref_off[1] + il) * win_size[0]
                                    + ref_off[0] + is];
                    tmpsrch = images[(cl + il) * win_size[0] + cs + is];
                    crossum += tmpref * tmpsrch;
                }
            }
            /* Add the last column to the window stats */
            is = ref_size[0] - 1;
            for (il = 0; il < ref_size[1]; il++)
            {
                tmpsrch = images[(cl + il) * win_size[0] + cs + is];
                srchsum += tmpsrch;
                srchsqr += tmpsrch * tmpsrch;
            }
            /* Compute the correlation coefficient */
            tmpref = refsqr-refsum/size * refsum;
            tmpsrch = srchsqr-srchsum/size * srchsum;
            csurf[cl * cor_size[0] + cs] = (tmpref * tmpsrch > 0.0 ?
                (crossum-refsum/size * srchsum)/sqrt(tmpref * tmpsrch) : 0.0);
            /* Subtract the first column from the window stats */
            is = 0;
            for (il = 0; il < ref_size[1]; il++)
            {
                tmpsrch = images[(cl + il) * win_size[0] + cs + is];
                srchsum -= tmpsrch;
                srchsqr -= tmpsrch * tmpsrch;
            }
        }
    }
      
    /* Copy out correlation surface */
    ref_off[0] = (int)(win_size[0] - cor_size[0] + 1)/2;
    ref_off[1] = (int)(win_size[1] - cor_size[1] + 1)/2;

    /* lines */
    for (i = 0, memptr = 0; i < win_size[1]; i++) 
    {
        /* samples */
        for (j = 0; j < win_size[0]; j++, memptr++) 
        {
            /* Fill in zeros around the valid offsets in the output buffer */
            if (i < ref_off[1] || j < ref_off[0] || 
                 i >= ref_off[1] + cor_size[1] || j >= ref_off[0] + cor_size[0])
            {
                unormc[memptr] = 0.0;
            }
            else
            {
                unormc[memptr] = *(csurf + (i - ref_off[1]) * 
                                      cor_size[0] + (j - ref_off[0]));
            }
        }
    }

    free(csurf);

    return SUCCESS;
}
