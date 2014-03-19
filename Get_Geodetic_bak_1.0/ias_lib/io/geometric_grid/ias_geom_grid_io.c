/*************************************************************************

NAME: ias_geom_grid_io.c

PURPOSE: Implements the geometric grid input/output library for applications

Algorithm References: None

**************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <hdf5.h>
#include <hdf5_hl.h>
#include "gctp.h"                     /* Header for SOM value */
#include "ias_logging.h"              /* Header for logging library */
#include "ias_const.h"                /* Header for some constants */
#include "ias_geom_grid.h"            /* Function prototypes and structures */
#include "ias_satellite_attributes.h" /* Header for band conversion */
#include "ias_miscellaneous.h"        /* Header for file type checking */

#define GRID_FORMAT_VERSION 1
#define GRID_FIELDS 25                /* Number of fileds in the 
                                         GEOM_GRID_RECORD structure */
#define BAND_FIELDS 17                /* Number of fields in the 
                                         GEOM_GRID_BAND_RECORD structure */
#define FILE_TYPE "Geom Grid File" 
#define GEOM_GRID_TABLE_TITLE "Geom Grid" 
#define GEOM_GRID_TABLE_NAME "Geom Grid Record" 
#define GEOM_GRID_BAND_TABLE_TITLE "Geom Grid Band" 
#define GEOM_GRID_BAND_TABLE_NAME "Geom Grid Band Record" 

#define GEOBOX_FRAME "GEOBOX"
#define PROJBOX_FRAME "PROJBOX"
#define UL_SIZE_FRAME "UL_SIZE"
#define MINBOX_FRAME "MINBOX"
#define MAXBOX_FRAME "MAXBOX"
#define PATH_ORIENTED_FRAME "PATH_ORIENTED"
#define PATH_MINBOX_FRAME "PATH_MINBOX"
#define PATH_MAXBOX_FRAME "PATH_MAXBOX"
 
#define LENGTH 25

/*************************************************************************
The GEOM_GRID_BAND_RECORD structure is needed because it is used to build up 
the non-pointer grid band data into a HDF5 grid band data table.
**************************************************************************/
typedef struct geom_grid_band_record
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

    int num_out_lines;      /* number of elements in the out_lines array */
    int num_out_samps;      /* number of elements in the out_samps array */
  
    int forward_present;    /* flag to indicate the forward coefficients are
                               included in the grid */
    int reverse_present;    /* flag to indicate the reverse coefficients are
                               include in the grid */
                             
    int grid_rows;          /* number of rows in the grid */
    int grid_cols;          /* number of columns in the grid */
  
    int degree;             /* Degree of the rough transformation polynomial 
                               for a forward grid. There are (degree+1)^2 data
                               elements in the poly_lines and poly_samps arrays.
                               */
} GEOM_GRID_BAND_RECORD;

typedef struct geom_grid_record
{
    char gridname[IAS_GRIDNAME_SIZE];  /* host file name of the grid file */

    IAS_GEOM_TYPE gridtype; /* type of grid */

    int nbands;             /* number of bands stored in the grid */

    int band_index_list[IAS_MAX_NBANDS];
                            /* list of valid bands in the grid */

    char frame_type[LENGTH];  /* Type of framing process to be used.  Valid
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
                               PATH_ORIENTED - Path-oriented framing.        */

    char units[IAS_UNITS_SIZE];/* Projection units string                    */

    int code;                  /* Projection code for the output space image.
                                  Values for this field are defined in the 
                                  "gctp.h" include file.                     */

    char datum[IAS_DATUM_SIZE];/* Projection datum code string               */

    int spheroid;              /* Projection spheroid code                   */

    int zone;                  /* Projection zone code for UTM or State Plane
                                  projections.                               */

    double projprms[IAS_PROJ_PARAM_SIZE];
                               /* Array of 15 projection coefficients as 
                                  required by the projection transformation 
                                  package. Refer to the projection package 
                                  documentation for a description of each field
                                  for a given projection.                    */

    IAS_CORNERS corners;       /* Projection coordinates of the resulting 
                                  output image's four corners.               */

    IAS_DBL_LAT_LONG center_pt;
                               /* Lat/long of the scene center point.    */

    IAS_DBL_XY proj_center_pt; /* Proj x/y of the scene center point.    */

    double rotate_angle;       /* Rotation angle of the path-oriented product.
                                  (degrees)                                  */

    double orient_angle;       /* Frame orientation angle in map coords. 
                                  (degrees)                                  */

} GEOM_GRID_RECORD;

/*************************************************************************

NAME: ias_geom_grid_initialize

PURPOSE: Initial Grid I/O library by dynamic allocating memory for gridbands 

RETURNS: SUCCESS / ERROR

**************************************************************************/
int ias_geom_grid_initialize 
(
    const int nbands,         /* I: number of bands in the gemetric grid */
    IAS_GEOM_GRID_TYPE *grid  /* I/O: Grid structure to initialize       */
)
{

    memset(grid, 0, sizeof(IAS_GEOM_GRID_TYPE));

    grid->gridbands = calloc(nbands, sizeof(IAS_GEOM_GRID_BAND_TYPE));
    if (grid->gridbands == NULL)
    {
        IAS_LOG_ERROR("Allocating geometric grid band type structure array");
        return ERROR;
    }

    //XQian 2013.06.20 for ias_geom_grid_free
//    grid->nbands = nbands;

    return SUCCESS;
}

/*************************************************************************

NAME: flip_SOM_corners_for_read

PURPOSE: Flip the corner points for the SOM projection, this routine allows
         a constant user interface.

RETURNS: None

**************************************************************************/
static void flip_SOM_corners_for_read 
(
    GEOM_GRID_RECORD *grid  /* I/O: Geometric Grid structure to be output   */
)
{
    /* Flip the corner points for the SOM projection ONLY */
    if (grid->code == SOM)
    {
        double tmp;      /* Temporary variable for flipping corner points*/
        tmp = -grid->corners.upleft.x;
        grid->corners.upleft.x = grid->corners.upleft.y;
        grid->corners.upleft.y = tmp;

        tmp = -grid->corners.loleft.x;
        grid->corners.loleft.x = grid->corners.loleft.y;
        grid->corners.loleft.y = tmp;

        tmp = -grid->corners.upright.x;
        grid->corners.upright.x = grid->corners.upright.y;
        grid->corners.upright.y = tmp;

        tmp = -grid->corners.loright.x;
        grid->corners.loright.x = grid->corners.loright.y;
        grid->corners.loright.y = tmp;
    }
}

/*************************************************************************

NAME: flip_SOM_corners_for_write

PURPOSE: Flip the corner points for the SOM projection, this routine allows
         a constant user interface.

RETURNS: None

**************************************************************************/
static void flip_SOM_corners_for_write 
(
    GEOM_GRID_RECORD *grid  /* I/O: Geometric Grid structure to be output   */
)
{
    /*  Flip the corner points for the SOM projection ONLY */
    if (grid->code == SOM)
    {
        double tmp;      /* Temporary variable for flipping corner points*/

        tmp = -grid->corners.upleft.y;
        grid->corners.upleft.y = grid->corners.upleft.x;
        grid->corners.upleft.x = tmp;

        tmp = -grid->corners.loleft.y;
        grid->corners.loleft.y = grid->corners.loleft.x;
        grid->corners.loleft.x = tmp;

        tmp = -grid->corners.upright.y;
        grid->corners.upright.y = grid->corners.upright.x;
        grid->corners.upright.x = tmp;

        tmp = -grid->corners.loright.y;
        grid->corners.loright.y = grid->corners.loright.x;
        grid->corners.loright.x = tmp;
    }
}

/******************************************************************************
Name: cleanup_grid_table_description

Purpose: Cleans up the resources allocated to the table description

Returns: None
    
******************************************************************************/
static void cleanup_grid_table_description
(
    hid_t fields_to_close[GRID_FIELDS]  /* I: array of fields to close */
)
{
    int i;

    /* close any types created for the table description */
    for (i = 0; i < GRID_FIELDS; i++)
    {
        if (fields_to_close[i] >= 0)
            H5Tclose(fields_to_close[i]);
    }
}

/******************************************************************************
Name: cleanup_band_table_description

Purpose: Cleans up the resources allocated to the table description

Returns: None
    
******************************************************************************/
static void cleanup_band_table_description
(
    hid_t fields_to_close[BAND_FIELDS]  /* I: array of fields to close */
)
{
    int i;

    /* close any types created for the table description */
    for (i = 0; i < BAND_FIELDS; i++)
    {
        if (fields_to_close[i] >= 0)
            H5Tclose(fields_to_close[i]);
    }
}

/*************************************************************************

NAME: build_grid_band_record_description

PURPOSE: Build geometric grid band data table

RETURNS: SUCCESS -- successfully build grid band record table
         ERROR -- error in building grid band record table
         EXIT_FAILURE -- The stack is probably corrupt so just exit
**************************************************************************/
static int build_grid_band_record_description
(
    size_t offsets[BAND_FIELDS],          /* O: Offsets into the structure */
    const char *field_names[BAND_FIELDS], /* O: Names of the fileds */
    hid_t field_type[BAND_FIELDS],        /* O: Data types of the fields */
    hid_t fields_to_close[BAND_FIELDS],   /* O: Fields to be closed */
    size_t field_size[BAND_FIELDS]        /* O: Sizes of the fields */
)
{
    int i;
    int error = 0;
    GEOM_GRID_BAND_RECORD gbr;     /* for field sizes */

    for (i = 0; i < BAND_FIELDS; i++)
        fields_to_close[i] = -1;

    /* Build up field for HDF5 band record table */
    i = 0;
    offsets[i] = HOFFSET(GEOM_GRID_BAND_RECORD, band_number);
    field_names[i] = "Band Number";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.band_number);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid band table definition: Band Number");
        cleanup_band_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_BAND_RECORD, gridtype);
    field_names[i] = "Grid Type";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.gridtype);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid band table definition: Grid Type");
        cleanup_band_table_description(fields_to_close);
        return ERROR;
    }
    
    offsets[++i] = HOFFSET(GEOM_GRID_BAND_RECORD, cell_lines);
    field_names[i] = "Lines in a Cell";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.cell_lines);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid band table definition: Lines in a Cell");
        cleanup_band_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_BAND_RECORD, cell_samps);
    field_names[i] = "Samps in a Cell";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.cell_samps);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid band table definition: Samps in a Cell");
        cleanup_band_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_BAND_RECORD, pixsize_line);
    field_names[i] = "Pixel Size Along Line";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gbr.pixsize_line);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid band table definition: "
                      "Pixel Size Along Line");
        cleanup_band_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_BAND_RECORD, pixsize_samp);
    field_names[i] = "Pixel Size Along Samp";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gbr.pixsize_samp);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid band table definition: "
                      "Pixel Size Along Samp");
        cleanup_band_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_BAND_RECORD, output_image_lines);
    field_names[i] = "Lines in Output Image";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.output_image_lines);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid band table definition: "
                      "Lines in Output Image");
        cleanup_band_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_BAND_RECORD, output_image_samps);
    field_names[i] = "Samps in Output Image";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.output_image_samps);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid band table definition: "
                      "Samps in Output Image");
        cleanup_band_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_BAND_RECORD, num_in_lines);
    field_names[i] = "Input Lines";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.num_in_lines);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid band table definition: Input Lines");
        cleanup_band_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_BAND_RECORD, num_in_samps);
    field_names[i] = "Input Samples";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.num_in_samps);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid band table definition: Input Samps");
        cleanup_band_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_BAND_RECORD, num_out_lines);
    field_names[i] = "Output Lines";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.num_out_lines);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid band table definition: Output Lines");
        cleanup_band_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_BAND_RECORD, num_out_samps);
    field_names[i] = "Output Samples";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.num_out_samps);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid band table definition: Output Samps");
        cleanup_band_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_BAND_RECORD, forward_present);
    field_names[i] = "Forward Coefficients Included";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.forward_present);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid band table definition: "
                      "Forward Coefficients Included");
        cleanup_band_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_BAND_RECORD, reverse_present);
    field_names[i] = "Reverse Coefficients Included";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.reverse_present);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid band table definition: "
                      "Reverse Coefficients Included");
        cleanup_band_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_BAND_RECORD, grid_rows);
    field_names[i] = "Rows in Grid";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.grid_rows);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid band table definition: Rows in Grid");
        cleanup_band_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_BAND_RECORD, grid_cols);
    field_names[i] = "Columns in Grid";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.grid_cols);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid band table definition: Columns in Grid");
        cleanup_band_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_BAND_RECORD, degree);
    field_names[i] = "Polynomial Degree";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.degree);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid band table definition: Polynomial Degree");
        cleanup_band_table_description(fields_to_close);
        return ERROR;
    }

    if (i < (BAND_FIELDS - 1))
    {
        /* fewer fields were added than expected */
        IAS_LOG_ERROR("Defined %d fields, but expected %d", i + 1, BAND_FIELDS);
        cleanup_band_table_description(fields_to_close);
        return ERROR;
    }
    else if (i >= BAND_FIELDS)
    {
        /* more fields than expected.  The stack is probably now corrupt so
           just exit since this is an obvious programming error that was just
           introduced. */
        IAS_LOG_ERROR("Error: too many fields found - stack probably "
                      "corrupted - exiting");
        exit(EXIT_FAILURE);
    }

    return SUCCESS;
}

/*************************************************************************

NAME: build_grid_record_description

PURPOSE: Build geometric grid data table

RETURNS: SUCCESS -- successfully build grid record table
         ERROR -- error in building grid record table
         EXIT_FAILURE -- The stack is probably corrupt so just exit
**************************************************************************/
static int build_grid_record_description
(
    size_t offsets[GRID_FIELDS],          /* O: Offsets into the structure */
    const char *field_names[GRID_FIELDS], /* O: Names of the fileds */
    hid_t field_type[GRID_FIELDS],        /* O: Data types of the fields */
    hid_t fields_to_close[GRID_FIELDS],   /* O: Fields to be closed */
    size_t field_size[GRID_FIELDS]        /* O: Sizes of the fields */
)
{
    hsize_t parm_dims[1];   /* for data array */    
    int i;
    int error = 0;
    GEOM_GRID_RECORD gt;     /* for field sizes */

    for (i = 0; i < GRID_FIELDS; i++)
        fields_to_close[i] = -1;

    /* Build up fields for HDF5 grid record table */
    i = 0;
    offsets[i] = HOFFSET(GEOM_GRID_RECORD, gridname);
    field_names[i] = "Host Grid Filename";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(gt.gridname)) < 0)
        error = 1;
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(gt.gridname);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: "
                      "Host Grid Filename");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, gridtype);
    field_names[i] = "SCAs Per Band";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gt.gridtype);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: SCAs Per Band");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, nbands);
    field_names[i] = "Number of Bands";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gt.nbands);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: Number of Bands");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, band_index_list);
    field_names[i] = "Band List";
    parm_dims[0]=sizeof(gt.band_index_list)/sizeof(int);
    field_type[i] =   H5Tarray_create(H5T_NATIVE_INT, 1, parm_dims);
    if (field_type[i] < 0)
        error =1;
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(gt.band_index_list);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: Band List");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, frame_type);
    field_names[i] = "Frame Type";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(gt.frame_type)) < 0)
        error = 1;
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(gt.frame_type);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: Frame Type");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, units);
    field_names[i] = "Projection Units";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(gt.units)) < 0)
        error = 1;
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(gt.units);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: Projection Units");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, code);
    field_names[i] = "Projection Codes";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gt.code);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: Projection Codes");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, datum);
    field_names[i] = "Projection Datum Codes";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(gt.datum)) < 0)
        error = 1;
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(gt.datum);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: "
                      "Projection Datum Codes");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, spheroid);
    field_names[i] = "Projection Spheroid Codes";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gt.code);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: "
                      "Projection Spheroid Codes");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, zone);
    field_names[i] = "Projection Zone Code";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gt.zone);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: "
                      "Projection Zone Code");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, projprms);
    field_names[i] = "Array of 15 Projection Coeffs";
    parm_dims[0]=sizeof(gt.projprms)/sizeof(double);
    field_type[i] =   H5Tarray_create(H5T_NATIVE_DOUBLE, 1, parm_dims);
    if (field_type[i] < 0)
        error =1;
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(gt.projprms);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: "
                      "Array of 15 Projection Coeffs");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, corners.upleft.x);
    field_names[i] = "Upper Left X value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.corners.upleft.x);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: "
                      "Upper Left X value");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, corners.upleft.y);
    field_names[i] = "Upper Left Y value";
    field_type[i] =  H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.corners.upleft.y);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: "
                      "Upper Left Y value");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, corners.upright.x);
    field_names[i] = "Upper Right X value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.corners.upright.x);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: "
                      "Upper Right X value");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, corners.upright.y);
    field_names[i] = "Upper Right Y value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.corners.upright.y);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: "
                      "Upper Right Y value");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, corners.loleft.x);
    field_names[i] = "Lower Left X value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.corners.loleft.x);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: "
                      "Lower Left X value");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, corners.loleft.y);
    field_names[i] = "Lower Left Y value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.corners.loleft.y);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: "
                      "Lower Left Y value");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, corners.loright.x);
    field_names[i] = "Lower Right X value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.corners.loright.x);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: "
                      "Lower Right X value");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, corners.loright.y);
    field_names[i] = "Lower Right Y value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.corners.loright.y);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: "
                      "Lower Right Y value");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, center_pt.lat);
    field_names[i] = "Scene Center Lat";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.center_pt.lat);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: Scene Center Lat");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, center_pt.lng);
    field_names[i] = "Scene Center Long";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.center_pt.lng);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: "
                      "Scene Center Long");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, proj_center_pt.x);
    field_names[i] = "Scene Center Proj X";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.proj_center_pt.x);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Error building grid table definition: "
                      "Scene Center Proj X");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, proj_center_pt.y);
    field_names[i] = "Scene Center Proj Y";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.proj_center_pt.y);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid table definition: "
                      "Scene Center Proj Y");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, rotate_angle);
    field_names[i] = "Rotation Angle";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.rotate_angle);

    offsets[++i] = HOFFSET(GEOM_GRID_RECORD, orient_angle);
    field_names[i] = "Frame Orientation Angle";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.orient_angle);
    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid table definition: "
                      "Frame Orientation Angle");
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }

    if (i < (GRID_FIELDS - 1))
    {
        /* fewer fields were added than expected */
        IAS_LOG_ERROR("Defined %d fields, but expected %d", i + 1, GRID_FIELDS);
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }
    else if (i >= GRID_FIELDS)
    {
        /* more fields than expected.  The stack is probably now corrupt so
           just exit since this is an obvious programming error that was just
           introduced. */
        IAS_LOG_ERROR("Error: too many fields found - stack probably "
                      "corrupted - exiting");
        exit(EXIT_FAILURE);
    }

    return SUCCESS;
}

/*************************************************************************

NAME: ias_read_geom_grid_header

PURPOSE: Read out geometric grid data from HDF5 header

RETURNS: SUCCESS -- successfully read grid header
         ERROR -- error in reading grid header

**************************************************************************/
int ias_read_geom_grid_header
(
    const char *grid_filename, /* I: Grid file name         */
    IAS_GEOM_GRID_TYPE *grid   /* O: Grid structure to read */
)
{
    hid_t file_id;                        /* File handle for HDF */
    size_t offsets[GRID_FIELDS];          /* Field offsets       */
    const char *field_names[GRID_FIELDS]; /* Name of the fields  */
    hid_t field_type[GRID_FIELDS];        /* Datatype of the fields */
    size_t field_size[GRID_FIELDS];       /* Size of the fields */
    hid_t fields_to_close[GRID_FIELDS];   /* Fields to close  */
    int status;                           /* Return status of the description 
                                             build routines               */
    size_t total_size;                    /* Total size of the table      */
    GEOM_GRID_RECORD grid_data;           /* Subset of the GEOM model structure
                                             containing members that go into a 
                                             general table.               */
    hsize_t number_of_fields;             /* Number of fields in the table */
    hsize_t number_of_records;            /* Number of records in the table */
    int i;
    int grid_format_version = GRID_FORMAT_VERSION;

    /* Open the input file */
    file_id =  H5Fopen(grid_filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file_id < 0)
    {
        IAS_LOG_ERROR("Opening grid file: %s", grid_filename);
        return ERROR;
    }

    /* Get the grid format attribute, if it isn't the correct one it can't
    be read */
    if (H5LTget_attribute_int(file_id, "/", "Grid Format Version",
            &grid_format_version) < 0)
    {
        IAS_LOG_ERROR("Could not get grid format version: %s", grid_filename);
        H5Fclose(file_id);
        return ERROR;
    }
    if (grid_format_version != GRID_FORMAT_VERSION)
    {
        IAS_LOG_ERROR("Incorrect grid format version: %s", grid_filename);
        H5Fclose(file_id);
        return ERROR;
    }

    /* Get the grid table information */
    status = H5TBget_table_info(file_id, GEOM_GRID_TABLE_NAME,
        &number_of_fields, &number_of_records);
    if (status < 0)
    {
        IAS_LOG_ERROR("Could not get grid table information");
        H5Fclose(file_id);
        return ERROR;
    }

    /* Check to make sure only one data record for the table since it has
       only one grid record */
    if (number_of_records != 1)
    {
        IAS_LOG_ERROR("Incorrect number of records");
        H5Fclose(file_id);
        return ERROR;
    }

    /* Build the grid record table */
    if (build_grid_record_description(offsets, field_names, field_type,
                fields_to_close, field_size) == ERROR)
    {
        IAS_LOG_ERROR("Building up grid record table");
        H5Fclose(file_id);
        return ERROR;
    }

    total_size = sizeof(GEOM_GRID_RECORD);

    /* Read the table, can also use "H5TBread_table" instead */
    if (H5TBread_records(file_id, GEOM_GRID_TABLE_NAME, 0, 1, total_size,
        offsets, field_size, &grid_data) < 0)
    {
        IAS_LOG_ERROR("Reading grid record table: %s", GEOM_GRID_TABLE_NAME);
        cleanup_grid_table_description(fields_to_close);
        H5Fclose(file_id);
        return ERROR;
    }

    /* Clean up the grid record table */
    cleanup_grid_table_description(fields_to_close);

    /* Close the HDF5 File */
    H5Fclose(file_id);

    /* flip the coords if the projection is SOM */
    flip_SOM_corners_for_read(&grid_data);

    /* Populate the contents that were read from the table fields to the final 
       grid structure */
    strcpy(grid->gridname, grid_data.gridname);
    grid->gridtype = grid_data.gridtype;
    grid->nbands = grid_data.nbands;

    for (i = 0; i < grid->nbands; i++)
        grid->band_index_list[i] = grid_data.band_index_list[i];

    if (strcasecmp(grid_data.frame_type, GEOBOX_FRAME) == 0)
        grid->frame_type = IAS_GEOBOX;
    else if (strcasecmp(grid_data.frame_type, PROJBOX_FRAME) == 0)
        grid->frame_type = IAS_PROJBOX;
    else if (strcasecmp(grid_data.frame_type, UL_SIZE_FRAME) == 0)
        grid->frame_type = IAS_UL_SIZE;
    else if (strcasecmp(grid_data.frame_type, MINBOX_FRAME) == 0)
        grid->frame_type = IAS_MINBOX;
    else if (strcasecmp(grid_data.frame_type, MAXBOX_FRAME) == 0)
        grid->frame_type = IAS_MAXBOX;
    else if (strcasecmp(grid_data.frame_type, PATH_ORIENTED_FRAME) == 0)
        grid->frame_type = IAS_PATH_ORIENTED;
    else if (strcasecmp(grid_data.frame_type, PATH_MINBOX_FRAME) == 0)
        grid->frame_type = IAS_PATH_MINBOX;
    else if (strcasecmp(grid_data.frame_type, PATH_MAXBOX_FRAME) == 0)
        grid->frame_type = IAS_PATH_MAXBOX;
    else
    {
        IAS_LOG_ERROR("Unsupported frame type %s in %s", 
                 grid_data.frame_type, grid_filename);
        return ERROR;
    }

    strcpy(grid->units, grid_data.units);
    grid->code = grid_data.code;
    strcpy(grid->datum, grid_data.datum);
    grid->spheroid = grid_data.spheroid;
    grid->zone = grid_data.zone;
    for (i = 0; i < IAS_PROJ_PARAM_SIZE; i++)
        grid->projprms[i] = grid_data.projprms[i];

    grid->corners = grid_data.corners;
    grid->center_pt = grid_data.center_pt;
    grid->proj_center_pt = grid_data.proj_center_pt;
    grid->rotate_angle = grid_data.rotate_angle;
    grid->orient_angle = grid_data.orient_angle;

    return SUCCESS;
}

/*************************************************************************

NAME: write_grid_header

PURPOSE: Write geometric grid data into HDF5 table

RETURNS: SUCCESS -- successfully write grid header
         ERROR -- error in writing grid header

**************************************************************************/
static int write_grid_header
(
    const char *grid_filename,       /* I: Grid file name          */
    const IAS_GEOM_GRID_TYPE *grid   /* I: Grid structure to write */
)
{
    hid_t file_id;                        /* File handle for HDF       */
    size_t offsets[GRID_FIELDS];          /* Offsets of the fields     */
    const char *field_names[GRID_FIELDS]; /* Names of the fields       */
    hid_t field_type[GRID_FIELDS];        /* Datatype of the fields    */
    size_t field_size[GRID_FIELDS];       /* Size of the fields        */
    hid_t fields_to_close[GRID_FIELDS];   /* Fields to close           */
    int status;                           /* Return status of the description 
                                             build routines */
    size_t total_size;                    /* Total size of the table       */
    GEOM_GRID_RECORD grid_data;           /* Subset of the GEOM model structure
                                             containing members that go into a 
                                             general table.                */
    int i;

    /* Initialize the structure */
    memset(&grid_data, 0, sizeof(grid_data));    
 
    /* Copy the input grid information to the internal structure members to be 
       written to the table fields in the HDF5 file */
    memset(grid_data.gridname, 0, sizeof(grid_data.gridname));
    strcpy(grid_data.gridname, grid->gridname);
    grid_data.gridtype = grid->gridtype;
    grid_data.nbands = grid->nbands;
    for (i = 0; i < grid->nbands; i++)
        grid_data.band_index_list[i] = grid->band_index_list[i];

    switch (grid->frame_type)
    {
        case IAS_GEOBOX:
            strcpy(grid_data.frame_type, GEOBOX_FRAME);
            break;
        case IAS_PROJBOX:
            strcpy(grid_data.frame_type, PROJBOX_FRAME);
            break;
        case IAS_UL_SIZE:
            strcpy(grid_data.frame_type, UL_SIZE_FRAME);
            break;
        case IAS_MINBOX:
            strcpy(grid_data.frame_type, MINBOX_FRAME);
            break;
        case IAS_MAXBOX:
            strcpy(grid_data.frame_type, MAXBOX_FRAME);
            break;
        case IAS_PATH_ORIENTED:
            strcpy(grid_data.frame_type, PATH_ORIENTED_FRAME);
            break;
        case IAS_PATH_MINBOX:
            strcpy(grid_data.frame_type, PATH_MINBOX_FRAME);
            break;
        case IAS_PATH_MAXBOX:
            strcpy(grid_data.frame_type, PATH_MAXBOX_FRAME);
            break;
        default:
            IAS_LOG_ERROR("Unsupported frame type: %d", grid->frame_type);
            return ERROR;
    }

    strcpy(grid_data.units, grid->units);
    grid_data.code = grid->code;
    strcpy(grid_data.datum, grid->datum);
    grid_data.spheroid = grid->spheroid;
    grid_data.zone = grid->zone;
    for (i = 0; i < IAS_PROJ_PARAM_SIZE; i++)
        grid_data.projprms[i] = grid->projprms[i];

    grid_data.corners = grid->corners;
    grid_data.center_pt = grid->center_pt;
    grid_data.proj_center_pt = grid->proj_center_pt;
    grid_data.rotate_angle = grid->rotate_angle;
    grid_data.orient_angle = grid->orient_angle;

    /* flip the coords if the projection is SOM */
    flip_SOM_corners_for_write(&grid_data);    

    /* Open the input file */
    file_id =  H5Fopen(grid_filename, H5F_ACC_RDWR, H5P_DEFAULT);

    if (file_id < 0)
    {
        IAS_LOG_ERROR("Opening grid file: %s", grid_filename);
        return ERROR;
    }

    /* Build the grid record table */
    if (build_grid_record_description(offsets, field_names, field_type,
                fields_to_close, field_size) == ERROR)
    {
        IAS_LOG_ERROR("Building the grid table");
        H5Fclose(file_id);
        return ERROR;
    }

    total_size = sizeof(GEOM_GRID_RECORD);

    /* Create the Grid Record Table     */
    status = H5TBmake_table(GEOM_GRID_TABLE_TITLE, file_id, 
                            GEOM_GRID_TABLE_NAME, GRID_FIELDS, 1, total_size,
                             field_names, offsets, field_type, 1, NULL, 0,
                             &grid_data);
    if (status < 0)
    {
     IAS_LOG_ERROR("Writing geometric grid table: %s", GEOM_GRID_TABLE_NAME);
        cleanup_grid_table_description(fields_to_close);
        H5Fclose(file_id);
        return ERROR;
    }   

    /* Clean up the grid record table */
    cleanup_grid_table_description(fields_to_close);

    /* Close the HDF5 File */
    H5Fclose(file_id);
    return SUCCESS;
}

/*************************************************************************

NAME: ias_read_geom_grid_band_header

PURPOSE: Read out geometric grid band data from HDF5 table

RETURNS: SUCCESS -- successfully read grid band header
         ERROR -- error in reading grid band header

NOTES: Grid band table has GEOM_BANDS sets of GEOM_GRID_BAND_RECORD data

**************************************************************************/
int ias_read_geom_grid_band_header
(
    const char *grid_filename, /* I: Grid file name         */
    IAS_GEOM_GRID_TYPE *grid   /* O: Grid structure to read */
)
{
    hid_t file_id;                        /* File handle for HDF     */
    size_t offsets[BAND_FIELDS];          /* Offsets of the fields   */
    const char *field_names[BAND_FIELDS]; /* Names of the fields     */
    hid_t field_type[BAND_FIELDS];        /* Datatypes of the fields */
    size_t field_size[BAND_FIELDS];       /* Size of the fields      */
    hid_t fields_to_close[BAND_FIELDS];   /* Fields to close         */
    int status;                           /* Return status of the description 
                                             build routines */
    size_t total_size;                    /* Total size of the table */
    int band_counter;
    hsize_t number_of_fields;             /* Number of fields in the table */
    hsize_t number_of_records;            /* Number of record in the table */

    /* Check grid is not NULL */
    if (grid == NULL)
    {
        IAS_LOG_ERROR("Grid is empty, grid header needs to be first read");
        return ERROR;
    }

    GEOM_GRID_BAND_RECORD band[grid->nbands]; 
                                          /* Subset of the GEOM model structure
                                             containing members that go into a 
                                             general table.          */

    /* Open the input file */
    file_id =  H5Fopen(grid_filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file_id < 0)
    {
        IAS_LOG_ERROR("Opening grid file: %s", grid_filename);
        return ERROR;
    }

    /* Get the grid table information */
    status = H5TBget_table_info(file_id, GEOM_GRID_BAND_TABLE_NAME,
        &number_of_fields, &number_of_records);
    if (status < 0)
    {
        IAS_LOG_ERROR("Could not get grid band table information:%s",
                    GEOM_GRID_BAND_TABLE_NAME);
        H5Fclose(file_id);        
        return ERROR;
    }

    if (number_of_records != grid->nbands)
    {
        IAS_LOG_ERROR("Incorrect number of records in grid band table");
        H5Fclose(file_id);        
        return ERROR;
    }

    /* Build the grid band record table */
    if (build_grid_band_record_description(offsets, field_names, field_type,
                fields_to_close, field_size) == ERROR)
    {
        IAS_LOG_ERROR ("Building grid band record table");
        H5Fclose(file_id);        
        return ERROR;
    }

    total_size = sizeof(GEOM_GRID_BAND_RECORD);

    /* Read out the grid band table */
    if (H5TBread_table(file_id, GEOM_GRID_BAND_TABLE_NAME, total_size,
            offsets, field_size, band) < 0)
    {
     IAS_LOG_ERROR("Reading grid band table: %s", GEOM_GRID_BAND_TABLE_NAME);
        /* Clean up the grid band table */
        cleanup_band_table_description(fields_to_close);
        H5Fclose(file_id);        
        return ERROR;
    }

    /* Clean up the grid band table */
    cleanup_band_table_description(fields_to_close);

    /* Close the HDF5 File */
    H5Fclose(file_id);

    /* Allocate memory for the gridbands array */
    if (grid->gridbands == NULL)
    {
        grid->gridbands = calloc(grid->nbands,
                                 sizeof(IAS_GEOM_GRID_BAND_TYPE));
        if (grid->gridbands == NULL)
        {
            IAS_LOG_ERROR("Allocating 2d grid band type structure array");
            H5Fclose(file_id);        
            return ERROR;
        }
    }

    /* Populate the contents that were read from the table fields to the final 
       grid structure */
    for (band_counter = 0; band_counter < grid->nbands; band_counter++)
    {
        IAS_GEOM_GRID_BAND_TYPE *grid_band;
        GEOM_GRID_BAND_RECORD *band_table;

        grid_band = &grid->gridbands[band_counter];
        band_table = &band[band_counter];

        grid_band->band_number 
                    = band_table->band_number;
        grid_band->gridtype 
                    = band_table->gridtype;
        grid_band->cell_lines 
                     = band_table->cell_lines;
        grid_band->cell_samps 
                     = band_table->cell_samps;
        grid_band->pixsize_line 
                     = band_table->pixsize_line;
        grid_band->pixsize_samp
                     = band_table->pixsize_samp;
        grid_band->output_image_lines 
                     = band_table->output_image_lines;
        grid_band->output_image_samps 
                     = band_table->output_image_samps;
        grid_band->num_in_lines 
                     = band_table->num_in_lines;
        grid_band->num_in_samps 
                     = band_table->num_in_samps;
        grid_band->num_out_lines 
                     = band_table->num_out_lines;
        grid_band->num_out_samps 
                     = band_table->num_out_samps;
        grid_band->forward_present 
                     = band_table->forward_present;
        grid_band->reverse_present 
                     = band_table->reverse_present;
        grid_band->grid_rows 
                     = band_table->grid_rows;
        grid_band->grid_cols 
                     = band_table->grid_cols;
        grid_band->degree 
                     = band_table->degree;
    }

    return SUCCESS;
}


/*************************************************************************

NAME: write_grid_band_header

PURPOSE: Write geometric grid data into HDF5 table

RETURNS: SUCCESS -- successfully write grid band header
         ERROR -- error in writing grid band header

NOTES: Grid band table has GEOM_BANDS*GEOM_NSCA sets of GEOM_GRID_BAND_RECORD 
       data

**************************************************************************/
static int write_grid_band_header
(
    const char *grid_filename,       /* I: Grid file name */
    const IAS_GEOM_GRID_TYPE *grid   /* I: Grid structure to write */
)
{
    hid_t file_id; /* File handle for HDF */
    size_t offsets[BAND_FIELDS];          /* Offsets of the fields   */
    const char *field_names[BAND_FIELDS]; /* Names of the fields     */
    hid_t field_type[BAND_FIELDS];        /* Datatype of the fields  */
    size_t field_size[BAND_FIELDS];       /* Size of the fields      */
    hid_t fields_to_close[BAND_FIELDS];   /* Fields to close         */
    int status;                           /* Return status of the description 
                                             build routines             */
    size_t total_size;                    /* Total size of the one record */
    GEOM_GRID_BAND_RECORD band[grid->nbands]; 
                                          /* Subset of the GEOM model structure 
                                             containing members that go into a 
                                             general table. */
    int band_counter;

    /* Initialize the buffer before copying data to make sure any pad bytes
       are initialized (to avoid valgrind errors) */
    memset(band, 0, sizeof(band));

    /* Copy the input grid information to the internal structure members to be 
       written to the table fields in the HDF5 file */
    for (band_counter = 0; band_counter < grid->nbands; band_counter++)
    {
        IAS_GEOM_GRID_BAND_TYPE *grid_band;
        GEOM_GRID_BAND_RECORD *band_table;

        grid_band = &grid->gridbands[band_counter];
        band_table = &band[band_counter];

        band_table->band_number 
              = grid_band->band_number;
        band_table->gridtype 
              = grid_band->gridtype;
        band_table->cell_lines 
              = grid_band->cell_lines;
        band_table->cell_samps 
              = grid_band->cell_samps;
        band_table->pixsize_line 
              = grid_band->pixsize_line;
        band_table->pixsize_samp
              = grid_band->pixsize_samp;
        band_table->output_image_lines
              = grid_band->output_image_lines;
        band_table->output_image_samps 
              = grid_band->output_image_samps;
        band_table->num_in_lines 
              = grid_band->num_in_lines;
        band_table->num_in_samps 
              = grid_band->num_in_samps;
        band_table->num_out_lines 
              = grid_band->num_out_lines;
        band_table->num_out_samps 
              = grid_band->num_out_samps;
        band_table->forward_present
              = grid_band->forward_present;
        band_table->reverse_present
              = grid_band->reverse_present;
        band_table->grid_rows 
              = grid_band->grid_rows;
        band_table->grid_cols 
              = grid_band->grid_cols;
        band_table->degree 
              = grid_band->degree;
    }

    /* Open the input file */
    file_id =  H5Fopen(grid_filename, H5F_ACC_RDWR, H5P_DEFAULT);
    if (file_id < 0)
    {
        IAS_LOG_ERROR("Opening grid file: %s", grid_filename);
        return ERROR;
    }

    /* Build up the grid band record table */
    if (build_grid_band_record_description(offsets, field_names, field_type,
                fields_to_close, field_size) == ERROR)
    {
        IAS_LOG_ERROR("Building the grid band table");
        H5Fclose(file_id);
        return ERROR;
    }

    total_size = sizeof(GEOM_GRID_BAND_RECORD);

    /* Build Up the Grid Band Table */
    status = H5TBmake_table(GEOM_GRID_BAND_TABLE_TITLE, file_id, 
               GEOM_GRID_BAND_TABLE_NAME, BAND_FIELDS, grid->nbands, 
               total_size, field_names, offsets, field_type, 1, NULL, 0, band);
    if (status < 0)
    {
     IAS_LOG_ERROR("Writing band record table: %s", GEOM_GRID_BAND_TABLE_NAME);
        cleanup_band_table_description(fields_to_close);
        H5Fclose(file_id);
        return ERROR;
    }
   
    /* Clean up the grid band table */
    cleanup_band_table_description(fields_to_close);

    /* Close the HDF5 File */
    H5Fclose(file_id);
    return SUCCESS;
}

/*************************************************************************

NAME: read_one_dataset

PURPOSE: read one grid band pointer dataset from HDF5 file

RETURNS: SUCCESS -- successfully read grid band pointers
         ERROR -- error in reading grid band pointers

**************************************************************************/
static int read_one_dataset
(
    hid_t group_id,           /* I: dataset identifier */
    int band_index,           /* I: band index for current read */
    const char *dataset_name, /* I: dataset name */   
    int  offset,              /* I: offset for the dataset */
    int expected_dataset_size,/* I: expected dataset size to read */
    IAS_GEOM_GRID_TYPE *grid   /* O: grid structure to read */
)
{
    hid_t dataset_id, dataspace_id;   /* identifiers */
    hsize_t dims[1];                  /* Number of row dimensions in array */
    hsize_t ndims;                    /* Number of row dimensions in array */
    herr_t  status;                   /* HDF5 return status */
    void **member_ptr;                /* pointer for getting offset location */
    void *dataset;                    /* dataset buffer pointer */

    /* Open the dataset */
    dataset_id = H5Dopen(group_id, dataset_name, H5P_DEFAULT);
    if (dataset_id < 1)
    {
        IAS_LOG_ERROR("Opening dataset: %s", dataset_name);
        return ERROR;     
    }

    /* Get the data space and its dimensions */
    dataspace_id = H5Dget_space(dataset_id);
    if (dataspace_id < 1)
    {
        IAS_LOG_ERROR("Getting dataspace for dataset : %s", dataset_name);
        return ERROR;     
    }
    ndims = H5Sget_simple_extent_ndims(dataspace_id);
    /* Check dataset space dimesion sizes */
    if (ndims != 1)
    {
        IAS_LOG_ERROR("Incorrect %s dimensions "
            "should be 1", dataset_name);
        H5Sclose(dataspace_id);
        H5Dclose(dataset_id);
        return ERROR;
    }


    H5Sget_simple_extent_dims(dataspace_id, dims, NULL);
    /* Check dataset space dimesion sizes */
    if (dims[0] != expected_dataset_size)
    {
        IAS_LOG_ERROR("Incorrect %s dimensions "
            "%llu, should be %d", dataset_name, dims[0],
            expected_dataset_size);
        H5Sclose(dataspace_id);
        H5Dclose(dataset_id);
        return ERROR;
    }
    H5Sclose(dataspace_id);

    /* Calculating the address of the particular array to read in
       the grid band structure by using offset as an indication of
       which member of the structure is being read, member_ptr is
       a void** because the dataset pointer will indicate the actual
       buffer and its type to read into */
    member_ptr = (void *)((unsigned char*)
                &grid->gridbands[band_index]) + offset;
    dataset = *member_ptr; 

    /* Read in one dataset */
    status = H5Dread(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, 
                         H5S_ALL, H5P_DEFAULT, dataset);
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading the %s dataset", dataset_name);
        H5Dclose(dataset_id);
        return ERROR;
    }

    /* Close the dataset. */
    H5Dclose(dataset_id);
    return SUCCESS;
}
    
/*************************************************************************

NAME: write_one_dataset

PURPOSE: write one grid band pointer dataset to HDF5 file

RETURNS: SUCCESS -- successfully read grid band pointers
         ERROR -- error in reading grid band pointers

**************************************************************************/
static int write_one_dataset
(
    hid_t group_id,         /* I: dataset identifier */
    int band_index,         /* I: band index for current read */
    char *dataset_name,     /* I: dataset name */
    int  offset,            /* I: offset of dataset */
    int dataset_size,       /* I: dataset size of write out dataset */
    const IAS_GEOM_GRID_TYPE *grid  /* I: grid structure to write */
)
{
    hid_t dataset_id, dataspace_id;   /* identifiers */
    hsize_t dims[1];                  /* Number of row dimensions in array */
    herr_t  status;                   /* HDF5 return status */
    void **member_ptr;                /* pointer for getting offset location */
    void *dataset;                    /* dataset buffer pointer */

    /* define the dataset data space dimension sizes */
    dims[0] = dataset_size;      

    /* Create the data space for dataset */
    dataspace_id = H5Screate_simple(1, dims, NULL);
    if (dataspace_id < 0)
    {
        IAS_LOG_ERROR("Creating data space for dataset %s", dataset_name);
        return ERROR;
    }

    /* Create dataset in group "B#"  */
    dataset_id = H5Dcreate(group_id, dataset_name, H5T_NATIVE_DOUBLE, 
            dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    if (dataset_id < 0)
    {
        IAS_LOG_ERROR("Creating dataset: %s", dataset_name);
        H5Sclose(dataspace_id);
        return ERROR;
    }

    H5Sclose(dataspace_id);

    /* Calculating the address of the particular array to read in
       the grid band structure by using offset as an indication of
       which member of the structure is being read, member_ptr is
       a void** because the dataset pointer will indicate the actual
       buffer and its type to read into */
    member_ptr = (void *)((unsigned char*)
                 &grid->gridbands[band_index]) + offset; 
    dataset = *member_ptr;

    /* Write one dataset. */
    status = H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, 
                      H5S_ALL, H5P_DEFAULT, dataset);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing the dataset: %s", dataset_name);
        H5Dclose(dataset_id);
        return ERROR;
    }

    /* Close the data space for the second dataset. */
    H5Dclose(dataset_id);
    return SUCCESS;
}
    
/*************************************************************************

NAME: ias_read_geom_grid_band_pointers

PURPOSE: Read out geometric grid band pointer data from HDF5 datasets

RETURNS: SUCCESS -- successfully read grid band pointers
         ERROR -- error in reading grid band pointers

**************************************************************************/
int ias_read_geom_grid_band_pointers 
(
  const char *grid_filename, /* I: Grid input file name                     */
  const int *band_number,    /* I/O: Band number to read from the grid */
  int *nbands,               /* I: Number of bands in the bands array       */
  IAS_GEOM_GRID_TYPE *grid   /* O: Grid structure populated from file       */
)
{
    hid_t file_id;                    /* File handle for HDF */
    hid_t group_id;                   /* Group handler */
    int band_counter;                 /* Band counter */
    int status;                       /* Return status */
    char group_name[LENGTH];              /* Group name */
    int offset;                       /* Offset of dataset */
    int start_index, end_index;
     
    /* Check grid is not NULL */
    if (grid == NULL)
    {
        IAS_LOG_ERROR("Grid is empty, grid header needs to be first read");
        return ERROR;
    }

    /* Open the input file */
    file_id =  H5Fopen(grid_filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file_id < 0)
    {
        IAS_LOG_ERROR("Opening grid file: %s", grid_filename);
        return ERROR;
    }

    /* If no bands are specified then malloc all the bands in the grid. */
    if (*nbands == 0 || *nbands == grid->nbands)
    {
        start_index = 0;
        end_index = grid->nbands;
        *nbands = end_index;
    }
    else if (*nbands == 1)   /* malloc only one band based on band_number */
    {
        /* Verify the band exists in the HDF file */
        if (*band_number < 1 || *band_number > grid->nbands)
        {
            IAS_LOG_ERROR("Band %d is not available", *band_number);
            return ERROR;   
        }
        start_index = ias_sat_attr_convert_band_number_to_index(*band_number);
        if (start_index == -1)
        {
            IAS_LOG_ERROR("Incorrect band index converted from band number %d",
                              *band_number);
            return ERROR;               
        }
        end_index = start_index + 1;
    }
    else
    {
        IAS_LOG_ERROR(" *nbands should be 0 or 1"); 
        return ERROR;
    }

    /* Check memory allocation for the gridbands array */
    if (grid->gridbands == NULL)
    {
        IAS_LOG_ERROR("The grid->gridbands should have been allocated");
        H5Fclose(file_id);        
        return ERROR;
    }

    /* Allocate memory for the requested bands */        
    status = ias_geom_grid_malloc(band_number, nbands, grid);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Allocating memory for grid in: %s", grid_filename);
        H5Fclose(file_id);
        return ERROR;
    } 

    /* Loop through the bands mallocing the correct reference bands.  */
    for (band_counter = start_index; band_counter < end_index; band_counter++)
    {
        /* Open "B#" group */
        sprintf(group_name, "/B%d",band_counter+1);
        group_id = H5Gopen(file_id, group_name, H5P_DEFAULT);
        if (group_id < 0) 
        { 
            IAS_LOG_ERROR("Opening %s in %s", group_name, grid_filename);
            ias_geom_grid_free(band_number, nbands, grid);
            H5Fclose(file_id);
            return ERROR; 
        } 

        /* Read in "in_lines" dataset */
        offset = offsetof(struct ias_geom_grid_band_type, in_lines);
        status = read_one_dataset(group_id, band_counter, "in_lines", offset, 
                          grid->gridbands[band_counter].num_in_lines, grid);
        if (status < 0)
        {
            IAS_LOG_ERROR("Reading dataset in_lines");
            ias_geom_grid_free(band_number, nbands, grid);
            H5Gclose(group_id);
            H5Fclose(file_id);
            return ERROR;
        } 

        /* Read in "in_samps" dataset */
        offset = offsetof(struct ias_geom_grid_band_type, in_samps);
        status = read_one_dataset(group_id, band_counter, "in_samps", offset, 
                          grid->gridbands[band_counter].num_in_samps, grid);
        if (status < 0)
        {
            IAS_LOG_ERROR("Reading dataset in_samps");
            ias_geom_grid_free(band_number, nbands, grid);
            H5Gclose(group_id);
            H5Fclose(file_id);
            return ERROR;
        } 

        /* Read in "out_lines" dataset */
        offset = offsetof(struct ias_geom_grid_band_type, out_lines);
        status = read_one_dataset(group_id, band_counter, "out_lines", offset, 
                          grid->gridbands[band_counter].num_out_lines,grid);
        if (status < 0)
        {
            IAS_LOG_ERROR("Reading dataset out_lines");
            ias_geom_grid_free(band_number, nbands, grid);
            H5Gclose(group_id);
            H5Fclose(file_id);
            return ERROR;
        } 

        /* Read in "out_samps" dataset */
        offset = offsetof(struct ias_geom_grid_band_type, out_samps);
        status = read_one_dataset(group_id, band_counter, "out_samps", offset, 
                         grid->gridbands[band_counter].num_out_samps, grid);
        if (status < 0)
        {
            IAS_LOG_ERROR("Reading dataset out_samps");
            ias_geom_grid_free(band_number, nbands, grid);
            H5Gclose(group_id);
            H5Fclose(file_id);
            return ERROR;
        }
 
        /* Read in "forward_coeffs" dataset */
        if (grid->gridbands[band_counter].forward_present)
        {
            offset = offsetof(struct ias_geom_grid_band_type, forward_coeffs);
            status = read_one_dataset(group_id, band_counter, 
                         "forward_coeffs", offset, 2*COEFS_SIZE*
                         grid->gridbands[band_counter].grid_rows
                         *grid->gridbands[band_counter].grid_cols, grid); 
            if (status < 0)
            {
                IAS_LOG_ERROR("Reading dataset sattoproj");
                ias_geom_grid_free(band_number, nbands, grid);
                H5Gclose(group_id);
                H5Fclose(file_id);
                return ERROR;
            } 
        }

        /* Read in "reverse_coeffs" dataset */
        if (grid->gridbands[band_counter].reverse_present)
        {
            offset = offsetof(struct ias_geom_grid_band_type, reverse_coeffs);
            status = read_one_dataset(group_id, band_counter, 
                         "reverse_coeffs", offset, 2*COEFS_SIZE*
                         grid->gridbands[band_counter].grid_rows
                         *grid->gridbands[band_counter].grid_cols, grid); 
            if (status < 0)
            {
                IAS_LOG_ERROR("Reading dataset projtosat");
                ias_geom_grid_free(band_number, nbands, grid);
                H5Gclose(group_id);
                H5Fclose(file_id);
                return ERROR;
            }
        } 

        /* Read in "poly_lines" dataset */
        offset = offsetof(struct ias_geom_grid_band_type, poly_lines);
        status = read_one_dataset(group_id, band_counter, 
                         "poly_lines", offset, 
                         (grid->gridbands[band_counter].degree+1)
                         *(grid->gridbands[band_counter].degree+1), grid); 
        if (status < 0)
        {
            IAS_LOG_ERROR("Reading dataset poly_lines");
            ias_geom_grid_free(band_number, nbands, grid);
            H5Gclose(group_id);
            H5Fclose(file_id);
            return ERROR;
        } 

        /* Read in "poly_samps" dataset */
        offset = offsetof(struct ias_geom_grid_band_type, poly_samps);
        status = read_one_dataset(group_id, band_counter, 
                         "poly_samps", offset,  
                         (grid->gridbands[band_counter].degree+1)
                         *(grid->gridbands[band_counter].degree+1), grid); 
        if (status < 0)
        {
            IAS_LOG_ERROR("Reading dataset poly_samps");
            ias_geom_grid_free(band_number, nbands, grid);
            H5Gclose(group_id);
            H5Fclose(file_id);
            return ERROR;
        } 

        /* Close the group. */
        H5Gclose(group_id);        
    }

    /* Close the HDF5 File */
    H5Fclose(file_id);
    return SUCCESS;
}

/*************************************************************************

NAME: write_grid_band_pointers

PURPOSE: Write geometric grid data into HDF5 datasets

RETURNS: SUCCESS -- successfully write grid band pointers 
         ERROR -- error in writing grid band pointers

**************************************************************************/
static int write_grid_band_pointers
(
    const char *grid_filename,      /* I: Grid filename */
    const IAS_GEOM_GRID_TYPE *grid   /* I: Grid structure to write */
)
{
    hid_t file_id;                    /* File handle for HDF */
    hid_t group_id;                   /* Dataset group handler */     
    int status;                       /* Return status */
    int band_counter;                 /* Band counter */
    char group_name[LENGTH];          /* Group name */ 
    int offset;                       /* Group offset */

    /* Check grid is not NULL */
    if (grid == NULL)
    {
        IAS_LOG_ERROR("Grid is empty");
        return ERROR;
    }

    /* Open the input file */
    file_id =  H5Fopen(grid_filename, H5F_ACC_RDWR, H5P_DEFAULT);
    if (file_id < 0)
    {
        IAS_LOG_ERROR("Opening grid file: %s", grid_filename);
        return ERROR;
    }

    /* Create "/B#" group and write in 6 databases for each band*/
    for (band_counter = 0; band_counter < grid->nbands; band_counter++)
    {
        /* Create group "B#" in the root group */
        sprintf(group_name, "/B%d", band_counter+1);
        group_id = H5Gcreate(file_id, group_name, H5P_DEFAULT, 
                       H5P_DEFAULT, H5P_DEFAULT);
        if (group_id < 0) 
        { 
            IAS_LOG_ERROR("Creating %s in %s", group_name, grid_filename);
            H5Fclose(file_id);
            return ERROR;
        } 


        /* Write out "in_lines" dataset */
        offset = offsetof(struct ias_geom_grid_band_type, in_lines);
        status = write_one_dataset(group_id, band_counter, "in_lines", offset, 
                          grid->gridbands[band_counter].num_in_lines, grid);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing dataset in_lines");
            H5Gclose(group_id);
            H5Fclose(file_id);
            return ERROR;
        } 

        /* Write out "in_samps" dataset */
        offset = offsetof(struct ias_geom_grid_band_type, in_samps);
        status = write_one_dataset(group_id, band_counter, "in_samps", offset, 
                          grid->gridbands[band_counter].num_in_samps, grid);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing dataset in_samps");
            H5Gclose(group_id);
            H5Fclose(file_id);
            return ERROR;
        } 

        /* Write out "out_lines" dataset */
        offset = offsetof(struct ias_geom_grid_band_type, out_lines);
        status = write_one_dataset(group_id, band_counter, "out_lines", offset,
                         grid->gridbands[band_counter].num_out_lines, grid);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing dataset out_lines");
            H5Gclose(group_id);
            H5Fclose(file_id);
            return ERROR;
        } 

        /* Write out "out_samps" dataset */
        offset = offsetof(struct ias_geom_grid_band_type, out_samps);
        status = write_one_dataset(group_id, band_counter, "out_samps", offset,
                         grid->gridbands[band_counter].num_out_samps, grid);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing dataset out_samps");
            H5Gclose(group_id);
            H5Fclose(file_id);
            return ERROR;
        } 

        /* Write out "forward_coeffs" dataset */
        if (grid->gridbands[band_counter].forward_present)
        {
            offset = offsetof(struct ias_geom_grid_band_type, forward_coeffs);
            status = write_one_dataset(group_id, band_counter, 
                         "forward_coeffs", offset, 2*COEFS_SIZE 
                         *grid->gridbands[band_counter].grid_rows
                         *grid->gridbands[band_counter].grid_cols, grid); 
            if (status < 0)
            {
                IAS_LOG_ERROR("Writing dataset sattoproj");
                H5Gclose(group_id);
                H5Fclose(file_id);
                return ERROR;
            }
        } 

        /* Write out "reverse_coeffs" dataset */
        if (grid->gridbands[band_counter].reverse_present)
        {
            offset = offsetof(struct ias_geom_grid_band_type, reverse_coeffs);
            status = write_one_dataset(group_id, band_counter, 
                         "reverse_coeffs", offset, 2*COEFS_SIZE 
                         *grid->gridbands[band_counter].grid_rows
                         *grid->gridbands[band_counter].grid_cols, grid); 
            if (status < 0)
            {
                IAS_LOG_ERROR("Writing dataset projtosat");
                H5Gclose(group_id);
                H5Fclose(file_id);
                return ERROR;
            } 
        }

        /* Write out "poly_lines" dataset */
        offset = offsetof(struct ias_geom_grid_band_type, poly_lines);
        status = write_one_dataset(group_id, band_counter, 
                         "poly_lines", offset, 
                         (grid->gridbands[band_counter].degree+1)
                         *(grid->gridbands[band_counter].degree+1), grid); 
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing dataset poly_lines");
            H5Gclose(group_id);
            H5Fclose(file_id);
            return ERROR;
        }
  
        /* Write out "poly_samps" dataset */
        offset = offsetof(struct ias_geom_grid_band_type, poly_samps);
        status = write_one_dataset(group_id, band_counter, 
                         "poly_samps", offset,  
                         (grid->gridbands[band_counter].degree+1)
                         *(grid->gridbands[band_counter].degree+1), grid); 

        if (status < 0)
        {
            IAS_LOG_ERROR("Writing dataset poly_samps");
            H5Gclose(group_id);
            H5Fclose(file_id);
            return ERROR;
        } 

        /* Close the group. */
        H5Gclose(group_id); 
    }
        
    /* Close the HDF5 File */
    H5Fclose(file_id);

    return SUCCESS;
}    

/*************************************************************************

NAME: ias_read_geom_grid

PURPOSE: Read out geometric grid data from HDF5 format file

RETURNS: SUCCESS -- successfully read in grid
         ERROR -- error in reading grid

**************************************************************************/
int ias_read_geom_grid 
(
    const char *grid_filename, /* I: Grid input file name                 */
    const int *band_number,    /* I/O: Band number to read from the grid */
    int *nbands,               /* I: Number of bands in the bands array   */
    IAS_GEOM_GRID_TYPE *grid   /* O: Grid structure populated from file   */
)
{
    /* Make sure the grid memory is initialized */
    memset(grid, 0, sizeof(IAS_GEOM_GRID_TYPE));

    /* Read grid header  */
    if (ias_read_geom_grid_header(grid_filename, grid) != SUCCESS)
    {
        IAS_LOG_ERROR("Reading grid header");
        return ERROR;
    }

    /* Read grid band header */
    if (ias_read_geom_grid_band_header(grid_filename, grid) != SUCCESS)
    {
        IAS_LOG_ERROR("Reading grid band header");
        return ERROR;
    }

    /* Read grid band pointers */
    if (ias_read_geom_grid_band_pointers(grid_filename,band_number,nbands,grid) 
             != SUCCESS)
    {
        IAS_LOG_ERROR("Reading grid band pointers");
        return ERROR;
    }

    return SUCCESS;
} 

/*************************************************************************

NAME: ias_write_geom_grid

PURPOSE: Write geometric grid data into HDF5 format file

RETURNS: SUCCESS -- successfully write grid
         ERROR -- error in writing grid

**************************************************************************/
int ias_write_geom_grid
(
    const char *grid_filename,       /* I: Grid input file name       */
    const IAS_GEOM_GRID_TYPE *grid   /* I: Grid structure to write   */
)
{
    hid_t file_id; /* File handle for HDF */
    int grid_format_version = GRID_FORMAT_VERSION;

    /* Open the output file */
    file_id = H5Fcreate(grid_filename, H5F_ACC_TRUNC, H5P_DEFAULT,
        H5P_DEFAULT);
    if (file_id < 0)
    {
        IAS_LOG_ERROR("Could not create grid file: %s", grid_filename);
        return ERROR;
    }

    /* Create a grid format version */
    if (H5LTset_attribute_int(file_id, "/", "Grid Format Version",
            &grid_format_version, 1) < 0)
    {
        IAS_LOG_ERROR("Could not create grid format version for %s",
                       grid_filename);
        H5Fclose(file_id);
        return ERROR;
    }

    /* Create a file grid type */
    if (H5LTset_attribute_string(file_id, "/", "File Type", FILE_TYPE) < 0)
    {
        IAS_LOG_ERROR("Could not create file type for %s",grid_filename);
        H5Fclose(file_id);
        return ERROR;
    }

    /* Close the HDF5 File */
    H5Fclose(file_id);

    /* Write grid header */
    if (write_grid_header(grid_filename, grid) != SUCCESS)
    {
        IAS_LOG_ERROR("Writing grid header");
        return ERROR;
    }

    /* Write grid band header */
    if (write_grid_band_header(grid_filename, grid) != SUCCESS)
    {
        IAS_LOG_ERROR("Writing grid band header");
        return ERROR;
    }

    /* Write grid band pointers */
    if (write_grid_band_pointers(grid_filename, grid)
               != SUCCESS)
    {
        IAS_LOG_ERROR("Writing grid band pointers");
        return ERROR;
    }

    return SUCCESS;
}  

/*************************************************************************

NAME: free_partial_grid

PURPOSE:free memory for partial grid band pointer data, the routine is
        called when allocating memory failed to free the already partial
        allocated memory space
        
RETURNS: None

**************************************************************************/
static void free_partial_grid 
(
    int band_index,            /* I: Number of bands of data to free */
    IAS_GEOM_GRID_TYPE *grid   /* I: Grid structure to free   */
)
{
    int band_counter;

    for (band_counter = 0; band_counter < band_index; band_counter++)
    {   
        free(grid->gridbands[band_counter].in_lines);
        grid->gridbands[band_counter].in_lines = NULL;
        free(grid->gridbands[band_counter].in_samps);
        grid->gridbands[band_counter].in_samps = NULL;
        free(grid->gridbands[band_counter].out_lines);
        grid->gridbands[band_counter].out_lines = NULL;
        free(grid->gridbands[band_counter].out_samps);
        grid->gridbands[band_counter].out_samps = NULL;
        free(grid->gridbands[band_counter].forward_coeffs);
        grid->gridbands[band_counter].forward_coeffs = NULL;
        free(grid->gridbands[band_counter].reverse_coeffs);
        grid->gridbands[band_counter].reverse_coeffs = NULL;
        free(grid->gridbands[band_counter].poly_lines);
        grid->gridbands[band_counter].poly_lines = NULL;
        free(grid->gridbands[band_counter].poly_samps);
        grid->gridbands[band_counter].poly_samps = NULL;
    }  
}

/*************************************************************************

NAME: ias_geom_grid_malloc

PURPOSE: Allocate memory for grid band pointer data

RETURNS: SUCCESS -- successfully malloc memory
         ERROR -- error in mallocing memory

**************************************************************************/
int ias_geom_grid_malloc 
(
  const int *band_number,    /* I/O: Array of bands to free from the grid   */
  int *nbands,               /* I/O: Number of bands in the bands array     */
  IAS_GEOM_GRID_TYPE *grid   /* I/O: Grid structure to allocate memory for  */
)
{
    int band_counter;
    int start_index, end_index;
  
    /* If no bands are specified then malloc all the bands in the grid. */
    if (*nbands == 0 || *nbands == grid->nbands)
    {
        start_index = 0;
        end_index = grid->nbands;
        *nbands = end_index;
    }
    else         /* malloc only one band based on band_number */
    {
        start_index = ias_sat_attr_convert_band_number_to_index(*band_number);
        if (start_index == -1)
        {
            IAS_LOG_ERROR("Incorrect band index converted from band number %d",
                              *band_number);
            return ERROR;               
        }
        end_index = start_index + 1;
    }

    /* Loop through the bands mallocing the correct reference bands.  */
    for (band_counter = start_index; band_counter < end_index; band_counter++)
    {
        grid->gridbands[band_counter].in_lines = malloc(
               grid->gridbands[band_counter].num_in_lines
               *sizeof(double)); 
        if (grid->gridbands[band_counter].in_lines == NULL)
        {
            IAS_LOG_ERROR("Allocating memory for in_lines");
            /* Free previously malloced pointers. */
            free_partial_grid(band_counter, grid);
            return ERROR;
        }         
        grid->gridbands[band_counter].in_samps = malloc(
            grid->gridbands[band_counter].num_in_samps 
            *sizeof(double)); 
        if (grid->gridbands[band_counter].in_samps == NULL)
        {
            IAS_LOG_ERROR("Allocating memory for in_samps");
            /* Free previously malloced pointers. */
            free_partial_grid(band_counter, grid);
            return ERROR;
        }         
        grid->gridbands[band_counter].out_lines = malloc(
            grid->gridbands[band_counter].num_out_lines
            *sizeof(double)); 
        if (grid->gridbands[band_counter].out_lines == NULL)
        {
            IAS_LOG_ERROR("Allocating memory for out_lines");
            /* Free previously malloced pointers. */
            free_partial_grid(band_counter, grid);
            return ERROR;
        }         
        grid->gridbands[band_counter].out_samps = malloc(
            grid->gridbands[band_counter].num_out_samps
            *sizeof(double)); 
        if (grid->gridbands[band_counter].out_samps == NULL)
        {
            IAS_LOG_ERROR("Allocating memory for out_samps");
            /* Free previously malloced pointers. */
            free_partial_grid(band_counter, grid);
            return ERROR;
        } 
        if (grid->gridbands[band_counter].forward_present)
        {        
            grid->gridbands[band_counter].forward_coeffs = malloc(
                grid->gridbands[band_counter].grid_rows
                *grid->gridbands[band_counter].grid_cols
                *sizeof(IAS_COEFFICIENTS)); 
            if (grid->gridbands[band_counter].forward_coeffs 
                == NULL)
            {
                IAS_LOG_ERROR("Allocating memory for forward_coeffs");
                /* Free previously malloced pointers. */
                free_partial_grid(band_counter, grid);
                return ERROR;
            }
        }         
        if (grid->gridbands[band_counter].reverse_present)
        {        
            grid->gridbands[band_counter].reverse_coeffs = malloc(
                grid->gridbands[band_counter].grid_rows
                *grid->gridbands[band_counter].grid_cols
                *sizeof(IAS_COEFFICIENTS)); 
            if (grid->gridbands[band_counter].reverse_coeffs 
                == NULL)
            {
                IAS_LOG_ERROR("Allocating memory for reverse_coeffs");
                /* Free previously malloced pointers. */
                free_partial_grid(band_counter, grid);
                return ERROR;
            }
        }         
        grid->gridbands[band_counter].poly_lines = calloc(
            (grid->gridbands[band_counter].degree+1)
            *(grid->gridbands[band_counter].degree+1),
            sizeof(double)); 
        if (grid->gridbands[band_counter].poly_lines == NULL)
        {
            IAS_LOG_ERROR("Allocating memory for poly_lines");
            /* Free previously malloced pointers. */
            free_partial_grid(band_counter, grid);
            return ERROR;
        }
        grid->gridbands[band_counter].poly_samps = calloc(
            (grid->gridbands[band_counter].degree+1)
            *(grid->gridbands[band_counter].degree+1),
            sizeof(double)); 
        if (grid->gridbands[band_counter].poly_samps == NULL)
        {
            IAS_LOG_ERROR("Allocating memory for poly_samps");
            /* Free previously malloced pointers. */
            free_partial_grid(band_counter, grid);
            return ERROR;
        }         
    }
    return SUCCESS;
}

/*************************************************************************

NAME: geom_grid_band_free

PURPOSE: Free memory for pointers within one gridbands pointer 

RETURNS: None

**************************************************************************/
static void geom_grid_band_free 
(
    IAS_GEOM_GRID_TYPE *grid, /* I/O: Grid structure in which band pointers
                                     to be freed             */
    const int band_index     /* I/O: band index to free from the grid   */
)
{
    /* free all the pointers for the band */
    free(grid->gridbands[band_index].in_lines);
    grid->gridbands[band_index].in_lines = NULL;
    free(grid->gridbands[band_index].in_samps);
    grid->gridbands[band_index].in_samps = NULL;
    free(grid->gridbands[band_index].out_lines);
    grid->gridbands[band_index].out_lines = NULL;
    free(grid->gridbands[band_index].out_samps);
    grid->gridbands[band_index].out_samps = NULL;
    if (grid->gridbands[band_index].forward_present)
    {
        free(grid->gridbands[band_index].forward_coeffs);
        grid->gridbands[band_index].forward_coeffs = NULL;
    }
    if (grid->gridbands[band_index].reverse_present)
    {
        free(grid->gridbands[band_index].reverse_coeffs);
        grid->gridbands[band_index].reverse_coeffs = NULL;
    }
    free(grid->gridbands[band_index].poly_lines);
    grid->gridbands[band_index].poly_lines = NULL;
    free(grid->gridbands[band_index].poly_samps);
    grid->gridbands[band_index].poly_samps = NULL;
}

/*************************************************************************

NAME: ias_geom_grid_free

PURPOSE: Free memory for grid band pointer data

RETURNS: SUCCESS -- successfully free memory
         ERROR -- error in freeing memory

**************************************************************************/
int ias_geom_grid_free 
(
  const int *band_number,    /* I/O: Array of bands to free from the grid   */
  int *nbands,               /* I/O: Number of bands in the bands array     */
  IAS_GEOM_GRID_TYPE *grid   /* I/O: Grid structure to be freed             */
)
{
    int band_counter;
    int start_index, end_index;
  
    /* If no bands are specified then malloc all the bands in the grid. */
    if (*nbands == 0 || *nbands == grid->nbands)
    {
        start_index = 0;
        end_index = grid->nbands;
        *nbands = end_index;
    }
    else if (*nbands == 1)   /* malloc only one band based on band_number */
    {
        /* Verify the band exists in the HDF file */
        if (*band_number < 1 || *band_number > grid->nbands)
        {
            IAS_LOG_ERROR("Band %d is not available", *band_number);
            return ERROR;   
        }
        start_index = ias_sat_attr_convert_band_number_to_index(*band_number);
        if (start_index == -1)
        {
            IAS_LOG_ERROR("Incorrect band index converted from band number %d",
                              *band_number);
            return ERROR;               
        }
        end_index = start_index + 1;
    }
    else
    {
        IAS_LOG_ERROR(" *nbands should be 0 or 1"); 
        return ERROR;
    }

    /* Loop through the bands freeing the correct reference bands. */
    for (band_counter = start_index; band_counter < end_index; band_counter++)
    {
        /* free all the pointers for the band */
        geom_grid_band_free(grid, band_counter);
    }
    /* free the gridbands array */
    free(grid->gridbands);
    grid->gridbands = NULL;

    return SUCCESS;
}

/*************************************************************************

NAME: ias_geom_grid_band_free

PURPOSE: Free memory for one grid band pointer data

RETURNS: SUCCESS -- successfully free memory
         ERROR -- error in freeing memory

**************************************************************************/
int ias_geom_grid_band_free 
(
    IAS_GEOM_GRID_TYPE *grid, /* I/O: Grid structure in which band pointers
                                     to be freed             */
    const int band_number     /* I/O: Array of bands to free from the grid   */
)
{
    int band_index;
  
    /* Verify the band exists in the HDF file */
    if (band_number < 1 || band_number > grid->nbands)
    {
        IAS_LOG_ERROR("Band %d is not available", band_number);
        return ERROR;   
    }
    band_index = ias_sat_attr_convert_band_number_to_index(band_number);
    if (band_index == -1)
    {
        IAS_LOG_ERROR("Incorrect band index converted from band number %d",
                              band_number);
        return ERROR;               
    }

    /* free all the pointers for the band */
    geom_grid_band_free(grid, band_index);

    return SUCCESS;
}

/*************************************************************************

NAME: ias_grid_is_geom_grid_file

PURPOSE: Judge whether the file a geometric grid file 

RETURNS: TRUE -- is a geometric grid file
         FALSE -- is not a geometric grid file

**************************************************************************/
int ias_grid_is_geom_grid_file
(
    const char *grid_filename /* I: Grid input file name                    */
)
{
    return ias_misc_check_file_type(grid_filename, FILE_TYPE);
}
