/****************************************************************************
NAME: ias_math_normalize_grey_cross

PURPOSE:  Convert raw cross-product sums to normalized cross-correlation 
          coefficients, while tabulating statistics needed for subsequent 
          evaluation

RETURN VALUE:
Type = int
Value           Description
-----           -----------
ERROR           Error allocating memory for normalization
SUCCESS         Cross products normalized

NOTES:

ALGORITHM REFERENCES:
1.  LAS 4.0 GREYCORR by R. White 8/83 and Y. Jun 7/84

*****************************************************************************/
#include <stdlib.h>
#include <math.h>
#include "ias_math.h"
#include "ias_logging.h"
#include "local_defines.h"

int ias_math_normalize_grey_cross
(
    const float *imager, /* I: Reference subimage */
    const float *images, /* I: Search subimage */
    const int *ref_size, /* I: Actual size of reference subimage--samps/lines */
    const int *srch_size,/* I: Actual size of search subimage--samps/lines */
    int  ncol,          /* I: #cols in cross-product sum array            */
    int  nrow,          /* I: #rows in cross-product sum array            */
    const double *unormc,/* I: Array of raw cross product sums */
    double *ccnorm,     /* O: Array of norm xcorr coeffs */
    double *pkval,      /* O: Table of top 32 normalized values */
    int  *ipkcol,       /* O: Table of column numbers for top 32 values */
    int  *ipkrow,       /* O: Table of row numbers for top 32 values */
    double *sums,       /* O: Sum of all normalized values, & sum of squares*/
    int abs_corr_coeff  /* I: the flag to use the abs of the correlation 
                              coeffs*/
)
{
    int  iptr[NPEAKS];   /* Index to values in location arrays        */
    int  ixcol[NPEAKS];  /* Col locals of 32 largest values           */
    int  nsnew[2];                
    int  nrnew[2];
    int  ixrow[NPEAKS];  /* Row locations of 32 largest values        */
    int  i,j,k;          /* Loop counters                             */
    int  ipfree;         /* Index to element freed by deleting 32-nd smallest*/
    int  ipt;            /* Index to image array                             */
    int  jstart;         /* Pointer to first element in current row          */
    int  jstop;          /* Pointer to last element in current row           */
    int  kol;            /* Loop counter                                     */
    int  koladd;         /* #search chip col added                           */
    int  kolsub;         /* #search chip col subtracted                      */
    int  line;           /* Loop counter                                     */
    int  lnadd;          /* #search chip line added                          */
    int  lnsub;          /* #search chip line subtracted                     */
    int  nrtot;          /* Number of pixels in reference image chip         */
    double iglnew;       /* New srch pixel to add into sums                  */
    double iglold;       /* Old srch pixel to remove from sums               */
    double *colsqr;      /* Sum of squares of search (current overlain area) */
    double *colsum;      /* Col sum of search chip (currently overlain area) */
    double xval[NPEAKS]; /* Temp peak value storage array                    */
    double refsum;       /* Sum of all reference-subimage pixel values       */
    double refsqr;       /* Sum of squares of reference-subimage pixels      */
    double rho;          /* Temp storage for xcorr coeffs                    */
    double rmean;        /* Mean of reference-subimage pixel values          */
    double rsigma;       /* Std Dev about mean of ref chip pixel values      */
    double rtotal;       /* Number of pixels in reference subimage           */
    double sigma1;       /* Val after 1st iter of Newton's method to app sqrt*/
    double sigmas = 0.0; /* STD DEV mult by rtotal--initial est for next row */
    double srchsm = 0.0; /* Sum of pixel vals for search subarea             */
    double srchsq = 0.0; /* Sum of squares of search subarea pixels          */
    double temp;         /* Temp var for search subarea STD DEV calc         */
    double tempmn;       /* Min val of temp--avoids divide by small values   */

    /* Compute pixel-value statistics for reference subimage */
    nsnew[0] = srch_size[0];
    nsnew[1] = srch_size[1];
    nrnew[0] = ref_size[0];
    nrnew[1] = ref_size[1];
    refsum = refsqr = 0;
    nrtot = nrnew[0] * nrnew[1];
    for (ipt = 0; ipt < nrtot; ipt++)
    {
        refsum += imager[ipt];
        refsqr += (double)imager[ipt] * imager[ipt];
    }
    rtotal = (double)nrtot;
    tempmn = MINSIGMA * rtotal * rtotal;
    rmean = refsum/rtotal;
    rsigma = refsqr/rtotal - rmean * rmean;
    rsigma = rsigma > MINSIGMA ? rsigma : MINSIGMA;
    rsigma = sqrt(rsigma);

    /* Allocate memory for column sums */
    i = (nsnew[0] > nrnew[0]) ? nsnew[0] : nrnew[0];
    colsum = malloc(i * sizeof(*colsum));
    if (colsum == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        return ERROR;
    }
    colsqr = malloc(i * sizeof(*colsqr));
    if (colsqr == NULL)
    {
        free(colsum);
        IAS_LOG_ERROR("Error allocating memory");
        return ERROR;
    }

    /* Clear sums and sums of squares of normalized correlation values */
    sums[0] = sums[1] = 0.0;
    for (k = 0; k < NPEAKS; k++)
    {
        xval[k] = -1.0;
        iptr[k] = k;
        ixcol[k] = 0.0;
        ixrow[k] = 0.0;
    }

    /* Compute normalized cross-corr values for one row of alignemnts at a time
       */
    /* Get column sums and sums of squares for portion of search subimage 
       overlain by reference in current row of alignments */
    for (jstart = 0, jstop = ncol, i = 0; i < nrow; i++, jstart += ncol, 
         jstop += ncol)
    {
        if (i == 0) 
        {
            for (kol = 0; kol < nsnew[0]; kol++)
                colsum[kol] = colsqr[kol] = 0.0;

            for (ipt = 0, line = 0; line < nrnew[1]; line++)
            {
                for (kol = 0; kol < nsnew[0]; kol++, ipt++)
                {
                    colsum[kol] += images[ipt];
                    colsqr[kol] += (double)images[ipt] * images[ipt];
                }
            }
        }
        else
        {
            lnsub = (i - 1) * nsnew[0];
            lnadd = lnsub + nsnew[0] * nrnew[1];
            for (kol = 0; kol < nsnew[0]; kol++)
            {
                iglnew = images[lnadd+kol];
                iglold = images[lnsub+kol];
                colsum[kol] = colsum[kol] + iglnew - iglold;
                colsqr[kol] = colsqr[kol] + iglnew * iglnew - 
                iglold * iglold;
            }
        }

        /* Complete commutation of search-subarea pixel statistics */
        for (j = jstart; j < jstop; j++)
        {
            if (j == jstart) 
            {
                srchsm = srchsq = 0.0;
                for (kol = 0; kol < nrnew[0]; kol++)
                {
                    srchsm += colsum[kol];
                    srchsq += colsqr[kol];
                }
                temp = rtotal * srchsq - srchsm * srchsm;
                temp = tempmn > temp ? tempmn : temp;
                sigmas = sqrt(temp);
            }
            else
            {
                kolsub = (j - jstart) - 1;
                koladd = kolsub + nrnew[0];
                srchsm = srchsm + colsum[koladd] - colsum[kolsub];
                srchsq = srchsq + colsqr[koladd] - colsqr[kolsub];
                temp = rtotal * srchsq - srchsm * srchsm;
                temp = tempmn > temp ? tempmn : temp;
                sigma1 = 0.5 * (sigmas + temp/sigmas);
                sigmas = 0.5 * (sigma1 + temp/sigma1);
            }

            /* Compute normalized cross-correlation value */
            rho = (unormc[j] - rmean * srchsm) / (rsigma * sigmas);
            if (abs_corr_coeff == TRUE)
                ccnorm[j] = fabsf(rho);
            else
                ccnorm[j] = rho;
            sums[0] += ccnorm[j];
            sums[1] += ccnorm[j] * ccnorm[j];

            /* Check whether value among top NPEAKS */
            if (rho > xval[iptr[NPEAKS-1]]) 
            {
                k = NPEAKS-1;
                ipfree = iptr[NPEAKS-1];
                for (;;k--)
                {
                    if((k <= 0)||(rho <= xval[iptr[k-1]]))
                        break;
                    iptr[k] = iptr[k-1];
                }
                iptr[k] = ipfree;
                xval[ipfree] = rho;
                ixcol[ipfree] = j - jstart;
                ixrow[ipfree] = i;
            }
        }
    }

    /* Copy peak values and coordinates in correct sequence */
    for (k = 0; k < NPEAKS; k++)
    {
        pkval[k] = xval[iptr[k]];
        ipkcol[k] = ixcol[iptr[k]];
        ipkrow[k] = ixrow[iptr[k]];
    }

    free(colsqr);
    free(colsum);

    return SUCCESS;
}
