#ifndef _IAS_GRID_H_
#define _IAS_GRID_H_

#include "ias_structures.h"
#include "ias_satellite_attributes.h"

#define GRIDNAME_SIZE 256
#define IAS_GRID_ILS_NOT_CALCULATED -3 /* ols2ils could not calculate ILS but
                               did not fail with an error */

typedef struct ias_grid_band_type
{

    int band_index;         /* band index */
    int sca_number;         /* SCA number */

    struct IAS_LNG_LS grid_cell_size;
                          /* Number of lines and samples in each grid cell.
                             The standard values for this will be 80 samples
                             for ms bands and 160 samples for the pan band.
                             The number of lines per grid cell will simply be
                             the number of lines per scan.                   */

    double inv_cell_lines;  /* 1.0/grid_cell_size.line -- an optimization   */
    double inv_cell_samps;  /* 1.0/grid_cell_size.samp -- an optimization   */

    double pixsize;         /* Projection distance per pixel in meters.
                               Note: This will be the same for x and y
                               dimensions.                                  */

    double zspacing;        /* Elevation difference (in meters) between grid
                              elevation planes                               */

    int  lines;             /* Number of lines in the output image           */

    int  samps;             /* Number of samples in the output image         */

    int  ngrid_lines;       /* Number of grid lines through the input image.
                               Note: There are ngrid_lines-1 grid cell rows in
                               each grid.                                    */

    int  ngrid_samps;       /* Number of grid columns through the input image.
                               Note: There are ngrid_samps-1 grid cell columns
                               in each grid.                                 */

    int  nzplanes;          /* Number of elevation planes in the 3D grid     */

    int  zeroplane;         /* Index of the zero elevation plane in the grid */

    int  *in_lines;         /* Pointer to the locations of the horizontal
                               coordinate (line number) on each grid row, going
                               from top to bottom.  The input grid cells are of
                               consistent size and are rectangular, 
                               perpendicular to the line-sample axis of the 
                               input image. Therefore, only intersections need 
                               be given. There are ngrid_lines elements in this
                                element. */

    int  *in_samps;         /* Pointer to the locations of the vertical
                               coordinate (pixel number) on each grid column,
                               going from left to right.  Again, only
                               intersections need be given.  There are
                               ngrid_samps in this element.                  */

    double *out_lines;      /* Pointer to the lines stored in the grid, in
                               output space.  These values are stored in row
                               major format for each grid point.  There
                               are ngrid_lines*ngrid_samps*nzplanes elements
                               in this array.                               */

    double *out_samps;      /* Pointer to the samples stored in the grid, in
                               output space.  These values are stored in row
                               major format for each grid point.  There
                               are ngrid_lines*ngrid_samps*nzplanes elements
                               in this array.                                */

    double *delta_line_oe;  /* Pointer to the odd/even detector line offsets 
                               stored in the grid. These deltas are in input 
                               space and represent the error in the line 
                               direction caused by simply delaying the 
                               odd/even detector samples to compensate for 
                               their forward offset position.  There are 
                               2*ngrid_lines*ngrid_samps in this element. */

    double *delta_samp_oe;  /*  Pointer to the odd/even detector sample offsets
                                stored in the grid. These deltas are in input 
                                space and represent the error in the sample 
                                direction caused by simply delaying the 
                                odd/even detector samples to compensate for 
                                their forward offset position.  There are 
                                2*ngrid_lines*ngrid_samps in this element. */

    struct IAS_COEFFICIENTS *sattoproj;
                            /* Pointers to transformation coefficients for
                               satellite to projection space (forward).
                               There are (ngrid_lines - 1) * (ngrid_samps - 1)
                               * nzplanes grid cells per pointer in sattoproj */

    struct IAS_COEFFICIENTS *projtosat;
                            /* Pointers to transformation coefficients for
                               projection to satellite space (inverse).
                               There are (ngrid_lines - 1) * (ngrid_samps - 1)
                               * nzplanes grid cells per pointer in projtosat */

    int  degree;              /* Degree of the rough transformation polynomial.
                                 This specifies that there are
                                 (degree+1)^2 * nzplanes data elements in the
                                 poly_lines and poly_samps ptrs.             */

    double *poly_lines;       /* Reverse rough transformation coefficients for
                                 the lines.  These are for the grid_cell_poly
                                 routine.  There are (degree+1)^2 * nzplanes
                                 data elements in this pointer.              */

    double *poly_samps;       /* Reverse rough transformation coefficients for
                                 the samples.  These are for the grid_cell_poly
                                 routine.  There are (degree+1)^2 * nzplanes
                                 data elements in this pointer.              */

   IAS_VECTOR *line_sensitivity; /* Input line jitter sensitivity 
                                    coefficients in pixels per radian.  There 
                                    are (ngrid_lines - 1) * (ngrid_samps - 1) 
                                    * nzplanes line sensitivity vectors. */
   IAS_VECTOR *samp_sensitivity; /* Input sample jitter sensitivity 
                                    coefficients in pixels per radian.  There 
                                    are (ngrid_lines - 1) * (ngrid_samps - 1) 
                                    * nzplanes sample sensitivity vectors. */
} IAS_GRID_BAND_TYPE;

typedef struct ias_grid_type
{
    char gridname[GRIDNAME_SIZE]; /* Host file name of the grid file */

    int nbands;             /* Number of bands stored in band_present */

    int maximum_nscas;      /* maximum number of SCAs for any band */

    int bands_available[IAS_MAX_NBANDS]; /* flag to indicate which bands are
                                            available in the grid (may not be
                                            in memory) */
    int bands_present[IAS_MAX_NBANDS]; /* flag to indicate which bands are
                                          present, indexed by the band index */

    int scas_per_band[IAS_MAX_NBANDS]; /* Number of SCAs per band */

    int dtype;              /* data type, about to delete */

    int correction_type;
                            /* This flag specifies if the grid is a systematic,
                             precision, or terrain corrected grid.
                               SYSTEMATIC - Systematic.
                               PRECISION - Precision corrected.
                               TERRAIN   - Terrain corrected.                */

    int  satnum;            /* Number of satellite that acquired the data    */

    int frame_type;         /* Type of framing process to be used.  Valid
                               values are
                               GEOBOX - User specifies UL and LR lat/long
                                 coordinates.
                               PROJBOX - User specifies LR output projection
                                 coordinates.  The user also specifies
                                 another point in output projection space,
                                 along with its coordinating line/sample in
                                 image space.
                               UL_SIZE - User specifies UL output projection
                                 coordinate and the number of lines and
                                 samples in output space.
                               MINBOX - Minbox framing.
                               PATH_ORIENTED - Path-oriented framing.   */     
    int acquisition_type;   /* Acquisition type: Earth, Lunar, or Stellar  */ 

    /* ==== Projection parameters and info. ====                           */
    char units[IAS_UNITS_SIZE]; /* Projection units string */

    int  code;              /* Projection code for the output space image.
                             Values for this field are defined in the "gctp.h"
                             include file.                                   */

    char datum[IAS_DATUM_SIZE]; /* Projection datum string */

    int  spheroid;          /* Projection spheroid code */

    int  zone;              /* Projection zone code for UTM or State Plane
                               projections. */

    double projprms[IAS_PROJ_PARAM_SIZE];
                          /* Array of 15 projection coefficients as required
                             by the projection transformation package.  Refer
                             to the projection package documentation for a
                             description of each field for a given projection.*/

    /* ==== Grid corners. ====                                               */
    struct IAS_CORNERS corners; /* Projection coordinates of the resulting 
                                   output image's four corners.  */

    /* ==== Path-oriented info. ====                                         */
    struct IAS_DBL_LAT_LONG center_pt;
                          /* Lat/long of the WRS scene center point.         */

    IAS_DBL_XY proj_center_pt;  /* Proj x/y of the WRS scene center point.   */

    double rotate_angle;    /* Rotation angle of the path-oriented product.
                             (degrees)                                       */

    double orient_angle;    /* Frame orientation angle in map coords. 
                               (degrees)                                     */

    /* ==== Individual band information. ====                                */
    IAS_GRID_BAND_TYPE **gridbands;
                           /* 2D array of pointers to the band specific grid
                              information built using the 2D array library.
                              The indexes are band and SCA. */

    /* ==== Active area corners information. ====                            */
    IAS_CORNERS active_corners[IAS_MAX_NBANDS];
                           /* Array of active area grid corner information
                              indexed by band index. The corners are specified
                              as latitude,longitude. */ 
} IAS_GRID_TYPE;

/* function prototypes */

int ias_grid_compute_rough_polynomial
( 
   IAS_GRID_BAND_TYPE *grid_band_ptr  /* I/O: Grid band to fill in */
);

int ias_grid_find_cell
( 
    const IAS_GRID_BAND_TYPE *grid_band_ptr, /* I: Grid band to be searched */
    int *row,       /* I/O: Row of grid, input as first guess */
    int *col,       /* I/O: Column of grid, input as first guess */
    double oline,   /* I: Output line */
    double osamp,   /* I: Output sample */
    int iplane      /* I: Elevation plane */
);

int ias_grid_ols2ils
( 
    const IAS_GRID_BAND_TYPE *grid_band_ptr, /* I: pointer to grid band */
    double oline,      /* I: output space line to map to input space */
    double osamp,      /* I: output space sample to map to input space */
    double *iline_ptr, /* O: pointer to input space line */
    double *isamp_ptr, /* O: pointer to input space sample */
    int  iplane        /* I: elevation plane */
);

int ias_grid_3d_ols2ils
( 
    const IAS_GRID_BAND_TYPE *grid_band_ptr, /* I: pointer to grid band */
    double oline,       /* I: output space line to map to input space */
    double osamp,       /* I: output space sample to map to input space */
    double *iline_ptr,  /* O: pointer to input space line */
    double *isamp_ptr,  /* O: pointer to input space sample */
    double elev         /* I: point elevation */
);

int ias_grid_ils2ols_at_elevation
(
    const IAS_GRID_BAND_TYPE *grid_band_ptr, /* I: Grid info for a band */
    double elev,    /* I: Elevation */
    double iline,   /* I: Input space line to map to output space */
    double isamp,   /* I: Input space sample to map to output space */
    double *oline,  /* O: Pointer to output space line */
    double *osamp   /* O: Pointer to output space line */
);

#endif
