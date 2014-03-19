#include <math.h>
#include "ias_logging.h"
#include "ias_math.h"
#include "ias_grid.h"

/******************************************************************************
Name: ias_grid_ols2ils

Purpose: The ias_grid_ols2ils routine maps an output space line/sample back
    into its corresponding input space line/sample.

Outputs:
    The input space line and sample
    
Returns:
    SUCCESS or IAS_GRID_ILS_NOT_CALCULATED

******************************************************************************/
int ias_grid_ols2ils
( 
    const IAS_GRID_BAND_TYPE *grid_band_ptr, /* I: pointer to grid band */
    double oline,      /* I: output space line to map to input space */
    double osamp,      /* I: output space sample to map to input space */
    double *iline_ptr, /* O: pointer to input space line */
    double *isamp_ptr, /* O: pointer to input space sample */
    int  iplane        /* I: elevation plane */
)
{
    int  new_row;    /* new cell row found to contain the output pixel */
    int  new_col;    /* new cell column found to contain the output pixel*/
    int  cell_index; /* cell index of new row/column */
    int  ncols = grid_band_ptr->ngrid_samps - 1; /* columns in grid */
    int  nrows = grid_band_ptr->ngrid_lines - 1; /* columns in grid */
    int  offset;
    double new_line; /* new input line */
    double new_samp; /* new input sample */
    double *poly_samps_coeff;
    double *poly_lines_coeff;
    double *a;       /* projtosat.a coefficients */
    double *b;       /* projtosat.b coefficients */
    double lms;      /* line multiplied by sample cached value */

    /* Make first guess of input pixel for an output pixel using 
       the "rough polynomial". */
    offset = iplane * (grid_band_ptr->degree +1) 
                    * (grid_band_ptr->degree + 1);
    poly_samps_coeff = &grid_band_ptr->poly_samps[offset];
    poly_lines_coeff = &grid_band_ptr->poly_lines[offset];
    new_samp = ias_math_eval_poly_xy(grid_band_ptr->degree, poly_samps_coeff,
                                     osamp, oline);
    new_line = ias_math_eval_poly_xy(grid_band_ptr->degree, poly_lines_coeff,
                                     osamp, oline);

    new_row = (int)(new_line * grid_band_ptr->inv_cell_lines);
    new_col = (int)(new_samp * grid_band_ptr->inv_cell_samps);

    /* We want to find the closest grid cell even if we're outside the grid.
       This can happen when trying to map even/odd detector offsets around the
       edge of the grid. */
    if ((new_row >= 0) && (new_row < nrows) && (new_col >= 0) 
        && (new_col < ncols))
    {
        /* After rough guess call routine to find the correct grid 
           cell associated with the output pixel */
        if (ias_grid_find_cell(grid_band_ptr, &new_row, &new_col, oline, osamp,
                               iplane) == ERROR)
        {
            /* error, so zero out data */
            *isamp_ptr = 0.0;
            *iline_ptr = 0.0;
    
            return IAS_GRID_ILS_NOT_CALCULATED;
        }
        else
        {
            /* Find input pixel location for output pixel location 
               using the inverse mapping coefficients of the proper 
               grid cell */
            lms = oline * osamp;

            cell_index = nrows * ncols * iplane + new_row * ncols + new_col;

            a = grid_band_ptr->projtosat[cell_index].a;
            b = grid_band_ptr->projtosat[cell_index].b;
            *isamp_ptr = a[0] + a[1] * osamp + a[2] * oline + a[3] * lms;
            *iline_ptr = b[0] + b[1] * osamp + b[2] * oline + b[3] * lms;

            return SUCCESS;
        }
    }

    /* Use the edge grid cells */
    if (new_row < 0)
        new_row = 0;
    if (new_col < 0)
        new_col = 0;
    if (new_row >= nrows)
        new_row = nrows-1;
    if (new_col >= ncols)
        new_col = ncols-1;

    /* Find input pixel location for output pixel location using the inverse
       mapping coefficients of the proper grid cell */
    lms = oline * osamp;
   
    cell_index = nrows * ncols * iplane + new_row * ncols + new_col;
   
    a = grid_band_ptr->projtosat[cell_index].a;
    b = grid_band_ptr->projtosat[cell_index].b;
    *isamp_ptr = a[0] + a[1] * osamp + a[2] * oline + a[3] * lms;
    *iline_ptr = b[0] + b[1] * osamp + b[2] * oline + b[3] * lms;
   
    return SUCCESS;
}

/******************************************************************************
Name: ias_grid_3d_ols2ils

Purpose: The ias_grid_3d_ols2ils routine maps an output space
    line/sample/elevation to the corresponding input space line/sample. It uses
    ias_grid_ols2ils on each bounding zplane.

Outputs:
    The input space line and sample.
    
Returns:
    SUCCESS, ERROR, or IAS_GRID_ILS_NOT_CALCULATED

******************************************************************************/
int ias_grid_3d_ols2ils
( 
    const IAS_GRID_BAND_TYPE *grid_band_ptr, /* I: pointer to grid band */
    double oline,       /* I: output space line to map to input space */
    double osamp,       /* I: output space sample to map to input space */
    double *iline_ptr,  /* O: pointer to input space line */
    double *isamp_ptr,  /* O: pointer to input space sample */
    double elev         /* I: point elevation */
)
{
    int zplane;     /* Grid Z-plane index number */
    double iline;   /* Input line */
    double isamp;   /* Input samp */
    double iline0;  /* Input line in first Z-plane */
    double isamp0;  /* Input samp in first Z-plane */
    double iline1;  /* Input line in second Z-plane */
    double isamp1;  /* Input samp in second Z-plane */
    double elev0;   /* First Z-plane elevation */
    double elev1;   /* Second Z-plane elevation */
    double w0, w1;  /* Z-plane interpolation weights */
    int status0;    /* First Z-plane ols2ils return status */
    int status1;    /* Second Z-plane ols2ils return status */

    /* Determine what "Z planes" to use for mapping */
    zplane = (int)floor(elev / grid_band_ptr->zspacing) 
           + grid_band_ptr->zeroplane;
    if (zplane+1 >= grid_band_ptr->nzplanes)
    {
        IAS_LOG_ERROR("Error finding zplane for elevation %f", elev);
        return ERROR;
    }

    /* Find input line/sample for input line/sample for Z plane 1 */
    status0 = ias_grid_ols2ils(grid_band_ptr, oline, osamp, &iline0, &isamp0,
                               zplane);

    /* Find input line/sample for input line/sample for Z plane 2 */
    status1 = ias_grid_ols2ils(grid_band_ptr, oline, osamp, &iline1, &isamp1,
                               (zplane+1));

    if ((status0 == SUCCESS) && (status1 == SUCCESS))
    {
        double inv_elev_diff;

        /* Find elevation that corresponds to planes 1 and 2 */
        elev0 = grid_band_ptr->zspacing * (zplane - grid_band_ptr->zeroplane);
        elev1 = elev0 + grid_band_ptr->zspacing;
        inv_elev_diff = 1.0 / (elev1 - elev0);

        /* Find interpolation weights for point between Z planes 1 and 2 */
        w0 = (elev1 - elev) * inv_elev_diff;
        w1 = (elev - elev0) * inv_elev_diff;

        /* Interpolated input line/sample from between Z planes */
        isamp = isamp0 * w0 + isamp1 * w1;
        iline = iline0 * w0 + iline1 * w1;

        /* return the calculate input line/sample */
        *iline_ptr = iline;
        *isamp_ptr = isamp;
        return SUCCESS;
    }
    else
    {
        *iline_ptr = 0.0;
        *isamp_ptr = 0.0;
        return IAS_GRID_ILS_NOT_CALCULATED;
    }
}
