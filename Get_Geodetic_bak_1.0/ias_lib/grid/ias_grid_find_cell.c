/****************************************************************************
Function: ias_grid_find_cell
 
Purpose:  Find the correct grid cell that contains output line/sample.
 
Returns:
    SUCCESS or ERROR
 
******************************************************************************/
 
#include <math.h>
#include "ias_grid.h"

/*
**
** MACROS
**
*/

#define MAX_ITER 100 
/* defines the maximum number of iterations to try before failing */

#define TOL 1.0e-2

int ias_grid_find_cell
( 
    const IAS_GRID_BAND_TYPE *grid_band_ptr, /* I: Grid band to be searched */
    int *row,       /* I/O: Row of grid, input as first guess */
    int *col,       /* I/O: Column of grid, input as first guess */
    double oline,   /* I: Output line */
    double osamp,   /* I: Output sample */
    int iplane      /* I: Elevation plane */
)
{
    int continue_search = 1; /* continue search as long as not zero */
    int  iter=1;             /* counter of solution iterations */
    int  delta_row;          /* change in row number between the last and 
                                current iteration */
    int  delta_col;          /* change in column number between the last and
                                current iteration */
    int  check_row;          /* previous iteration value of delta_row */ 
    int  check_col;          /* previous iteration value of delta_col */
    double lms;              /* cached value of line * sample */
    double iline,isamp;      /* input line and sample */
    double inv_lines_per_cell = grid_band_ptr->inv_cell_lines;
    double inv_samps_per_cell = grid_band_ptr->inv_cell_samps;
    int  trow;
    int  tcol;
    double mapped_oline;
    double mapped_osamp;

    int new_row;             /* current iteration calculated row */
    int new_col;             /* current iteration calculated column */
    int grid_ncols;          /* number of columns in the grid */
    int grid_nrows;          /* number of rows in the grid */
    int cell_index;          /* Index into cell/zplane */
    double *a;
    double *b;

    /* initialize the history at the first guess */
    trow = *row;
    tcol = *col;

    /* get the number of columns and rows in the grid */
    grid_ncols = grid_band_ptr->ngrid_samps - 1;
    grid_nrows = grid_band_ptr->ngrid_lines - 1;

    /* initialize the check row and column to large values */
    check_row = grid_nrows;
    check_col = grid_ncols;

    /* cache the output line * output sample */
    lms = oline * osamp;

    while (continue_search == 1)
    {
        /* calculate the input line and sample based on the current row
           and column guess */
        cell_index = grid_nrows * grid_ncols * iplane + trow * grid_ncols
                   + tcol;

        a = grid_band_ptr->projtosat[cell_index].a;
        b = grid_band_ptr->projtosat[cell_index].b;
        isamp = a[0] + a[1] * osamp + a[2] * oline + a[3] * lms;
        iline = b[0] + b[1] * osamp + b[2] * oline + b[3] * lms;

        /* convert the input line and sample to grid row and column.  The 
           0.5 is subtracted from the input line to bias the guess to the 
           scan above in case of scan overlap */
        new_row = (int)(iline * inv_lines_per_cell);
        new_col = (int)(isamp * inv_samps_per_cell);

        /* Make sure we haven't mapped out of the grid */
        if (new_row < 0)
            new_row = 0;
        if (new_row >= grid_nrows)
            new_row = grid_nrows - 1;
        if (new_col < 0)
            new_col = 0;
        if (new_col >= grid_ncols)
            new_col = grid_ncols - 1;

        cell_index = grid_nrows * grid_ncols * iplane + new_row * grid_ncols 
            + new_col;

        if ( new_row != trow || new_col != tcol )
        {
           a = grid_band_ptr->projtosat[cell_index].a;
           b = grid_band_ptr->projtosat[cell_index].b;
           isamp = a[0] + a[1] * osamp + a[2] * oline + a[3] * lms;
           iline = b[0] + b[1] * osamp + b[2] * oline + b[3] * lms;
        }

        a = grid_band_ptr->sattoproj[cell_index].a;
        b = grid_band_ptr->sattoproj[cell_index].b;
        mapped_osamp = a[0] + a[1] * isamp + a[2] * iline
                     + a[3] * (isamp * iline);
        mapped_oline = b[0] + b[1] * isamp + b[2] * iline
                     + b[3] * (isamp * iline);

        /* if the new row and column agree with the last row and column, the
           correct grid cell has been found.  Return the grid cell */
        if ((fabs(mapped_oline - oline) <= TOL)
                && (fabs(mapped_osamp - osamp) <= TOL))
        {
            *row = new_row;
            *col = new_col;
            return SUCCESS;
        }
        else
        {
            /* increment the iteration count */
            iter++;

            /* find the change in the row and column between the current and
               last iteration */
            delta_row = new_row - trow;
            delta_col = new_col - tcol;

            trow = new_row;
            tcol = new_col;

            /* if the number of iterations has exceeded the maximum iteration
               count and the solution is not converging, stop looking */
            if (iter > MAX_ITER)
                continue_search = 0;

            if (fabs(delta_row)>fabs(check_row))
                if (fabs(delta_col)>fabs(check_col))
                    continue_search = 0;

            /* save the current row and column change */
            check_row = delta_row;
            check_col = delta_col;
        }
    }

    /* failed to find the correct grid cell.  return an error but do not
       generate an error message since this could be normal */
    return ERROR;
}
