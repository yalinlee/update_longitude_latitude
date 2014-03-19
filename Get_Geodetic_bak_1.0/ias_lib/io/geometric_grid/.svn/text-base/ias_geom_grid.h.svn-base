#ifndef _IAS_GEOM_GRID_H_
#define _IAS_GEOM_GRID_H_

#include "ias_const.h"
#include "ias_grid.h"
#include "ias_types.h"
#include "ias_structures.h"
#include "ias_satellite_attributes.h"

#define IAS_GRIDNAME_SIZE 256

/* types of grids defined */
typedef enum
{
    FORWARD_GEOMETRIC_GRID,
    REVERSE_GEOMETRIC_GRID
} IAS_GEOM_TYPE;

typedef struct ias_geom_grid_band_type
{
    int band_number;        /* band number */

    IAS_GEOM_TYPE gridtype; /* type of grid this defines */

    int cell_lines;         /* lines in each grid cell */
    int cell_samps;         /* samples in each grid cell */

    double pixsize_line;    /* projection distance per pixel in the line 
                               direction (in meters) */
    double pixsize_samp;    /* projection distance per pixel in the sample
                               direction (in meters) */
                             
    int output_image_lines; /* number of lines in the output image */
    int output_image_samps; /* number of samples in the output image */

    int num_in_lines;       /* number of elements in the in_lines array */
    int num_in_samps;       /* number of elements in the in_samps array */

    double *in_lines;       /* pointer to the array of num_in_lines input space
                               lines */
    double *in_samps;       /* pointer to the array of num_in_samps input space
                               samples */

    int num_out_lines;      /* number of elements in the out_lines array */
    int num_out_samps;      /* number of elements in the out_samps array */
  
    double *out_lines;      /* pointer to the array of num_out_lines output 
                               space lines */
    double *out_samps;      /* pointer to the array of num_out_samps output 
                               space samples */

    int forward_present;    /* flag to indicate the forward coefficients are
                               included in the grid */
    int reverse_present;    /* flag to indicate the reverse coefficients are
                               include in the grid */
                             
    int grid_rows;          /* number of rows in the grid */
    int grid_cols;          /* number of columns in the grid */
  
    IAS_COEFFICIENTS *forward_coeffs;
                            /* pointer to the array of grid_rows * grid_cols 
                               coefficients for input space to output space 
                               transformations. */

    IAS_COEFFICIENTS *reverse_coeffs;
                            /* pointer to the array of grid_rows * grid_cols 
                               coefficients for output space to input space 
                               transformations. */

    int degree;             /* Degree of the rough transformation polynomial 
                               for a forward grid. There are (degree+1)^2 data
                               elements in the poly_lines and poly_samps arrays.
                               */

    double *poly_lines;     /* Reverse rough transformation coefficients for
                               the lines.  These are for the grid_cell_poly
                               routine.  There are (degree+1)^2 data elements
                               in this pointer. */

    double *poly_samps;     /* Reverse rough transformation coefficients for
                               the samples.  These are for the grid_cell_poly
                               routine.  There are (degree+1)^2 data elements
                               in this pointer. */

} IAS_GEOM_GRID_BAND_TYPE;

typedef struct ias_geom_grid_type
{
    char gridname[IAS_GRIDNAME_SIZE];  /* host file name of the grid file */

    IAS_GEOM_TYPE gridtype;            /* type of grid                    */

    int nbands;                        /* number of bands in the grid     */

    int band_index_list[IAS_MAX_NBANDS];      
                                       /* list of valid bands in the grid */

    IAS_FRAME_TYPE frame_type;  /* Enumerated frame_type value, will be 
                                   converted to string value geom_grid_record
                                   structure */

    char units[IAS_UNITS_SIZE]; /* Projection units string */

    int code;                   /* Projection code for the output space image.
                                   Values for this field are defined in the 
                                   "gctp.h" file.                           */

    char datum[IAS_DATUM_SIZE]; /* Projection datum code string             */

    int spheroid;               /* Projection spheroid code                 */

    int zone;                   /* Projection zone code for UTM or State Plane
                                   projections.  */

    double projprms[IAS_PROJ_PARAM_SIZE];
                                /* Array of 15 projection coefficients as 
                                   required by the projection transformation 
                                   package. Refer to the projection package 
                                   documentation for a description of each 
                                   field for a given projection.*/

    IAS_CORNERS corners;        /* Projection coordinates of the resulting 
                                   output image's four corners. */

    IAS_DBL_LAT_LONG center_pt;
                                /* Lat/long of the scene center point. */

    IAS_DBL_XY proj_center_pt;  /* Proj x/y of the scene center point. */

    double rotate_angle;       /* Rotation angle of the path-oriented product.
                                  (degrees)                                  */

    double orient_angle;        /* Frame orientation angle in map coords. 
                                   (degrees)                               */

    IAS_GEOM_GRID_BAND_TYPE *gridbands;
                                /* Array of specific grid band information */
                             
} IAS_GEOM_GRID_TYPE;


/* PROTOTYPES */
int ias_geom_grid_initialize 
(
    const int nbands,         /* I: number of bands in the gemetric grid */
    IAS_GEOM_GRID_TYPE *grid  /* I/O: Grid structure to initialize       */
);

int ias_write_geom_grid 
(
    const char *grid_filename,       /* I: Grid input file name             */
    const IAS_GEOM_GRID_TYPE *grid   /* I: Grid structure to be output     */
);

int ias_read_geom_grid 
(
    const char *grid_filename, /* I: Grid input file name                 */
    const int *band_number,    /* I/O: Band number to read from the grid */
    int *nbands,               /* I: Number of bands in the bands array   */
    IAS_GEOM_GRID_TYPE *grid   /* O: Grid structure populated from file   */
);

int ias_read_geom_grid_header 
(
    const char *grid_filename, /* I: Grid file name*/
    IAS_GEOM_GRID_TYPE *grid   /* O: Grid structure populated from file     */
);

int ias_read_geom_grid_band_header 
(
    const char *grid_filename, /* I: Grid file name*/
    IAS_GEOM_GRID_TYPE *grid   /* O: Grid structure populated from file     */
);

int ias_read_geom_grid_band_pointers 
(
  const char *grid_filename, /* I: Grid input file name                     */
  const int *band_number,    /* I/O: Band number to read from the grid */
  int *nbands,               /* I: Number of bands in the bands array       */
  IAS_GEOM_GRID_TYPE *grid   /* O: Grid structure populated from file       */
);

int ias_geom_grid_malloc 
(
  const int *band_number,    /* I/O: Array of bands to free from the grid   */
  int *nbands,               /* I/O: Number of bands in the bands array     */
  IAS_GEOM_GRID_TYPE *grid   /* I/O: Grid structure to allocate memory for  */
);

int ias_geom_grid_free 
(
  const int *band_number,    /* I/O: Array of bands to free from the grid   */
  int *nbands,               /* I/O: Number of bands in the bands array     */
  IAS_GEOM_GRID_TYPE *grid   /* I/O: Grid structure to be freed             */
);

int ias_geom_grid_band_free 
(
    IAS_GEOM_GRID_TYPE *grid, /* I/O: Grid structure in which band pointers
                                     to be freed             */
    const int band_number     /* I/O: Array of bands to free from the grid   */
);

int ias_grid_is_geom_grid_file
(
    const char *grid_filename /* I: Grid input file name                    */
);

#endif
