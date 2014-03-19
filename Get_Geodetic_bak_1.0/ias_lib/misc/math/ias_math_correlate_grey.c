/****************************************************************************
NAME:           IAS_MATH_CORRELATE_GREY        

PURPOSE:  Correlate a reference subimage with a search subimage using the pixel
          grey levels and evaluate the results

RETURN VALUE:
Type = int
Value           Description
-----           -----------
ERROR           Unable to perform correlation
SUCCESS         Correlation performed (doesn't guarantee the subimages actually
                correlated well, returned parameters need to be consulted for
                how well it correlated)

NOTES:
This routine may be used to correlate two windows, whether they are the same 
size or different sizes.  The only restrictions are:
-the search window must be as big or bigger than the reference window.
-if the search and reference windows are not the same size, each dimension must 
 be a power of 2
-if the search and reference windows are the same size, the maximum offset may
 be no greater than one fourth of the smallest window dimension.
When correlating same-size windows, the nominal offset between the windows 
(nom_off) is usually (0.0,0.0) unless the calling function does some window 
offsetting.
*** These routines were taken from LAS.  Thus, array parameters are in x 
(sample), y (line) order.  This deviates from the IAS standard of line/sample 
order.  The parameters that are this way are:
  srch_size, ref_size, nom_off, fit_offset, est_err

ALGORITHM REFERENCES:
1.  LAS 4.0 GREYCORR by R. White 6/83

*****************************************************************************/
#include <stdlib.h>
#include <math.h>
#include "ias_math.h"
#include "ias_const.h"
#include "ias_logging.h"
#include "local_defines.h"

int ias_math_correlate_grey
(
    const float *images, /* I: Search subimage                              */
    const float *imager, /* I: Reference subimage                           */
    const int *srch_size,/* I: Actual size of search subimage:  samps,lines */
    const int *ref_size, /* I: Actual size of reference subimage: samps,lines */
    double min_corr,    /* I: Minimum acceptable correlation strength        */
    IAS_CORRELATION_FIT_TYPE fit_method, /* I: Surface Fit Method:
                                    FIT_ELLIP_PARA - Elliptical paraboloid 
                                    FIT_ELLIP_GAUSS - Elliptical Gaussian 
                                    FIT_RECIP_PARA - Reciprocal Paraboloid 
                                    FIT_ROUND - Round to nearest int    */
    double max_disp,    /* I: Maximum allowed diagonal displacement from nominal
                              tiepoint loc to location found by correlation  */
    const double *nom_off,/* I: Nominal horiz & vert offsets of UL corner of 
                              reference subimage relative to search subimage */
    double *strength,   /* O: Strength of correlation                        */
    double *fit_offset, /* O: Best-fit horiz & vert offsets of correlate peak*/
    double *est_err,    /* O: Est horiz error, vert error, and h-v cross term in
                              best-fit offsets (3 values)                     */
    double *diag_disp,  /* O: Actual diagonal displacement from nominal tiepoint
                              location to location found by correlation      */
    int *mult_peak_flag,/* O: subsidiary peak too near edge of search area */
    int *edge_flag,     /* O: peak too near edge of search area */
    int *low_peak_flag, /* O: strength of peak below minimum */
    int *max_disp_flag, /* O: diag displacement from nom location exceeds max*/
    int abs_corr_coeff  /* I: flag to use the abs of the correlation coeffs */
)
{
    int ipkcol[NPEAKS]; /* Col number for each of top 32 correlation values  */
    int ipkrow[NPEAKS]; /* Row number for each of top 32 correlation values  */
    int ncol;           /* Number of columns of search alignments            */
    int nrow;           /* Number of rows of search alignments               */
    int max_off;        /* Integer greater than or equal to max_disp         */
    int same_size;      /* Flag indicating to use same-size correlation      */
    double *ccnorm = NULL;/* Normalized xcorr coefs for each search alignment */
    double cpval[9];    /* 3 by 3 array of xcorr values, in std dev          */
    double pkoffs[2];   /* Fractional horiz & vert offsets of best-fit peak  */
    double pkval[NPEAKS];/* Largest 32 values of normalized xcorr coefs       */
    double sums[2];     /* Sum and sum of squares of all xcorr values        */
    double noffset[2];  /* Nominal offset for use in calculating actual
                           correlation offset                                */
    double *unormc=NULL;/* Unnormalized cross-product sum for each alignment */
    double temp;

    /* Check window sizes */
    if ((srch_size[0] < ref_size[0]) || (srch_size[1] < ref_size[1]))
    {
        IAS_LOG_ERROR("Search window (%d x %d) must not be smaller "
            "than reference window (%d x %d)", srch_size[0], srch_size[1], 
            ref_size[0], ref_size[1]);
        return ERROR;
    }

    /* Initialize error flags and other variables */
    *mult_peak_flag = SUCCESS;
    *edge_flag = SUCCESS;
    *low_peak_flag = SUCCESS;
    *max_disp_flag = SUCCESS;
    *strength = 1.0;
    est_err[0] = 0.0;
    est_err[1] = 0.0;

    /*  Figure out if we are going to do same-size-window correlation or
    different-size-window correlation.  Require windows to have same number of 
    lines AND samples. 
    The reason there are two ways to go about this is, that the different-size
    correlation was inherited from LAS, and it does some slick things with zero
    padding, data transposing, and using the the amount of fill (the difference
    in window sizes) as the valid area of the correlation surface to search
    for the peak.  The same-size correlation method is more straightforward,
    with one wrinkle in the use of changing signs of elements in the
    complex * complex conjugate part of the correlation to center the peak (move
    quadrants around) when doing the inverse fft. */
    if ((srch_size[0] == ref_size[0]) && (srch_size[1] == ref_size[1]))
        same_size = TRUE;
    else
    {
        /* do different-size correlation when either dimension is different */
        same_size = FALSE;
    }

    /* use the same-size correlation routines */
    if (same_size)  
    {
        /* See if this is a case of fine resolution correlation */
        if (srch_size[0]%2 > 0 || srch_size[1]%2 > 0 ||
            srch_size[0] < 6   || srch_size[1] < 6)
        {
            if (ias_math_correlate_fine(images, imager, srch_size, ref_size,
                fit_offset, diag_disp) != SUCCESS)
            {
                *low_peak_flag = ERROR;
                *strength = 0.0;
            }
            return SUCCESS;
        }

        /* Set nominal offsets to 1/2 the search window size 
           since the correlation peak will be near the center 
           of the search window...
           Add in the nominal offsets set by the caller.
           In most cases these should be 0 unless they 
           are doing some additional window offsetting. */
        noffset[0] = (double)(srch_size[0] / 2.0) + nom_off[0];
        noffset[1] = (double)(srch_size[1] / 2.0) + nom_off[1];

        /* The search window will always be as large or larger than the ref
           Set ncol, nrow to the srch window size for use by 
           ias_math_evaluate_grey */
        ncol = srch_size[0];
        nrow = srch_size[1];

        /* Allocate memory for ccnorm & unormc */
        unormc = malloc(srch_size[0] * srch_size[1] * sizeof(*unormc));
        if (unormc == NULL)
        {
            IAS_LOG_ERROR("Error allocating memory");
            return ERROR;
        }
        ccnorm = malloc(srch_size[0] * srch_size[1] * sizeof(*ccnorm));
        if (ccnorm == NULL)
        {
            free(unormc);
            IAS_LOG_ERROR("Error allocating memory");
            return ERROR;
        }

        /* Perform same-size window correlation in the space domain */
        temp = max_disp;
        max_off = (int)ceil( temp );     /* Get the input maximum offset */
        if ( max_off > srch_size[0]/4 )  /* Make sure it's not too big */
            max_off = srch_size[0]/4;    /* Limit offset to 1/4 window size */
        if ( max_off > srch_size[1]/4 )  /* Check both dimensions */
            max_off = srch_size[1]/4;
        if (ias_math_compute_grey_cross_same_size(images, imager, srch_size, 
                max_off, unormc) == ERROR)
        {
            free(unormc);
            free(ccnorm);
            IAS_LOG_ERROR("Error calculating reference-search cross products");
            return ERROR;
        }

        /* Compile cross-correlation statistics */
        ias_math_normalize_grey_cross_same_size(srch_size, unormc, ccnorm,
                pkval, ipkcol, ipkrow, sums, abs_corr_coeff);
    }
    else    
    {
        /* use the different-size correlation routines */
        /* Use the nominal offsets set by the caller */
        noffset[0] = nom_off[0];
        noffset[1] = nom_off[1];

        /* When the windows are different size, and they are padded out to equal
           size with the data in the upper left rather that in the center, this
           introduces an offset in the location of the correlation peak.  This
           offset is win_size/2 - chip_size/2 (or (win_size - ref_size)/2 ).  So
           an area roughly twice the offset is used to search for the peak. */
        ncol = srch_size[0] - ref_size[0] + 1;
        nrow = srch_size[1] - ref_size[1] + 1;

        /* Allocate memory for ccnorm & unormc */
        unormc = malloc(ncol * nrow * sizeof(*unormc));
        if (unormc == NULL)
        {
            IAS_LOG_ERROR("Error allocating memory");
            return ERROR;
        }
        ccnorm = malloc(ncol * nrow * sizeof(*ccnorm));
        if (ccnorm == NULL)
        {
            free(unormc);
            IAS_LOG_ERROR("Error allocating memory");
            return ERROR;
        }

        /* Compute raw cross-product sums */
        if (ias_math_compute_grey_cross(images, imager, srch_size, ref_size, 
                ncol, nrow, unormc) == ERROR)
        {
            free(unormc);
            free(ccnorm);
            IAS_LOG_ERROR("Error calculating reference-search cross products");
            return ERROR;
        }

        /* Compute normalized cross-correlation values and compile statistics */
        if (ias_math_normalize_grey_cross(imager,images,ref_size,srch_size,ncol,
                      nrow,unormc,ccnorm,pkval,ipkcol,ipkrow,sums, 
                      abs_corr_coeff) == ERROR)
        {
            free(unormc);
            free(ccnorm);
            IAS_LOG_ERROR("Error normalizing cross-correlation values");
            return ERROR;
        }
    }

    /* Evaluate strength of correlation peak */
    ias_math_evaluate_grey(ncol,nrow,ccnorm,pkval,ipkcol,ipkrow,sums,min_corr,
              strength,cpval,mult_peak_flag,edge_flag,low_peak_flag);

    /* only use the ias_math_evaluation_grey results if there isn't an edge
           error */
    if (*edge_flag != ERROR) 
    {
        /* Determine offsets of peak relative to nominal location */
        if (fit_method != IAS_FIT_ROUND)
        {
            if (ias_math_fit_registration(cpval, fit_method, pkoffs, est_err) 
                     == ERROR)
            {
                free(unormc);
                free(ccnorm);
                IAS_LOG_ERROR("Error calculating correlation fit");
                return ERROR;
            }
            fit_offset[0] = (double)ipkcol[0] - noffset[0] + pkoffs[0];
            fit_offset[1] = (double)ipkrow[0] - noffset[1] + pkoffs[1];
        }
        else
        {
            fit_offset[0] = (double)ipkcol[0] - noffset[0];
            fit_offset[1] = (double)ipkrow[0] - noffset[1];

            /* Not doing any fitting, just fill in a nominal error */
            est_err[0] = 0.5;
            est_err[1] = 0.5;
        }

         /* Determine diagonal displacement from nominal and check against 
            maximum acceptable value  */
        *diag_disp = sqrt(fit_offset[0] * fit_offset[0]
            + fit_offset[1] * fit_offset[1]);
        if (*diag_disp > max_disp)
            *max_disp_flag = ERROR;
    }

    free(unormc);
    free(ccnorm);

    return SUCCESS;
}
