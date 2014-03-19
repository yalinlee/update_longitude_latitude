/****************************************************************************
Name: ias_grid_ils2ols_at_elevation

Purpose: Finds the output line and sample from the grid.  This is done by 
adding an incremental distance, dx and dy, to the input line and sample and 
finding the corresponding output line and sample.

RETURN VALUE:
Type = int
Value   Description
-----   -----------
SUCCESS Successful calculation of the output line and sample from grid
ERROR   Operation failed
*******************************************************************************/
#include <math.h>

#include "ias_logging.h" /* IAS logging library */
#include "ias_grid.h" /* IAS grid structure */

int ias_grid_ils2ols_at_elevation
(
    const IAS_GRID_BAND_TYPE *grid_band_ptr, /* I: Grid info for a band */
    double elev,    /* I: Elevation */
    double iline,   /* I: Input space line to map to output space */
    double isamp,   /* I: Input space sample to map to output space */
    double *oline,  /* O: Pointer to output space line */
    double *osamp   /* O: Pointer to output space line */
)
{
    int r;          /* Row, calculated */
    int c;          /* Column, calculated */
    double *a;      /* Projection to satellite coefficients from the grid */
    double *b;      /* Projection to satellite coefficients from the grid */
    double lms;     /* Line multiplied by sample */
    int zplane;     /* Grid Z-plane index number */
    double oline0;  /* Input line in first Z-plane */
    double osamp0;  /* Input samp in first Z-plane */
    double oline1;  /* Input line in second Z-plane */
    double osamp1;  /* Input samp in second Z-plane */
    double elev0;   /* First Z-plane elevation */
    double elev1;   /* Second Z-plane elevation */
    double w0;      /* Z-plane interpolation weights */
    double w1;      /* Z-plane interpolation weights */
    double inv_elev_diff; /* Inverted elevation difference */
    int ncols = grid_band_ptr->ngrid_samps - 1; /* Columns in grid */
    int nrows = grid_band_ptr->ngrid_lines - 1; /* Columns in grid */
    int cell_index; /* Cell index of new row/column */

    /* Determine what "Z planes" to use for mapping */
    zplane = (int)floor(elev / grid_band_ptr->zspacing) + 
        grid_band_ptr->zeroplane;
    if (zplane + 1 >= grid_band_ptr->nzplanes)
    {
        IAS_LOG_ERROR("Zplane %d + 1 is >= grid number of zplanes %d",
            zplane, grid_band_ptr->nzplanes);
        return ERROR;
    }

    /* Find elevation that corresponds to planes 1 and 2 */
    elev0 = grid_band_ptr->zspacing * (zplane - grid_band_ptr->zeroplane);
    elev1 = elev0 + grid_band_ptr->zspacing;
    inv_elev_diff = 1.0 / (elev1 - elev0);

    /* Row and column in the grid */
    r = (int)(iline / grid_band_ptr->grid_cell_size.line);
    c = (int)(isamp / grid_band_ptr->grid_cell_size.samp);

    /* Calculate for use with coeffs from grid */
    lms = iline * isamp;

    /* Find the output line and sample. Currently forward mapping coefficients
       are calculated.  The model could also be called to give the output 
       line/sample location.  This would be more accurate */
    cell_index = nrows * ncols * zplane + r * ncols + c;
    a = &grid_band_ptr->sattoproj[cell_index].a[0];
    osamp0 = a[0] + a[1] * isamp + a[2] * iline + a[3] * lms;

    b = &grid_band_ptr->sattoproj[cell_index].b[0];
    oline0 = b[0] + b[1] * isamp + b[2] * iline + b[3] * lms;

    cell_index = nrows * ncols * (zplane+1) + r * ncols + c;
    a = &grid_band_ptr->sattoproj[cell_index].a[0];
    osamp1 = a[0] + a[1] * isamp + a[2] * iline + a[3] * lms;

    b = &grid_band_ptr->sattoproj[cell_index].b[0];
    oline1 = b[0] + b[1] * isamp + b[2] * iline + b[3] * lms;

    /* Find interpolation weights for point between Z planes 1 and 2 */
    w0 = (elev1 - elev) * inv_elev_diff;
    w1 = (elev - elev0) * inv_elev_diff;

    /* Interpolated input line/sample from between Z planes */
    *osamp = osamp0 * w0 + osamp1 * w1;
    *oline = oline0 * w0 + oline1 * w1;

    return SUCCESS;
}
