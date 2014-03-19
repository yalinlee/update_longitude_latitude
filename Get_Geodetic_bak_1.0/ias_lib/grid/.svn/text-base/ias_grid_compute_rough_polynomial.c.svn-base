/****************************************************************************
Function: ias_grid_compute_rough_polynomial
 
Purpose:  Calculate a "rough" mapping of output to input lines/samples and
    populate the grid with the information.
 
Returns:
    ERROR or SUCCESS

NOTES:
    The coefficients computed are used as a rough estimate of an inverse model.
    This poynomial is used to initially locate the correct grid cell to be 
    used in mapping from output line/sample to input line/sample.
*******************************************************************************/
#include <stdlib.h>
#include <math.h>
#include "ias_logging.h"
#include "ias_math.h"
#include "ias_grid.h"

int ias_grid_compute_rough_polynomial
( 
   IAS_GRID_BAND_TYPE *grid_band_ptr  /* I/O: Grid band to fill in */
)
{
    double *b;           /* Array for least squares */
    double *v;           /* Array for least squares */
    double *A;           /* Array for least squares */
    int i,n,m;           /* Counters */
    int numpts;          /* Number of points used in least squares */
    int cnt;             /* Counter for least squares */
    int ncoeff;          /* Number of coefficients used in least squares */
    int ncols;           /* Number of columns */
    int nrows;           /* Number of rows */
    int pdegree;         /* Degree of the rough transformatioanal coeffs */
    int qflag = 0;       /* flag for c_qrdecomp do not order A by columns */
    int zplane;          /* elevation plane loop counter */
    int nzplanes;        /* number of elevation planes */

    /* extract some information from the grid */
    pdegree = grid_band_ptr->degree;
    ncoeff = (pdegree + 1) * (pdegree + 1);
    nrows = grid_band_ptr->ngrid_lines;
    ncols = grid_band_ptr->ngrid_samps;
    nzplanes = grid_band_ptr->nzplanes;
    numpts = ncols * nrows;

    /* allocate buffer space for least squares routine */
    A = (double *)malloc(sizeof(double) * numpts * ncoeff);
    if (A == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        return ERROR;
    }
    b = (double *)malloc(sizeof(double) * numpts);
    if (b == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        free(A);
        return ERROR;
    }
    v = (double *)malloc(sizeof(double) * numpts);
    if (v == NULL)
    {
        IAS_LOG_ERROR("Error allocating memory");
        free(A);
        free(b);
        return ERROR;
    }

    /* loop over the planes present in the grid to calculate the rough 
       polynomial for each elevation plane */
    for (zplane = 0; zplane < nzplanes; zplane++)
    {
        int zoffset = zplane * numpts;

        double *osamps = &grid_band_ptr->out_samps[zoffset];
        double *olines = &grid_band_ptr->out_lines[zoffset];
        int *ilines = grid_band_ptr->in_lines;
        int *isamps = grid_band_ptr->in_samps;

        /* Fill least squares routine according to degree of polynomial.  This
           mapping is from output image lines/samples (all lines/samples in
           grid output space) to input image lines/samples (all lines/samples
           in grid input space). */
        cnt = 0;
        for (i = 0; i <= pdegree; i++)
        {
            for (m = 0; m <= pdegree; m++)
            {
                for (n = 0; n < numpts; n++)
                {
                    double samp = osamps[n];
                    double line = olines[n];
                    double x = pow(samp,m);
                    double y = pow(line,i);
                    A[cnt] = x*y;
                    cnt++;
                }
            }
        }

        /* Perform least squares routine.  First the coefficients for output
           lines to input lines are found and stored.  Next the coefficients
           for output samples to input samples are found and stored. */

        ias_math_matrix_QRfactorization(A, numpts, ncoeff, v, qflag);

        for (i = 0, m = 0; m < nrows; m++)
            for (n = 0; n < ncols; n++, i++)
                b[i] = ilines[m];

        ias_math_matrix_QRsolve(A, numpts, ncoeff, v, b, 0);

        for (n = 0; n < ncoeff; n++)
            grid_band_ptr->poly_lines[n+zplane*ncoeff] = b[n];

        for (i = 0, m = 0; m < nrows; m++)
            for (n = 0; n < ncols; n++, i++)
                b[i] = isamps[n];

        ias_math_matrix_QRsolve(A, numpts, ncoeff, v, b, 0);

        for (n = 0; n < ncoeff; n++)
            grid_band_ptr->poly_samps[n+zplane*ncoeff] = b[n];
    }

    free(A);
    free(b);
    free(v);

    return SUCCESS;
}
