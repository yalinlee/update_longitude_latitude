/****************************************************************************
NAME:              ias_math_evaluate_grey

PURPOSE:  Evaluate various measures of correlation validity and extract a 
          subarea of the cross correlation array centered on the peak

RETURN VALUE:
No return value

ALGORITHM REFERENCES:
1.0  LAS 4.0 GREYCORR and EDGECORR by R. White 6/83

*****************************************************************************/
#include <stdlib.h>
#include <math.h>
#include "ias_math.h"
#include "local_defines.h"

void ias_math_evaluate_grey
(
    int  ncol,      /* I: Number of columns in coincidence-count array */
    int  nrow,      /* I: Number of rows in coincidence-count array */
    const double *ccnorm,/* I: Array of norm xcorr coeffs */
    const double *pkval,/* I: Table of top 32 normalized values */
    const int  *ipkcol, /* I: Table of column numbers for top 32 values */
    const int  *ipkrow, /* I: Table of row numbers for top 32 values */
    double *sums,       /* O: Sum of all normalized values and sum of squares */
    double min_corr,    /* I: Minimum acceptable correlation strength */
    double *strength,   /* 0: Strength of correlation */
    double *cpval,      /* 0: 3 by 3 array of xcorr vals in std dev */
    int  *mult_peak_flag,/* O: subsidiary peak too near edge of search area */
    int  *edge_flag,    /* O: peak too near edge of search area */
    int  *low_peak_flag /* O: strength of peak below minimum */
)
{
    int ipt5[2];       /* Highest 2 vals > 2 cols or rows from peak value */
    int i,j;           /* Loop counters */
    int icol;          /* Initial col in 9 by 9 neighborhood of corr peak */
    int idist;         /* Max horiz or vert distance from array peak */
    int iptr;          /* Index to 5 by 5 array of output values */
    int ipt7;          /* Index of largest val > 3 cols or rows from peak */
    int irow;          /* Initial row in 9 by 9 neighborhood */
    int krbase;        /* Offset to beginning of current row in array */
    int lcol;          /* Last col in 9 by 9 area centered on peak */
    int lrow;          /* Last row in 9 by 9 neighborhood */
    int npts;          /* #array values outside 9 by 9 neighborhood */
    int n5;            /* #large values outside 5 by 5 neighborhood of peak */
    int n7;            /* #large values outside 7 by 7 neighborhood of peak */
    double bmean;      /* Mean of background cross-correlation values */
    double bsigma;     /* Standard deviation of background about mean */
    double tempa,tempb;/* temporary variables */

    /* Initialize accept/reject code, strength, and outlier pointers */
    *strength = 0.0;
    ipt5[0] = NPEAKS - 1; /* start with the lowest peak in the list */
    ipt5[1] = NPEAKS - 1;
    ipt7 = 0;

    /* Check for peak value within two rows or columns of edge */
    if ((ipkcol[0] <= 1) || (ipkcol[0] >= (ncol - 2)) ||
        (ipkrow[0] <= 1) || (ipkrow[0] >= (nrow - 2)))
    {
        *edge_flag = ERROR;
        return;
    }

    /* Find largest values outside 5 by 5 and 7 by 7 neighborhoods of peak */
    n5 = 0;
    n7 = 0;
    i = 1;

    for (;;)
    {
        if ((n5 > 1) || (i > (NPEAKS - 1)))
            break;
        tempa=abs((int)(ipkcol[0] - ipkcol[i]));
        tempb=abs((int)(ipkrow[0] - ipkrow[i]));
        idist = tempa > tempb ? (int)tempa : (int)tempb;
        if (idist > 2) 
        {
            /* n5 counts those outside 5x5 area of peak, meaning dist > 2 */
            ipt5[n5] = i;
            n5++;
            if ((n7 == 0) && (idist > 3)) 
            {
                /* n7 counts outside 7x7 area, dist > 3 */
                ipt7 = i;
                n7 = 1;
            }
        }
        i++;
    }

    /* If another peak was found near the primary peak */
    if ((ipt5[0] <= 2) || (ipt5[1] <= 4))
    {
         /* Log information about the point and return multi-peak flag but
            don't return an error since the caller may choose to ignore this 
            flag */
         *mult_peak_flag = ERROR;
    }

    /* Find edges of 9 by 9 array centered on peak */
    icol = 0 > (ipkcol[0] - 4) ? 0 : ipkcol[0] - 4;
    irow = 0 > (ipkrow[0] - 4) ? 0 : ipkrow[0] - 4;
    lcol = (ncol - 1) < (ipkcol[0] + 4) ? ncol - 1 : ipkcol[0] + 4;
    lrow = (nrow - 1) < (ipkrow[0] + 4) ? nrow - 1 : ipkrow[0] + 4;

    /* Eliminate points within 9 by 9 array from background statistics */
    krbase = ncol * irow;
    for (i = irow; i <= lrow; i++)
    {
        for (j = icol; j <= lcol; j++)
        {
            sums[0] -= ccnorm[krbase+j];
            sums[1] -= ccnorm[krbase+j]*ccnorm[krbase+j];
        }
        krbase += ncol;
    }

    /* Compute background mean and standard deviation */
    npts = ncol * nrow - (lcol - icol + 1) * (lrow - irow + 1);
    if (npts <= 0)
    {
        bmean = 0.0;
        bsigma = 1.0;
    }
    else
    {
        bmean = sums[0] / (double)npts;
        tempa = sums[1] / (double)npts - bmean * bmean;
        tempa = MINSIGMA > tempa ? MINSIGMA : tempa;
        bsigma = sqrt(tempa);
    }

    /* Compute correlation strength and check against minimum */
    if (n7 == 0)
        *strength = 2 * ((pkval[0] - bmean) / bsigma) - 0.2;
    else 
    {
        *strength = (pkval[0] - bmean) / bsigma + 
                    (pkval[0] - pkval[ipt7]) / bsigma + 0.2 * (n7 - 1.0);
    }

    if (*strength < min_corr)
    {
        *low_peak_flag = ERROR;
    }

    /* Convert 3 by 3 neighborhood of peak to standard deviations above mean */
    krbase = ncol*(ipkrow[0] - 1);
    for (iptr = 0, i = 0; i < 3; i++, krbase += ncol)
    {
        for (j = ipkcol[0] - 1; j <= ipkcol[0] + 1; j++, iptr++)
            cpval[iptr] = (ccnorm[krbase + j] - bmean) / bsigma;
    }
}
