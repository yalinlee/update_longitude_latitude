/****************************************************************************
NAME: IAS_MATH_CORRELATE_FINE

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
- This routine may be used to correlate two windows that are the same size.
- The input search and reference subimages are expected to not have zero or
  negative values.

*****************************************************************************/
#include <math.h>
#include "ias_math.h"

int ias_math_correlate_fine
(
    const float *images, /* I: Search subimage */
    const float *imager, /* I: Reference subimage */
    const int *srch_size,/* I: Actual size of search subimage:  samps,lines */
    const int *ref_size, /* I: Actual size of reference subimage: samps,lines */
    double *fit_offset, /* O: Best-fit horizontal & vertical offsets of 
                              correlation peak */
    double *diag_disp   /* O: Actual diagonal displacement from nominal 
                              tiepoint location to location found by 
                              correlation */
)
{
    double norm[16];    /* Least squares normal equations matrix */
    double ninv[16];    /* Inverse of norm */
    double cvec[4];     /* Constants vector C */
    double pvec[4];     /* Parameter vector */
    double s0;          /* Central pixel in the 3x3 search sub-window */
    double sx;          /* Slope estimate in the sample direction */
    double sy;          /* Slope estimate in the line direction */
    double sxy;         /* Rate of slope change */ 
    double dnr;         /* DN of the reference pixel corresponding to s0 */
    double rhs;         /* Right hand side of the observation equation */
    double ds;          /* Delta sample */
    double dl;          /* Delta line */
    double g;           /* Delta gain */
    double b;           /* Delta bias */
    double sig_g = 0.05;/* Apriori gain standard deviation (percent) */
    double sig_b = 5.0; /* Apriori bias standard deviation (DN) */
    int ii;             /* Loop index */
    int jj;             /* Loop index */
    int kk;             /* Loop index */

    /* Check to make sure the image windows are compatible */
    if (srch_size[0] != ref_size[0] || srch_size[1] != ref_size[1] ||
        srch_size[0] < 3 || srch_size[1] < 3) 
    {
        /* don't output an error message since the calling routine has to
           really figure out if this is an error worth reporting */
        return ERROR;
    }

    /* Initialize the least squares fit */
    for (ii = 0; ii < 4; ii++)
    {
        /* Initialize all elements of the constants vector C to zero */
        cvec[ii] = 0.0;
    }
    for (jj = 0; jj < 16; jj++)
    {
        /* Initialize all elements of the normal equation matrix to zero */
        norm[jj] = 0.0;
    }

    /* Set the normal equation diagonal term corresponding to the gain
       parameter, N[2][2], to 1/(sig_g^2) */
    norm[10] = 1.0 / sig_g / sig_g;
    /* Set the normal equation diagonal term corresponding to the bias
       parameter, N[3][3], to 1/(sig_b^2) */
    norm[15] = 1.0 / sig_b / sig_b;

    /* Initialize the 4 adjustment parameter values to zero */
    ds = dl = g = b = 0.0;

    for (kk = 0; kk < 3; kk++) /* Do three iterations */
    {
        /* Loop through the samples */
        for (jj = 1; jj < srch_size[1] - 1; jj++)
        {
            /* Loop through the lines */
            for (ii = 1; ii < srch_size[0] - 1; ii++)
            {
                s0 = images[jj*srch_size[0]+ii];  /* Base value */
                sx = (images[jj*srch_size[0]+ii+1] - 
                    images[jj*srch_size[0]+ii-1]) / 2.0; /* Sample term */
                sy = (images[(jj+1)*srch_size[0]+ii] - 
                    images[(jj-1)*srch_size[0]+ii]) / 2.0; /* Line term */
                sxy = (images[(jj+1)*srch_size[0]+ii+1] - 
                    images[(jj+1)*srch_size[0]+ii-1] - 
                    images[(jj-1)*srch_size[0]+ii+1] + 
                    images[(jj-1)*srch_size[0]+ii-1]) / 4.0; /* Cross term */
                dnr = imager[jj*ref_size[0]+ii];  /* Reference value */

                /* only accept imagery that is not zero or negative */
                if (s0 <= 0.0 || dnr <= 0.0)
                {
                    /* don't output an error message since the calling routine
                       has to really figure out if this is an error worth
                       reporting */
                    return ERROR;
                }

                /* The matching model is:
                   sx*ds + sy*dl - dnr*g + b = dnr - s0 - ds*dl*sxy
                   So, for this sample point:
                   [ sx  sy  -dnr  1 ] * X = [ dnr-s0-ds*dl*sxy ]  */

                /* Compute the right hand side of the observation equation
                   using the DN of the reference pixel corresponding to the
                   central search pixel and the current estimates of delta
                   sample and delta line */
                rhs = dnr - s0 - ds * dl * sxy;

                /* Add this observation to the normal equations */
                norm[0]  += sx * sx;
                norm[1]  += sx * sy;
                norm[2]  -= sx * dnr;
                norm[3]  += sx;
                cvec[0]  += sx * rhs;
                norm[5]  += sy * sy;
                norm[6]  -= sy * dnr;
                norm[7]  += sy;
                cvec[1]  += sy * rhs;
                norm[10] += dnr * dnr;
                norm[11] -= dnr;
                cvec[2]  -= dnr * rhs;
                norm[15] += 1.0;
                cvec[3]  += rhs;
            }
        }

        /* Complete the symmetric normal equation matrix */
        norm[4]  = norm[1];   /* N[1][0] = N[0][1] */
        norm[8]  = norm[2];   /* N[2][0] = N[0][2] */
        norm[9]  = norm[6];   /* N[2][1] = N[1][2] */
        norm[12] = norm[3];   /* N[3][0] = N[0][3] */
        norm[13] = norm[7];   /* N[3][1] = N[1][3] */
        norm[14] = norm[11];  /* N[3][2] = N[2][3] */

        /* Solve the normal equations.  First invert the normal equations */
        if (ias_math_invert_matrix(norm, ninv, 4) != SUCCESS)
        {
            /* don't output an error message since the calling routine has to
               really figure out if this is an error worth reporting */
            return ERROR;
        }

        /* Next multiply the inverted normal equations matrix by the 
           constants vector */
        ias_math_multiply_matrix(ninv, cvec, pvec, 4, 4, 4, 1);

        /* Extract the components of the parameter vector */
        ds = pvec[0];
        dl = pvec[1];
        g = pvec[2];
        b = pvec[3];
    }

    /* Return the results of the final iteration */
    fit_offset[0] = ds;
    fit_offset[1] = dl;
    *diag_disp = sqrt(ds * ds + dl * dl);

    return SUCCESS;
}

