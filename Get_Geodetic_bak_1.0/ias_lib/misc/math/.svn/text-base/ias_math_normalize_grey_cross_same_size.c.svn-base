/****************************************************************************
NAME:                        ias_math_normalize_grey_cross_same_size

PURPOSE:  Convert raw cross-product sums to normalized cross-correlation 
          coefficients, using tabulated statistics from previous step 
          (grey_cross_ss).

RETURNS: nothing

NOTES:
This function is much simpler than the one for unequal-sized windows, since all
normalizing is done by the space domain same size correlator. All that has to
be done here is statistics gathering to set up the peak finder.

*****************************************************************************/
#include <math.h>
#include "ias_math.h"
#include "local_defines.h"

void ias_math_normalize_grey_cross_same_size
(
    const int *surf_size, /* I: Size of correlation surface (unormc and ccnorm)
                              samps,lines */
    const double *unormc, /* I: Array of raw cross product sums */
    double *ccnorm,       /* O: Array of norm xcorr coeffs */
    double *pkval,        /* O: Table of top NPEAK normalized values */
    int *ipkcol,          /* O: Table of column numbers for top NPEAK values */
    int *ipkrow,          /* O: Table of row numbers for top NPEAK values */
    double sums[2],       /* O: Sum of all normalized values, sum of squares */
    int abs_corr_coeff    /* I: the flag to use the abs correlation coeffs */
)
{
    int iptr[NPEAKS];     /* Index to values in location arrays           */
    int ixcol[NPEAKS];    /* Col locals of NPEAKS largest values          */
    int ixrow[NPEAKS];    /* Row locations of NPEAKS largest values       */
    double xval[NPEAKS];  /* Temp peak value storage array                */
    int i,j,pk,m;         /* Loop counters                                */
    int ipfree;           /* Element freed by deleting NPEAKS'th smallest */


    /* Clear sums and sums of squares of normalized correlation values
       Also initialize peak location & value arrays */
    sums[0] = sums[1] = 0.0;
    for (pk = 0; pk < NPEAKS; pk++)
    {
        xval[pk] = -1.0;
        iptr[pk] = pk;
        ixcol[pk] = 0.0;
        ixrow[pk] = 0.0;
    }

    /* Load the cross-correlation surface, looping over lines/rows */
    for (i = 0, m = 0; i < surf_size[1]; i++)
    {
        /* Looping over samples/cols */
        for (j = 0; j < surf_size[0]; j++,m++)
        {
            if (abs_corr_coeff == TRUE)
                ccnorm[m] = fabsf(unormc[m]);
            else
                ccnorm[m] = unormc[m];

            sums[0] += ccnorm[m];
            sums[1] += ccnorm[m] * ccnorm[m];

            /* Check whether value among top NPEAKS */
            if (ccnorm[m] > xval[iptr[NPEAKS-1]]) 
            {
                pk = NPEAKS-1;
                ipfree = iptr[NPEAKS-1];
                for (;;pk--)
                {
                    if ((pk <= 0) || (ccnorm[m] <= xval[iptr[pk-1]]))
                        break;
                    iptr[pk] = iptr[pk-1];
                }
                iptr[pk] = ipfree;
                xval[ipfree] = ccnorm[m];
                ixcol[ipfree] = j;
                ixrow[ipfree] = i;
            }
        }
    }

    /* Copy peak values and coordinates in correct sequence */
    for (pk = 0; pk < NPEAKS; pk++)
    {
        pkval[pk] = xval[iptr[pk]];
        ipkcol[pk] = ixcol[iptr[pk]];
        ipkrow[pk] = ixrow[iptr[pk]];
    }
}
