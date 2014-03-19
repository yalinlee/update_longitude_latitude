/****************************************************************************
NAME: ias_math_compute_grey_cross

PURPOSE: Provides the routines needed to compute the unnormalized (raw) sum of
    pixel-by-pixel cross products between the reference and search images for
    every combination of horizontal and vertical offsets of the reference
    relative to the search image.  The main routine is
    ias_math_compute_grey_cross.

ALGORITHM REFERENCES:
1.  LAS 4.0 GREYCORR by R. White & G. Neal 8/83

*****************************************************************************/
#include <stdlib.h>
#include <math.h>
#include "ias_math.h"
#include "ias_logging.h"
#include "local_defines.h"


/****************************************************************************
NAME:                                IAS_MATH_FFTND

PURPOSE:  Generic N-d FFT function

RETURN VALUE:
No return value

NOTES:

ALGORITHM REFERENCES:
   This routine was derived from a multi-dimensional FFT in "Numerical 
   Recipes in C", Section 12.4, by Press, FLannery, Teukolsky, and 
   Vetterling, Cambridge University Press, 1988.
******************************************************************************/
void ias_math_fftnd
(
    double data[], /* I/O: array */
    int nn[],      /* I: dimensions of array 1) Rows and 2) columns of array */
    int ndim,      /* I: array dimension */
    int isign      /* I: Flag where 1 = 2 dimensional discrete Fourier Transform
                         -1 = inverse transform times the product of
                              the lengths of all dimensions */
)
{
    int i1,i2,i3,i2rev,i3rev,ip1,ip2,ip3,ifp1,ifp2;  /* looping indices */
    int ibit,idim,k1,k2,n,nprev,nrem,ntot;           /* looping indices */
    float tempi,tempr;                       /* temporary real & imag values */
    double theta,wi,wpi,wpr,wr,wtemp;        /* Double for trig recurrences */
    double two_pi;                           /* variable to store 2*pi */

    /* calculate the value of 2*pi */
    two_pi = ias_math_get_pi() * 2.0;

    ntot = 1;
    /* Calc no of complex vals */
    for (idim = 1; idim <= ndim; idim++)
        ntot *= nn[idim];
    nprev = 1;
    for (idim = ndim; idim >= 1; idim--) 
    {
        n = nn[idim];
        nrem = ntot/(n * nprev);
        ip1 = nprev << 1;
        ip2 = ip1 * n;
        ip3 = ip2 * nrem;
        i2rev = 1;

        /* Perform bit reversal */
        for (i2 = 1; i2 <= ip2; i2 += ip1)
        {
            if (i2 < i2rev) 
            {
                for (i1 = i2; i1 <= i2 + ip1-2; i1 += 2) 
                {
                    for (i3 = i1; i3 <= ip3; i3 += ip2) 
                    {
                        i3rev = i2rev + i3-i2;
                        SWAP(data[i3],data[i3rev]);
                        SWAP(data[i3 + 1],data[i3rev + 1]);
                    }
                }
            }
            ibit = ip2 >> 1;
            while (ibit >= ip1 && i2rev > ibit) 
            {
                i2rev -= ibit;
                ibit >>= 1;
            }
            i2rev += ibit;
        }
        /* Danielson-Lanczos section */
        ifp1 = ip1;
        while (ifp1 < ip2) 
        {
            ifp2 = ifp1 << 1;
            /* Init. trig recurrence */
            theta = isign * two_pi/(ifp2/ip1);
            wtemp = sin(0.5 * theta);
            wpr = -2.0 * wtemp * wtemp;
            wpi = sin(theta);
            wr = 1.0;
            wi = 0.0;
            for (i3 = 1; i3 <= ifp1; i3 += ip1) 
            {
                for (i1 = i3; i1 <= i3 + ip1 - 2; i1 += 2) 
                {
                     for (i2 = i1; i2 <= ip3; i2 += ifp2) 
                     {
                         k1 = i2;
                         k2 = k1 + ifp1;
                         tempr = wr * data[k2] - wi * data[k2 + 1];
                         tempi = wr * data[k2 + 1] + wi * data[k2];
                         data[k2] = data[k1] - tempr;
                         data[k2 + 1] = data[k1 + 1] - tempi;
                         data[k1] += tempr;
                         data[k1 + 1] += tempi;
                      }
               }
               /* Trig recurrence */
               wr += (wtemp = wr) * wpr - wi * wpi;
               wi += wi * wpr + wtemp * wpi;
           }
           ifp1 = ifp2;
        }
        nprev *= n;
    }
}

/****************************************************************************
NAME:                ias_math_fft2d

PURPOSE:
Calculate a 2-d discrete Fourier Transform

RETURN VALUE:
No return value

NOTES:
   FFT2D replaces DATA by its 2 dimensional discrete Fourier Transform,
   if ISIGN = 1.  Each dimension must be a power of 2.  DATA is an array of 
   type float, twice the length of the product of these lengths in which the 
   data are stored as in a multidimensional complex array:  real and imaginary 
   parts of each element are in consecutive locations, and the rightmost index 
   of the array increases most rapidly as one proceeds along DATA.  This is 
   equivalent to storing the array by rows (the C-way; FORTRAN stores by 
   columns!).  IF ISIGN = -1, DATA is replaced by its inverse transform times 
   the product of the lengths of all dimensions.

ALGORITHM REFERENCES:
   This routine was derived from a multi-dimensional FFT in "Numerical 
   Recipes in C", by Press, FLannery, Teukolsky, and Vetterling, 
   Cambridge University Press, 1988.

*****************************************************************************/
void ias_math_fft2d
(
    double *data,  /* I/O: array */
    int  nrows,    /* I: number of rows */
    int  ncols,    /* I: number of columns */
    int  isign     /* I: Flag where 1 = 2 dimensional discrete Fourier Transform
                         -1 = inverse transform times the product of
                              the lengths of all dimensions */
)
{
    int  dim[3];   /* dimensions of array, (1) Rows and (2) columns of array */

    /* back up the pointer because it comes in as a 0-relative array, but the
       fft routines want to work with an array that starts at 1 */
    data--;

    if ((nrows == 0) || (ncols == 0))
        return;
    dim[1] = nrows;
    dim[2] = ncols;
    ias_math_fftnd(data,dim,2,isign);        /* Call generic n-d FFT */
}

/****************************************************************************
NAME:     ias_math_compute_grey_cross                   

PURPOSE:  Compute the unnormalized (raw) sum of pixel-by-pixel cross products 
          between the reference and search images for every combination of 
          horizontal and vertical offsets of the reference relative to the 
          search image

RETURN VALUE:
Type = int
Value           Description
-----           -----------
ERROR           Error allocating memory for cross product calculation
SUCCESS         Calculated cross products

*****************************************************************************/
int ias_math_compute_grey_cross
(
    const float *images, /* I: Search subimage */
    const float *imager, /* I: Reference subimage */
    const int *srch_size,/* I: Actual size of search subimage:  samps/lines */
    const int *ref_size, /* I: Actual size of reference subimage: samps/lines */
    int ncol,           /* I: # of columns in cross-product sums array(unormc)*/
    int nrow,           /* I: # of rows in cross-product sums array (unormc) */
    double *unormc      /* O: Array of unnormalized (raw) counts of edge 
                              coincidences for each alignment of reference
                              image relative to search image */
)
{
    float *membfr;      /* Buffer in which reference subimage is properly 
                           aligned and zero padded                           */
    float *membfs;      /* Buffer for aligning and padding search subimage*/
    int imgptr;         /* Pointer into subimage arrays                   */
    int line;           /* Loop index:  current buffer line               */
    int lncont;         /* Pointer to start of next segment of line       */
    int lnstrt;         /* Pointer to first segment of line               */
    int memptr;         /* Loop index:  pointer into AP transfer buffers  */
    int ndxout;         /* Pointer into array for correlation output      */
    int nsnew[2];       /* Size of search window                          */
    int nrnew[2];       /* Size of reference window                       */
    int memdim[2];      /* Power-of-2 dimensions for AP arrays            */
    int i,j;            /* Loop counters                                  */
    double denom;       /* Denominator to calc unnormalized xcorr values  */
    double tempim;      /* Temporary imaginary value when replacing cser  */
    IAS_COMPLEX *cser;  /* Complex of search subimage                     */
    IAS_COMPLEX *cref;  /* Complex of reference subimage                  */

    nsnew[0] = srch_size[0];
    nsnew[1] = srch_size[1];
    nrnew[0] = ref_size[0];
    nrnew[1] = ref_size[1];

    /* Zero extend search image to next higher power of 2
       Minimum window size is 64x64 */
    for (i = 0; i < 2; i++)
    {
        memdim[i] = 64;
        for (;;)
        {
            if (nsnew[i] <= memdim[i])
                break;
            memdim[i] *= 2;
        }
    }

    /* Fix for incorrect handling of non-square arrays */
    memdim[0] = memdim[0] > memdim[1] ? memdim[0] : memdim[1];
    memdim[1] = memdim[0];

    membfr = (float *)malloc(memdim[0] * memdim[1] * sizeof(float));
    if (membfr == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        return ERROR;
    }
    membfs = (float *)malloc(memdim[0] * memdim[1] * sizeof(float));
    if (membfs == NULL)
    {
        free(membfr);
        IAS_LOG_ERROR("Error allocating memory");
        return ERROR;
    }
    cser = (IAS_COMPLEX *)malloc(memdim[0] * memdim[1] * sizeof(IAS_COMPLEX));
    if (cser == NULL)
    {
        free(membfr);
        free(membfs);
        IAS_LOG_ERROR("Error allocating memory");
        return ERROR;
    }
    cref = (IAS_COMPLEX *)malloc(memdim[0] * memdim[1] * sizeof(IAS_COMPLEX));
    if (cref == NULL)
    {
        free(membfr);
        free(membfs);
        free(cser);
        IAS_LOG_ERROR("Error allocating memory");
        return ERROR;
    }

    for (lnstrt = 0, imgptr = 0, line = 0; line < memdim[1]; line++)
    {
        if (line < nsnew[1])
        {
            lncont = lnstrt + nsnew[0];
            for (memptr = lnstrt; memptr < lncont; imgptr++, memptr++)
                membfs[memptr] = images[imgptr];
        }
        else
            lncont = lnstrt;
        lnstrt += memdim[0];
        if (lncont < lnstrt)
        {
            for (memptr = lncont; memptr < lnstrt; memptr++)
                membfs[memptr] = 0.0;
        }
    }

    /* Make complex 2-d of search data
       Note that the data are put into cser in column-major order since the 
       fft routine wants the data that way (see ias_math_fft2d). */
    for (memptr = 0, i = 0; i < memdim[1]; i++)
    {
        for (j = 0; j < memdim[0]; j++, memptr++)
        {
            cser[j * memdim[1] + i].re = membfs[memptr];
            cser[j * memdim[1] + i].im = 0.0;
        }
    }

    /* Now zero extend reference image */
    for (lnstrt = 0, imgptr = 0, line = 0; line < memdim[1]; line++)
    {
        if (line < nrnew[1]) 
        {
            lncont = lnstrt + nrnew[0];
            for (memptr = lnstrt; memptr < lncont; imgptr++, memptr++)
                membfr[memptr] = imager[imgptr];
        }
        else
            lncont = lnstrt;
        lnstrt += memdim[0];
        if (lncont < lnstrt) 
        {
            for (memptr = lncont; memptr < lnstrt; memptr++)
                membfr[memptr] = 0.0;
        }
    }

    /* Make complex 2-d of reference data
       Note that the data are put into cser in column-major order since the 
       fft routine wants the data that way (see ias_math_fft2d). */
    for (memptr = 0, i = 0; i < memdim[1]; i++)
    {
        for (j = 0; j < memdim[0]; j++, memptr++)
        {
            cref[j * memdim[1] + i].re = membfr[memptr];
            cref[j * memdim[1] + i].im = 0.0;
        }
    }

    /* Take fft of search and reference data */
    ias_math_fft2d((double *)cser,memdim[1],memdim[0],1);
    ias_math_fft2d((double *)cref,memdim[1],memdim[0],1);

    /* Make point by multiplication of search subimage ft with conjugate of 
       reference subimage ft */
    for (i = 0; i < memdim[0] * memdim[1]; i++)
    {
        tempim = cref[i].re * cser[i].im-cref[i].im * cser[i].re;
        cser[i].re = cref[i].re * cser[i].re + cref[i].im * cser[i].im;
        cser[i].im = tempim;
    }

    /* Take inverse fft of cser */
    ias_math_fft2d((double *)cser,memdim[1],memdim[0],-1);

    /* Extract part of correlation array which is valid */
    denom = memdim[0] * memdim[1];
    ndxout = 0;
    for (i = 0; i < nrow; i++)
    {
        for (j = 0; j < ncol; j++, ndxout++)
            unormc[ndxout] = cser[j * memdim[1] + i].re / denom;
    }

    free(membfr);
    free(membfs);
    free(cser);
    free(cref);

    return SUCCESS;
}
