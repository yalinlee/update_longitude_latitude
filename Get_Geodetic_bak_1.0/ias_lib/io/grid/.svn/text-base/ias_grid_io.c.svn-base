/*************************************************************************

NAME: ias_grid_io.c

PURPOSE: Implements the grid input/output library for applications

Algorithm References: None

**************************************************************************
                        Property of the U.S. Government
                            USGS EROS Data Center
**************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <hdf5.h>
#include <hdf5_hl.h>
#include "gctp.h"
#include "ias_logging.h"
#include "ias_const.h"
#include "ias_grid_io.h"              /* Function prototypes */
#include "ias_miscellaneous.h"        /* Header for 2d array allocation */
#include "ias_satellite_attributes.h" /* Header for band conversion*/

#define GRID_FORMAT_VERSION 1
#define GRID_FIELDS 30
#define BAND_FIELDS 15
#define ACTIVE_CORNERS_FIELDS 8
#define NUM_SENSITIVITY_DIRECTIONS 3  /* Roll, pitch, and yaw */

#define FILE_TYPE "Grid File" 
#define GRID_TABLE_TITLE "Grid" 
#define GRID_TABLE_NAME "Grid Record" 
#define GRID_BAND_TABLE_TITLE "Grid Band" 
#define GRID_BAND_TABLE_NAME "Grid Band Record" 
#define GRID_ACTIVE_CORNERS_TABLE_TITLE "Grid Active Corners" 
#define GRID_ACTIVE_CORNERS_TABLE_NAME "Grid Active Corners Record" 

#define INPUT_LINES "in_lines"        /* Band dataset names in actual grid */
#define INPUT_SAMPLES "in_samps"      /* file, associated with fields in   */
#define OUTPUT_LINES "out_lines"      /* the IAS_GRID_BAND_TYPE structure  */
#define OUTPUT_SAMPLES "out_samps"    /* in ias_lib/grid/ias_grid.h        */ 
#define DELTA_LINE_ODD_EVEN "delta_line_oe" 
#define DELTA_SAMPLE_ODD_EVEN "delta_samp_oe" 
#define SATELLITE_TO_PROJECTION "sattoproj" 
#define PROJECTION_TO_SATELLITE "projtosat"
#define POLYNOMIAL_LINES "poly_lines"
#define POLYNOMIAL_SAMPLES "poly_samps" 
#define LINE_SENSITIVITY "line_sensitivity"
#define SAMPLE_SENSITIVITY "samp_sensitivity" 

/* Define the structure for tracking information about an open grid file.
   Note that this is currently only used when writing the file, but it should
   be extended to the reading routines as well at some point. */
struct ias_grid_file
{
    char *filename;     /* filename for error messages */
    hid_t file_id;      /* HDF5 id of the open file */
};

/*************************************************************************
The GRID_BAND_RECORD structure is needed because it is used to build up 
the non-pointer grid band data into a HDF5 grid band data table.
**************************************************************************/
typedef struct GRID_BAND_RECORD
{

    int band_index;         /* band index */
    int sca_number;         /* SCA number */
    struct IAS_LNG_LS grid_cell_size; 
                          /* Number of lines/samples in each grid cell */
    double inv_cell_lines;  /* 1.0/grid_cell_size.line -- an optimization */
    double inv_cell_samps;  /* 1.0/grid_cell_size.samp -- an optimization */
    double pixsize;         /* Projection distance per pixel in meters.
                               Note: This will be the same for x and y
                               dimensions. */
    double zspacing;        /* Elevation difference (in meters) between grid
                               elevation planes */
    int lines;              /* Number of lines in the output image */
    int samps;              /* Number of samples in the output image */
    int ngrid_lines;        /* Number of grid lines through the input image.
                               Note: There are ngrid_lines-1 grid cell rows in
                               each grid. */
    int ngrid_samps;        /* Number of grid columns through the input image.
                             Note: There are ngrid_samps-1 grid cell columns
                             in each grid. */
    int nzplanes;           /* Number of elevation planes in the 3D grid*/
    int zeroplane;          /* Index of the zero elevation plane in the grid */
    int degree;             /* Degree of the rough transformation polynomial.
                             This specifies that there are
                             (degree+1)^2 * nzplanes data elements in the
                             poly_lines and poly_samps ptrs. */
} GRID_BAND_RECORD;

/*************************************************************************
The GRID_RECORD structure is needed because it is used to build up the
grid data only into a HDF5 grid data table.
**************************************************************************/
typedef struct grid_record
{
    /* ==== General grid info. ==== */
    char gridname[GRIDNAME_SIZE];
                            /* Host file name of the grid file */
    int maximum_nscas;      /* Maximum number of SCAs for any band */
    int nbands;             /* Number of bands stored in bands_available */
    int bands_available[IAS_MAX_NBANDS]; /* Array of flags showing the bands
                               available in the grid file */
    int scas_per_band[IAS_MAX_NBANDS]; /* Number of SCAs per band */
    int dtype;              /* data type - this is not used but is kept for
                               backward compatibility */
    char correction_type[25];
                            /* This flag specifies if the grid is a systematic,
                             precision, or terrain corrected grid.
                               IAS_SYSTEMATIC - Systematic.
                               IAS_PRECISION - Precision corrected.
                               IAS_TERRAIN   - Terrain corrected. */
    int satnum;             /* Number of satellite that acquired the data */
    char frame_type[25];    /* Type of framing process to be used.  Valid
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
                               PATH_ORIENTED - Path-oriented framing.        
                               LUNAR -
                               Stellar - */
    /* Newly added grid structure element */
    char acquisition_type[25]; /* Acquisition type: Earth, Lunar, or Stellar */
    /* ==== Projection parameters and info. ==== */
    char units[IAS_UNITS_SIZE]; /* Projection units string */
    int code;               /* Projection code for the output space image.
                             Values for this field are defined in the "gctp.h"
                             include file. */
    char datum[IAS_DATUM_SIZE]; /* Projection datum code string */
    int spheroid;           /* Projection spheroid code */
    int zone;               /* Projection zone code for UTM or State Plane
                             projections. */
    double projprms[IAS_PROJ_PARAM_SIZE];
                          /* Array of 15 projection coefficients as required
                             by the projection transformation package.  Refer
                             to the projection package documentation for a
                             description of each field for a given projection.*/
    struct IAS_CORNERS corners;  /* Projection coordinates of the resulting 
                                  output image's four corners */
    /* ==== Path-oriented info. ==== */
    struct IAS_DBL_LAT_LONG center_pt; 
                            /* Lat/Long of the WRS scene center point. */
    struct IAS_DBL_XY proj_center_pt;   
                            /* Proj x/y of the WRS scene center  point. */
    double rotate_angle;    /* Rotation angle of the path-oriented product.
                             (degrees) */
    double orient_angle;    /* Frame orientation angle in map coords.(degrees)*/
} GRID_RECORD;

/*************************************************************************
The GRID_ACTIVE_CORNERS_RECORD structure is needed because it is used to build 
up the grid active area corners data only into a HDF5 grid data table.
**************************************************************************/
typedef struct grid_active_corners_record
{
    IAS_CORNERS active_area_corners; /* Corner coordinates of the "active"
                               image area, where coverage is available for 
                               each band.  These corners are in "upleft", 
                               "upright", "loleft", "loright" order. */
} GRID_ACTIVE_CORNERS_RECORD;

/*************************************************************************

NAME: ias_grid_initialize

PURPOSE: Initial Grid I/O library by dynamic allocating memory for gridbands.
   If bands or scas are 0, the grid is just set to 0.  Memory is not allocated
   in that case.

RETURNS: SUCCESS / ERROR

**************************************************************************/
int ias_grid_initialize 
(
    IAS_GRID_TYPE *grid, /* O: Grid structure to initialize */
    int bands,           /* I: number of bands in the gridbands array */
    int scas             /* I: maximum number of scas in the gridbands array */
)
{
    if (!((bands == 0) && (scas == 0)) && !((bands == IAS_MAX_NBANDS) && 
        (scas == IAS_MAX_NSCAS)))
    {
        IAS_LOG_ERROR("Invalid parameter bands %d or scas %d; these should "
            "either both be 0 or bands should be %d and scas should be %d",
            bands, scas, IAS_MAX_NBANDS, IAS_MAX_NSCAS);
        return ERROR;
    }

    memset(grid, 0, sizeof(IAS_GRID_TYPE));

    if ((bands != 0) && (scas != 0))
    {
        int band_index;
        int sca_index;

        grid->gridbands = (IAS_GRID_BAND_TYPE **)ias_misc_allocate_2d_array
                   (bands, scas, sizeof(IAS_GRID_BAND_TYPE));
        if (grid->gridbands == NULL)
        {
            IAS_LOG_ERROR("Allocating 2d grid band type structure array");
            return ERROR;
        }

        /* initialize the gridbands */
        for (band_index = 0; band_index < bands; band_index++)
        {
            for (sca_index = 0; sca_index < scas; sca_index++)
            {
                memset(&grid->gridbands[band_index][sca_index], 0, 
                       sizeof(IAS_GRID_BAND_TYPE));
            }
        }

        grid->nbands = bands;
        grid->maximum_nscas = scas;

        /* initialize the number of SCAs for each band */
        for (band_index = 0; band_index < bands; band_index++)
        {
            int band_number 
                = ias_sat_attr_convert_band_index_to_number(band_index);
            if (band_number == ERROR)
            {
                IAS_LOG_ERROR("Invalid band number for band index: %d", 
                    band_index);
                ias_misc_free_2d_array((void **)grid->gridbands);
                grid->gridbands = NULL;
                return ERROR;
            }
            grid->scas_per_band[band_index]
                    = ias_sat_attr_get_scas_per_band(band_number);
            if (grid->scas_per_band[band_index] == ERROR)
            {
                IAS_LOG_ERROR("Invalid number of SCAs retrieved for band %d",
                    band_number);
                ias_misc_free_2d_array((void **)grid->gridbands);
                grid->gridbands = NULL;
                return ERROR;
            }
        }
    }

    return SUCCESS;
}

/*************************************************************************

NAME: flip_SOM_corners_for_read

PURPOSE: Flip the corner points for the SOM projection

RETURNS: None

**************************************************************************/
static void flip_SOM_corners_for_read 
(
    GRID_RECORD *grid  /* I/O: Grid structure to be read */
)
{
    double tmp;      /* Temporary variable for flipping corner points */

    /* Flip the corner points for the SOM projection ONLY */
    if (grid->code == SOM)
    {
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

PURPOSE: Flip the corner points for the SOM projection

RETURNS: None

**************************************************************************/
static void flip_SOM_corners_for_write 
(
    GRID_RECORD *grid  /* I/O: Grid structure to be output */
)
{
    double tmp;      /* Temporary variable for flipping corner points */

    /*  Flip the corner points for the SOM projection ONLY */
    if (grid->code == SOM)
    {
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
NAME: cleanup_grid_table_description

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

/*************************************************************************

NAME: build_grid_band_record_description

PURPOSE: Build grid band data table

RETURNS: SUCCESS -- successfully build grid band record table
         ERROR -- error in building grid band record table

**************************************************************************/
static int build_grid_band_record_description
(
    size_t offsets[BAND_FIELDS],          /* O: Offsets into the structure */
    const char *field_names[BAND_FIELDS], /* O: Names of the fields */
    hid_t field_type[BAND_FIELDS],        /* O: Data types of the fields */
    size_t field_size[BAND_FIELDS]        /* O: Sizes of the fields */
)
{
    int i;
    GRID_BAND_RECORD gbr;     /* for field sizes */

    i = 0;
    offsets[i] = HOFFSET(GRID_BAND_RECORD, band_index);
    field_names[i] = "Band Number";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.band_index);

    /* Newly added grid band structure variable */
    offsets[++i] = HOFFSET(GRID_BAND_RECORD, sca_number);
    field_names[i] = "SCA Number";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.sca_number);
    
    offsets[++i] = HOFFSET(GRID_BAND_RECORD, grid_cell_size.line);
    field_names[i] = "Number of Cell Lines";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.grid_cell_size.line);

    offsets[++i] = HOFFSET(GRID_BAND_RECORD, grid_cell_size.samp);
    field_names[i] = "Number of Cell Samples";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.grid_cell_size.samp);

    offsets[++i] = HOFFSET(GRID_BAND_RECORD, inv_cell_lines);
    field_names[i] = "Cell Lines Inversion";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gbr.inv_cell_lines);

    offsets[++i] = HOFFSET(GRID_BAND_RECORD, inv_cell_samps);
    field_names[i] = "Cell Samples Inversion";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gbr.inv_cell_samps);

    offsets[++i] = HOFFSET(GRID_BAND_RECORD, pixsize);
    field_names[i] = "Projection Pixel Distance";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gbr.pixsize);

    offsets[++i] = HOFFSET(GRID_BAND_RECORD, zspacing);
    field_names[i] = "Grid Elevation Difference";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gbr.zspacing);

    offsets[++i] = HOFFSET(GRID_BAND_RECORD, lines);
    field_names[i] = "Number of Output Lines";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.lines);

    offsets[++i] = HOFFSET(GRID_BAND_RECORD, samps);
    field_names[i] = "Number of Output Samples";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.samps);

    offsets[++i] = HOFFSET(GRID_BAND_RECORD, ngrid_lines);
    field_names[i] = "Number of Input Lines";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.ngrid_lines);

    offsets[++i] = HOFFSET(GRID_BAND_RECORD, ngrid_samps);
    field_names[i] = "Number of Input Samples";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.ngrid_samps);

    offsets[++i] = HOFFSET(GRID_BAND_RECORD, nzplanes);
    field_names[i] = "Number of Elevation Planes";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.nzplanes);

    offsets[++i] = HOFFSET(GRID_BAND_RECORD, zeroplane);
    field_names[i] = "Zero Elevation Plane";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.zeroplane);

    offsets[++i] = HOFFSET(GRID_BAND_RECORD, degree);
    field_names[i] = "Degree";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gbr.degree);

    if (i < (BAND_FIELDS - 1))
    {
        /* fewer fields were added than expected */
        IAS_LOG_ERROR("Defined %d fields, but expected %d", i + 1, BAND_FIELDS);
        return ERROR;
    }
    else if (i >= BAND_FIELDS)
    {
        /* more fields than expected.  The stack is probably now corrupt so
           just exit since this is an obvious programming error that was just
           introduced. */
        IAS_LOG_ERROR("Too many fields found - stack probably corrupted - "
                      "exiting");
        exit(EXIT_FAILURE);
    }

    return SUCCESS;
}

/*************************************************************************

NAME: build_grid_active_corners_record_description

PURPOSE: Build grid active area corners data table

RETURNS: SUCCESS -- successfully build grid active corners record table
         ERROR -- error in building grid active corners record table

**************************************************************************/
static int build_grid_active_corners_record_description
(
    size_t offsets[ACTIVE_CORNERS_FIELDS], /* O: Offsets into the structure */
    const char *field_names[ACTIVE_CORNERS_FIELDS], /* O: Names of the fields */
    hid_t field_type[ACTIVE_CORNERS_FIELDS], /* O: Data types of the fields */
    size_t field_size[ACTIVE_CORNERS_FIELDS] /* O: Sizes of the fields */
)
{
    int i;
    GRID_ACTIVE_CORNERS_RECORD gacr;      /* for field sizes */

    i = 0;
    offsets[i] = HOFFSET(GRID_ACTIVE_CORNERS_RECORD, 
        active_area_corners.upleft.x);
    field_names[i] = "Upper Left X value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gacr.active_area_corners.upleft.x);

    offsets[++i] = HOFFSET(GRID_ACTIVE_CORNERS_RECORD, 
        active_area_corners.upleft.y);
    field_names[i] = "Upper Left Y value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gacr.active_area_corners.upleft.y);

    offsets[++i] = HOFFSET(GRID_ACTIVE_CORNERS_RECORD, 
        active_area_corners.upright.x);
    field_names[i] = "Upper Right X value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gacr.active_area_corners.upright.x);

    offsets[++i] = HOFFSET(GRID_ACTIVE_CORNERS_RECORD, 
        active_area_corners.upright.y);
    field_names[i] = "Upper Right Y value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gacr.active_area_corners.upright.y);

    offsets[++i] = HOFFSET(GRID_ACTIVE_CORNERS_RECORD, 
        active_area_corners.loleft.x);
    field_names[i] = "Lower Left X value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gacr.active_area_corners.loleft.x);

    offsets[++i] = HOFFSET(GRID_ACTIVE_CORNERS_RECORD, 
        active_area_corners.loleft.y);
    field_names[i] = "Lower Left Y value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gacr.active_area_corners.loleft.y);

    offsets[++i] = HOFFSET(GRID_ACTIVE_CORNERS_RECORD, 
        active_area_corners.loright.x);
    field_names[i] = "Lower Right X value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gacr.active_area_corners.loright.x);

    offsets[++i] = HOFFSET(GRID_ACTIVE_CORNERS_RECORD, 
        active_area_corners.loright.y);
    field_names[i] = "Lower Right Y value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gacr.active_area_corners.loright.y);

    if (i < (ACTIVE_CORNERS_FIELDS - 1))
    {
        /* fewer fields were added than expected */
        IAS_LOG_ERROR("Defined %d fields, but expected %d", i + 1, 
            ACTIVE_CORNERS_FIELDS);
        return ERROR;
    }
    else if (i >= ACTIVE_CORNERS_FIELDS)
    {
        /* more fields than expected.  The stack is probably now corrupt so
           just exit since this is an obvious programming error that was just
           introduced. */
        IAS_LOG_ERROR("Too many fields found - stack probably corrupted - "
                      "exiting");
        exit(EXIT_FAILURE);
    }

    return SUCCESS;
}

/*************************************************************************

NAME: build_grid_record_description

PURPOSE: Build grid data table

RETURNS: SUCCESS -- successfully build grid record table
         ERROR -- error in building grid record table

**************************************************************************/
static int build_grid_record_description
(
    size_t offsets[GRID_FIELDS],          /* O: Offsets into the structure */
    const char *field_names[GRID_FIELDS], /* O: Names of the fields */
    hid_t field_type[GRID_FIELDS],        /* O: Data types of the fields */
    hid_t fields_to_close[GRID_FIELDS],   /* O: Fields to be closed */
    size_t field_size[GRID_FIELDS]        /* O: Sizes of the fields */
)
{
    hsize_t parm_dims[1];   /* for data array */    
    int i;
    int error = 0;
    GRID_RECORD gt;         /* for field sizes */

    for (i = 0; i < GRID_FIELDS; i++)
        fields_to_close[i] = -1;

    i = 0;
    offsets[i] = HOFFSET(GRID_RECORD, gridname);
    field_names[i] = "Host Grid Filename";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(gt.gridname)) < 0)
        error = 1;
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(gt.gridname);

    offsets[++i] = HOFFSET(GRID_RECORD, maximum_nscas);
    field_names[i] = "Maximum SCAs";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gt.maximum_nscas);

    offsets[++i] = HOFFSET(GRID_RECORD, nbands);
    field_names[i] = "Number of Bands";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gt.nbands);

    offsets[++i] = HOFFSET(GRID_RECORD, bands_available);
    field_names[i] = "Bands Available";
    parm_dims[0]=sizeof(gt.bands_available)/sizeof(int);
    field_type[i] =   H5Tarray_create(H5T_NATIVE_INT, 1, parm_dims);
    if (field_type[i] < 0)
        error =1;
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(gt.bands_available);

    offsets[++i] = HOFFSET(GRID_RECORD, scas_per_band);
    field_names[i] = "SCAs per Band";
    parm_dims[0]=sizeof(gt.scas_per_band)/sizeof(int);
    field_type[i] =   H5Tarray_create(H5T_NATIVE_INT, 1, parm_dims);
    if (field_type[i] < 0)
        error =1;
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(gt.scas_per_band);

    offsets[++i] = HOFFSET(GRID_RECORD, dtype);
    field_names[i] = "Data Type";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gt.dtype);

    offsets[++i] = HOFFSET(GRID_RECORD, correction_type);
    field_names[i] = "Correction Type";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(gt.correction_type)) < 0)
        error = 1;
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(gt.correction_type);

    offsets[++i] = HOFFSET(GRID_RECORD, satnum);
    field_names[i] = "Number of Satellite";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gt.satnum);

    offsets[++i] = HOFFSET(GRID_RECORD, frame_type);
    field_names[i] = "Frame Type";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(gt.frame_type)) < 0)
        error = 1;
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(gt.frame_type);

    offsets[++i] = HOFFSET(GRID_RECORD, acquisition_type);
    field_names[i] = "Acquisition Type";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(gt.acquisition_type)) < 0)
        error = 1;
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(gt.acquisition_type);
    
    offsets[++i] = HOFFSET(GRID_RECORD, units);
    field_names[i] = "Projection Units";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(gt.units)) < 0)
        error = 1;
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(gt.units);

    offsets[++i] = HOFFSET(GRID_RECORD, code);
    field_names[i] = "Projection Codes";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gt.code);

    offsets[++i] = HOFFSET(GRID_RECORD, datum);
    field_names[i] = "Projection Datum Code";
    field_type[i] = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(field_type[i], sizeof(gt.datum)) < 0)
        error = 1;
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(gt.datum);

    offsets[++i] = HOFFSET(GRID_RECORD, spheroid);
    field_names[i] = "Projection Spheroid Code";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gt.code);

    offsets[++i] = HOFFSET(GRID_RECORD, zone);
    field_names[i] = "Projection Zone Code";
    field_type[i] = H5T_NATIVE_INT;
    field_size[i] = sizeof(gt.zone);

    offsets[++i] = HOFFSET(GRID_RECORD, projprms);
    field_names[i] = "Array of 15 Projection Coefficients";
    parm_dims[0]=sizeof(gt.projprms)/sizeof(double);
    field_type[i] =   H5Tarray_create(H5T_NATIVE_DOUBLE, 1, parm_dims);
    if (field_type[i] < 0)
        error =1;
    fields_to_close[i] = field_type[i];
    field_size[i] = sizeof(gt.projprms);

    offsets[++i] = HOFFSET(GRID_RECORD, corners.upleft.x);
    field_names[i] = "Upper Left X value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.corners.upleft.x);

    offsets[++i] = HOFFSET(GRID_RECORD, corners.upleft.y);
    field_names[i] = "Upper Left Y value";
    field_type[i] =  H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.corners.upleft.y);

    offsets[++i] = HOFFSET(GRID_RECORD, corners.upright.x);
    field_names[i] = "Upper Right X value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.corners.upright.x);

    offsets[++i] = HOFFSET(GRID_RECORD, corners.upright.y);
    field_names[i] = "Upper Right Y value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.corners.upright.y);

    offsets[++i] = HOFFSET(GRID_RECORD, corners.loleft.x);
    field_names[i] = "Lower Left X value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.corners.loleft.x);

    offsets[++i] = HOFFSET(GRID_RECORD, corners.loleft.y);
    field_names[i] = "Lower Left Y value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.corners.loleft.y);

    offsets[++i] = HOFFSET(GRID_RECORD, corners.loright.x);
    field_names[i] = "Lower Right X value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.corners.loright.x);

    offsets[++i] = HOFFSET(GRID_RECORD, corners.loright.y);
    field_names[i] = "Lower Right Y value";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.corners.loright.y);

    offsets[++i] = HOFFSET(GRID_RECORD, center_pt.lat);
    field_names[i] = "WRS Scene Center Latitude";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.center_pt.lat);

    offsets[++i] = HOFFSET(GRID_RECORD, center_pt.lng);
    field_names[i] = "WRS Scene Center Longitude";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.center_pt.lng);

    offsets[++i] = HOFFSET(GRID_RECORD, proj_center_pt.x);
    field_names[i] = "WRS Scene Center Projection X";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.proj_center_pt.x);

    offsets[++i] = HOFFSET(GRID_RECORD, proj_center_pt.y);
    field_names[i] = "WRS Scene Center Projection Y";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.proj_center_pt.y);

    offsets[++i] = HOFFSET(GRID_RECORD, rotate_angle);
    field_names[i] = "Rotation Angle";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.rotate_angle);

    offsets[++i] = HOFFSET(GRID_RECORD, orient_angle);
    field_names[i] = "Frame Orientation Angle";
    field_type[i] = H5T_NATIVE_DOUBLE;
    field_size[i] = sizeof(gt.orient_angle);

    if (error)
    {
        /* an error occurred generating the table */
        IAS_LOG_ERROR("Building grid table definition");
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
        IAS_LOG_ERROR("Too many fields found - stack probably corrupted - "
                      "exiting");
        exit(EXIT_FAILURE);
    }

    return SUCCESS;
}

/*************************************************************************

NAME: ias_grid_active_corners_header_write

PURPOSE: Write grid active area corners data to HDF5 table

RETURNS: SUCCESS -- successfully write grid active area corners header
         ERROR -- error in writing grid active area corners header

**************************************************************************/
static int ias_grid_active_corners_header_write
(
    IAS_GRID_FILE *file,        /* I: Grid file pointer */
    const IAS_GRID_TYPE *grid   /* I: Grid structure to write */
)
{
    size_t offsets[ACTIVE_CORNERS_FIELDS]; /* These arrays are used to set */
    const char *field_names[ACTIVE_CORNERS_FIELDS]; /* up the definition of the 
                                             grid. */
    hid_t field_type[ACTIVE_CORNERS_FIELDS]; /* They will be filled by calls */
    size_t field_size[ACTIVE_CORNERS_FIELDS]; /* to the appropriate table
                                             description build routine. */
    int status;                           /* Return status of the description 
                                             build routines */
    size_t total_size;                    /* Total size of the one record */
    GRID_ACTIVE_CORNERS_RECORD corners[grid->nbands]; 
                                          /* Subset of the grid structure 
                                             containing members that go into a 
                                             general table. */
    int band_index;

    memset(corners, 0, sizeof(corners));

    /* Copy the input grid information to the internal structure members to be 
       written to the table fields in the HDF5 file */
    for (band_index = 0; band_index < grid->nbands; band_index++)
    {
        corners[band_index].active_area_corners 
            = grid->active_corners[band_index];
    }

    /* Build up the grid band record table */
    if (build_grid_active_corners_record_description(offsets, field_names, 
                field_type, field_size) == ERROR)
    {
        IAS_LOG_ERROR("A problem has occurred in building the active corners"
            " table for: %s", file->filename);
        return ERROR;
    }

    total_size = sizeof(GRID_ACTIVE_CORNERS_RECORD);

    /* Build Up the Grid Band Table */
    status = H5TBmake_table(GRID_ACTIVE_CORNERS_TABLE_TITLE, file->file_id, 
               GRID_ACTIVE_CORNERS_TABLE_NAME, ACTIVE_CORNERS_FIELDS, 
               grid->nbands, total_size, field_names, offsets, field_type, 1, 
               NULL, 0, corners);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing active corners record table: %s", 
            file->filename);
        return ERROR;
    }
   
    return SUCCESS;
}

/*************************************************************************

NAME: ias_grid_active_corners_header_read

PURPOSE: Read grid active area corners data from HDF5 table

RETURNS: SUCCESS -- successfully read grid active area corners header
         ERROR -- error in reading grid active area corners header

**************************************************************************/
static int ias_grid_active_corners_header_read
(
    hid_t file_id,            /* Open file handle for HDF */
    const char *grid_filename,/* I: Grid file name */
    IAS_GRID_TYPE *grid       /* O: Grid structure to read */
)
{
    size_t offsets[ACTIVE_CORNERS_FIELDS]; /* These arrays are used to set up */
    const char *field_names[ACTIVE_CORNERS_FIELDS]; /* the definition of the  */
                                           /* grid.  They will be filled by */
    hid_t field_type[ACTIVE_CORNERS_FIELDS]; /* calls to the appropriate */
    size_t field_size[ACTIVE_CORNERS_FIELDS];/* table description build 
                                              routine. */
    int status;                           /* Return status of the description 
                                             build routines */
    size_t total_size;                    /* Total size of the table */
    GRID_ACTIVE_CORNERS_RECORD corners[grid->nbands]; 
                                          /* Subset of the grid structure 
                                             containing members that go into a 
                                             general table. */
    int band_index;
    hsize_t number_of_fields;
    hsize_t number_of_records;
    int expected_records = 0;

    /* calculate the total number of records to expect */
    expected_records = grid->nbands;

    /* Get the grid table information */
    status = H5TBget_table_info(file_id, GRID_ACTIVE_CORNERS_TABLE_NAME,
        &number_of_fields, &number_of_records);
    if (status < 0)
    {
       IAS_LOG_ERROR("Could not get grid active records table information: %s",
           grid_filename);
       return ERROR;
    }

    if (number_of_records != expected_records)
    {
        IAS_LOG_ERROR("Incorrect number of records: %s, "
            "expected %d got %lld", grid_filename, expected_records,
            number_of_records);
        return ERROR;
    }

    if (number_of_fields != ACTIVE_CORNERS_FIELDS)
    {
        IAS_LOG_ERROR("Incorrect number of fields: %s, "
            "expected %d got %lld", grid_filename, ACTIVE_CORNERS_FIELDS,
            number_of_fields);
        return ERROR;
    }

    /* Build the grid active corners record table */
    if (build_grid_active_corners_record_description(offsets, field_names, 
                field_type, field_size) == ERROR)
    {
        IAS_LOG_ERROR("Building Active Corners Record Table: %s", 
                grid_filename);
        return ERROR;
    }

    total_size = sizeof(GRID_ACTIVE_CORNERS_RECORD);

    /* Read out the grid active corners table */
    if (H5TBread_table(file_id, GRID_ACTIVE_CORNERS_TABLE_NAME, total_size,
            offsets, field_size, corners) < 0)
    {
        IAS_LOG_ERROR("Reading Grid Active Corners Table: %s", grid_filename);
        return ERROR;
    }

    /* Populate the contents that were read from the table fields to the final 
       grid structure */
    for (band_index = 0; band_index < grid->nbands; band_index++)
    {
        grid->active_corners[band_index]
            = corners[band_index].active_area_corners;
    }

    return SUCCESS;
}

/*************************************************************************

NAME: ias_grid_header_read

PURPOSE: Read out grid data from HDF5 header

RETURNS: SUCCESS -- successfully read grid header
         ERROR -- error in reading grid header

**************************************************************************/
int ias_grid_header_read
(
    const char *grid_filename,/* I: Grid file name */
    IAS_GRID_TYPE *grid   /* O: Grid structure to read */
)
{
    hid_t file_id;                        /* File handle for HDF */
    size_t offsets[GRID_FIELDS];          /* These arrays are used to set   */
    const char *field_names[GRID_FIELDS]; /* up the definition of the grid. */
    hid_t field_type[GRID_FIELDS];        /* They will be filled by calls   */
    size_t field_size[GRID_FIELDS];       /* to the appropriate table       */
    hid_t fields_to_close[GRID_FIELDS];   /* description build routine.     */
    int status;                           /* Return status of the description 
                                             build routines */
    size_t total_size;                    /* Total size of the table */
    GRID_RECORD grid_data;                /* Subset of the model structure 
                                             containing members that go into a 
                                             general table. */
    hsize_t number_of_fields;
    hsize_t number_of_records;
    int i;

    /* Open the input file */
    file_id = H5Fopen(grid_filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file_id < 0)
    {
        IAS_LOG_ERROR("Opening grid file: %s", grid_filename);
        return ERROR;
    }

    /* Get the grid table information */
    status = H5TBget_table_info(file_id, GRID_TABLE_NAME,
        &number_of_fields, &number_of_records);
    if (status < 0)
    {
        IAS_LOG_ERROR("Could not get grid table information: %s",grid_filename);
        H5Fclose(file_id);
        return ERROR;
    }

    if (number_of_records != 1)
    {
        IAS_LOG_ERROR("Incorrect number of records: %s",grid_filename);
        H5Fclose(file_id);
        return ERROR;
    }

    /* Build the grid record table */
    if (build_grid_record_description(offsets, field_names, field_type,
                fields_to_close, field_size) == ERROR)
    {
        IAS_LOG_ERROR("Building up Grid Record Table: %s", grid_filename);
        H5Fclose(file_id);
        return ERROR;
    }

    total_size = sizeof(GRID_RECORD);

    /* Read the table, can also use "H5TBread_table" instead */
    if (H5TBread_records(file_id, GRID_TABLE_NAME, 0, 1, total_size,
        offsets, field_size, &grid_data) < 0)
    {
        IAS_LOG_ERROR("Failed Reading Grid Record Table: %s", grid_filename);
        cleanup_grid_table_description(fields_to_close);
        H5Fclose(file_id);
        return ERROR;
    }

    /* Clean up the grid record table */
    cleanup_grid_table_description(fields_to_close);

    /* Populate nbands which is used by the active corners header read */
    grid->nbands = grid_data.nbands;

    /* Read grid active corners header */
    if (ias_grid_active_corners_header_read(file_id, grid_filename, grid) 
        != SUCCESS)
    {
        IAS_LOG_ERROR("Reading grid active corners header: %s", grid_filename);
        H5Fclose(file_id);
        return ERROR;
    }

    /* Close the HDF5 File */
    H5Fclose(file_id);

    /* flip the coords if the projection is SOM */
    flip_SOM_corners_for_read(&grid_data);

    /* Populate the contents that were read from the table fields to the final 
       grid structure */
    strcpy(grid->gridname, grid_data.gridname);
    grid->maximum_nscas = grid_data.maximum_nscas;

    for (i = 0; i < grid->nbands; i++)
    {
        grid->bands_available[i] = grid_data.bands_available[i];
        grid->scas_per_band[i] = grid_data.scas_per_band[i];
    }

    grid->dtype = grid_data.dtype;

    if (strcasecmp(grid_data.correction_type, "IAS_SYSTEMATIC") == 0)
        grid->correction_type = 0;
    else if (strcasecmp(grid_data.correction_type, "IAS_PRECISION") == 0)
        grid->correction_type = 1;
    else if (strcasecmp(grid_data.correction_type, "IAS_TERRAIN") == 0)
        grid->correction_type = 2;
    else
    {
        IAS_LOG_ERROR("Unsupported correction type %s in %s", 
                 grid_data.correction_type, grid_filename);
        return ERROR;
    }

    grid->satnum = grid_data.satnum;

    if (strcasecmp(grid_data.frame_type, "IAS_GEOBOX") == 0)
        grid->frame_type = IAS_GEOBOX;
    else if (strcasecmp(grid_data.frame_type, "IAS_PROJBOX") == 0)
        grid->frame_type = IAS_PROJBOX;
    else if (strcasecmp(grid_data.frame_type, "IAS_UL_SIZE") == 0)
        grid->frame_type = IAS_UL_SIZE;
    else if (strcasecmp(grid_data.frame_type, "IAS_MINBOX") == 0)
        grid->frame_type = IAS_MINBOX;
    else if (strcasecmp(grid_data.frame_type, "IAS_MAXBOX") == 0)
        grid->frame_type = IAS_MAXBOX;
    else if (strcasecmp(grid_data.frame_type, "IAS_PATH_ORIENTED") == 0)
        grid->frame_type = IAS_PATH_ORIENTED;
    else if (strcasecmp(grid_data.frame_type, "IAS_PATH_MINBOX") == 0)
        grid->frame_type = IAS_PATH_MINBOX;
    else if (strcasecmp(grid_data.frame_type, "IAS_PATH_MAXBOX") == 0)
        grid->frame_type = IAS_PATH_MAXBOX;
    else if (strcasecmp(grid_data.frame_type, "IAS_LUNAR_MINBOX") == 0)
        grid->frame_type = IAS_LUNAR_MINBOX;
    else if (strcasecmp(grid_data.frame_type, "IAS_LUNAR_MAXBOX") == 0)
        grid->frame_type = IAS_LUNAR_MAXBOX;
    else if (strcasecmp(grid_data.frame_type, "IAS_STELLAR_FRAME") == 0)
        grid->frame_type = IAS_STELLAR_FRAME;
    else
    {
        IAS_LOG_ERROR("Unsupported frame type %s in %s", 
                 grid_data.frame_type, grid_filename);
        return ERROR;
    }

    if (strcasecmp(grid_data.acquisition_type, "IAS_EARTH") == 0)
        grid->acquisition_type = 0;
    else if (strcasecmp(grid_data.acquisition_type, "IAS_LUNAR") == 0)
        grid->acquisition_type = 1;
    else if (strcasecmp(grid_data.acquisition_type, "IAS_STELLAR") == 0)
        grid->acquisition_type = 2;
    else
    {
        IAS_LOG_ERROR("Unsupported acquisition type %s in %s", 
                 grid_data.acquisition_type, grid_filename);
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

NAME: ias_grid_header_write

PURPOSE: Write grid data into HDF5 table

RETURNS: SUCCESS -- successfully write grid header
         ERROR -- error in writing grid header

**************************************************************************/
int ias_grid_header_write
(
    IAS_GRID_FILE *file,        /* I: Grid file pointer */
    const IAS_GRID_TYPE *grid   /* I: Grid structure to write */
)
{
    size_t offsets[GRID_FIELDS];          /* These arrays are used to set   */
    const char *field_names[GRID_FIELDS]; /* up the definition of the grid. */
    hid_t field_type[GRID_FIELDS];        /* They will be filled by calls   */
    size_t field_size[GRID_FIELDS];       /* to the appropriate table       */
    hid_t fields_to_close[GRID_FIELDS];   /* description build routine.     */
    int status;                           /* Return status of the description 
                                             build routines */
    size_t total_size;                    /* Total size of the table */
    GRID_RECORD grid_data;                /* Subset of the model structure 
                                             containing members that go into a 
                                             general table. */
    int i;

    /* Initialize the structure */
    memset(&grid_data, 0, sizeof(grid_data));    
 
    /* Copy the input grid information to the internal structure members to be 
       written to the table fields in the HDF5 file */
    strcpy(grid_data.gridname, grid->gridname);
    grid_data.maximum_nscas = grid->maximum_nscas;
    grid_data.nbands = grid->nbands;
    for (i = 0; i < grid->nbands; i++)
    {
        grid_data.bands_available[i] = grid->bands_available[i];
        grid_data.scas_per_band[i] = grid->scas_per_band[i];
    }

    grid_data.dtype = grid->dtype;
    switch (grid->correction_type)
    {
        case 0 :
            strcpy(grid_data.correction_type, "IAS_SYSTEMATIC");
            break;
        case 1 :
            strcpy(grid_data.correction_type, "IAS_PRECISION");
            break;
        case 2 :
            strcpy(grid_data.correction_type, "IAS_TERRAIN");
            break;
        default:
            IAS_LOG_ERROR("Unsupported correction type: %s", file->filename);
            return ERROR;
    }

    grid_data.satnum = grid->satnum;

    switch (grid->frame_type)
    {
        case IAS_GEOBOX:
            strcpy(grid_data.frame_type, "IAS_GEOBOX");
            break;
        case IAS_PROJBOX:
            strcpy(grid_data.frame_type, "IAS_PROJBOX");
            break;
        case IAS_UL_SIZE:
            strcpy(grid_data.frame_type, "IAS_UL_SIZE");
            break;
        case IAS_MINBOX:
            strcpy(grid_data.frame_type, "IAS_MINBOX");
            break;
        case IAS_MAXBOX:
            strcpy(grid_data.frame_type, "IAS_MAXBOX");
            break;
        case IAS_PATH_ORIENTED:
            strcpy(grid_data.frame_type, "IAS_PATH_ORIENTED");
            break;
        case IAS_PATH_MINBOX:
            strcpy(grid_data.frame_type, "IAS_PATH_MINBOX");
            break;
        case IAS_PATH_MAXBOX:
            strcpy(grid_data.frame_type, "IAS_PATH_MAXBOX");
            break;
        case IAS_LUNAR_MINBOX:
            strcpy(grid_data.frame_type, "IAS_LUNAR_MINBOX");
            break;
        case IAS_LUNAR_MAXBOX:
            strcpy(grid_data.frame_type, "IAS_LUNAR_MAXBOX");
            break;
        case IAS_STELLAR_FRAME:
            strcpy(grid_data.frame_type, "IAS_STELLAR_FRAME");
            break;
        default:
            IAS_LOG_ERROR("Unsupported frame type: %d", grid->frame_type);
            return ERROR;
    }

    switch (grid->acquisition_type)
    {
        case 0 :
            strcpy(grid_data.acquisition_type, "IAS_EARTH");
            break;
        case 1 :
            strcpy(grid_data.acquisition_type, "IAS_LUNAR");
            break;
        case 2 :
            strcpy(grid_data.acquisition_type, "IAS_STELLAR");
            break;
        default:
            IAS_LOG_ERROR("Unsupported acquisition type: %d",
                    grid->acquisition_type);
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

    /* Build the grid record table */
    if (build_grid_record_description(offsets, field_names, field_type,
                fields_to_close, field_size) == ERROR)
    {
        IAS_LOG_ERROR("A problem has occurred in building the table: %s", 
                            file->filename);
        return ERROR;
    }

    total_size = sizeof(GRID_RECORD);

    /* Create the Grid Record Table     */
    status = H5TBmake_table(GRID_TABLE_TITLE, file->file_id, 
                            GRID_TABLE_NAME, GRID_FIELDS, 1, total_size,
                             field_names, offsets, field_type, 1, NULL, 0,
                             &grid_data);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing grid table: %s", file->filename);
        cleanup_grid_table_description(fields_to_close);
        return ERROR;
    }   

    /* Clean up the grid record table */
    cleanup_grid_table_description(fields_to_close);

    /* Write grid active area corners header */
    if (ias_grid_active_corners_header_write(file, grid) != SUCCESS)
    {
        IAS_LOG_ERROR("Writing grid active area corners header: %s", 
            file->filename);
        ias_grid_close(file);
        return ERROR;
    }

    return SUCCESS;
}

/*************************************************************************

NAME: ias_grid_band_header_read

PURPOSE: Read out grid band data from HDF5 table

RETURNS: SUCCESS -- successfully read grid band header
         ERROR -- error in reading grid band header

NOTES: Grid band table has BANDS*NSCA sets of GRID_BAND_RECORD data

**************************************************************************/
int ias_grid_band_header_read
(
    const char *grid_filename,/* I: Grid file name*/
    IAS_GRID_TYPE *grid       /* O: Grid structure to read */
)
{
    hid_t file_id;                        /* File handle for HDF */
    size_t offsets[BAND_FIELDS];          /* These arrays are used to set   */
    const char *field_names[BAND_FIELDS]; /* up the definition of the grid. */
    hid_t field_type[BAND_FIELDS];        /* They will be filled by calls   */
    size_t field_size[BAND_FIELDS];       /* to the appropriate table
                                             description build routine.     */
    int status;                           /* Return status of the description 
                                             build routines */
    size_t total_size;                    /* Total size of the table */
    GRID_BAND_RECORD band[grid->nbands*grid->maximum_nscas]; 
                                          /* Subset of the model structure 
                                             containing members that go into a 
                                             general table. */
    int band_index;
    int sca_index;
    GRID_BAND_RECORD *record_ptr;          /* pointer to a read record */
    hsize_t number_of_fields;
    hsize_t number_of_records;
    int expected_records = 0;

    /* calculate the total number of records to expect */
    for (band_index = 0; band_index < grid->nbands; band_index++)
        expected_records += grid->scas_per_band[band_index];

    /* Open the input file */
    file_id =  H5Fopen(grid_filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file_id < 0)
    {
        IAS_LOG_ERROR("Opening grid file: %s", grid_filename);
        return ERROR;
    }

    /* Get the grid table information */
    status = H5TBget_table_info(file_id, GRID_BAND_TABLE_NAME,
        &number_of_fields, &number_of_records);
    if (status < 0)
    {
        IAS_LOG_ERROR("Could not get grid band table information: %s", 
                           grid_filename);
        H5Fclose(file_id);        
        return ERROR;
    }

    if (number_of_records != expected_records)
    {
        IAS_LOG_ERROR("Incorrect number of records: %s, "
            "expected %d got %lld", grid_filename, expected_records,
            number_of_records);
        H5Fclose(file_id);        
        return ERROR;
    }

    /* Build the grid band record table */
    if (build_grid_band_record_description(offsets, field_names, field_type,
                field_size) == ERROR)
    {
        IAS_LOG_ERROR("Building Band Record Table: %s", grid_filename);
        H5Fclose(file_id);        
        return ERROR;
    }

    total_size = sizeof(GRID_BAND_RECORD);

    /* Read out the grid band table */
    if (H5TBread_table(file_id, GRID_BAND_TABLE_NAME, total_size,
            offsets, field_size, band) < 0)
    {
        IAS_LOG_ERROR("Reading Grid Band Table: %s", grid_filename);
        H5Fclose(file_id);        
        return ERROR;
    }

    /* Close the HDF5 File */
    H5Fclose(file_id);

    /* Allocate memory for 2d gridbands array */
    if (grid->gridbands == NULL)
    {
        grid->gridbands = (IAS_GRID_BAND_TYPE **)ias_misc_allocate_2d_array
               (grid->nbands, grid->maximum_nscas, sizeof(IAS_GRID_BAND_TYPE));
        if (grid->gridbands == NULL)
        {
            IAS_LOG_ERROR("Allocating 2d grid band type structure array");
            H5Fclose(file_id);        
            return ERROR;
        }
    }

    /* Populate the contents that were read from the table fields to the final 
       grid structure */
    record_ptr = band;
    for (band_index = 0; band_index < grid->nbands; band_index++)
    {
        for (sca_index = 0; sca_index < grid->scas_per_band[band_index];
             sca_index++)
        {
            grid->gridbands[band_index][sca_index].band_index 
                    = record_ptr->band_index;
            grid->gridbands[band_index][sca_index].sca_number 
                    = record_ptr->sca_number;
            grid->gridbands[band_index][sca_index].grid_cell_size.line 
                     = record_ptr->grid_cell_size.line;
            grid->gridbands[band_index][sca_index].grid_cell_size.samp 
                     = record_ptr->grid_cell_size.samp;
            grid->gridbands[band_index][sca_index].inv_cell_lines 
                     = record_ptr->inv_cell_lines;
            grid->gridbands[band_index][sca_index].inv_cell_samps
                     = record_ptr->inv_cell_samps;
            grid->gridbands[band_index][sca_index].pixsize 
                     = record_ptr->pixsize;
            grid->gridbands[band_index][sca_index].zspacing 
                     = record_ptr->zspacing;
            grid->gridbands[band_index][sca_index].lines = record_ptr->lines;
            grid->gridbands[band_index][sca_index].samps = record_ptr->samps;
            grid->gridbands[band_index][sca_index].ngrid_lines 
                     = record_ptr->ngrid_lines;
            grid->gridbands[band_index][sca_index].ngrid_samps 
                     = record_ptr->ngrid_samps;
            grid->gridbands[band_index][sca_index].nzplanes 
                     = record_ptr->nzplanes;
            grid->gridbands[band_index][sca_index].zeroplane 
                     = record_ptr->zeroplane;
            grid->gridbands[band_index][sca_index].degree 
                     = record_ptr->degree;
            record_ptr++;
        }
    }

    return SUCCESS;
}


/*************************************************************************

NAME: ias_grid_band_header_write

PURPOSE: Write grid data into HDF5 table

RETURNS: SUCCESS -- successfully write grid band header
         ERROR -- error in writing grid band header

NOTES: Grid band table has BANDS*NSCA sets of GRID_BAND_RECORD data

**************************************************************************/
int ias_grid_band_header_write
(
    IAS_GRID_FILE *file,        /* I: Grid file pointer */
    const IAS_GRID_TYPE *grid   /* I: Grid structure to write */
)
{
    size_t offsets[BAND_FIELDS];          /* These arrays are used to set   */
    const char *field_names[BAND_FIELDS]; /* up the definition of the grid. */
    hid_t field_type[BAND_FIELDS];        /* They will be filled by calls   */
    size_t field_size[BAND_FIELDS];       /* to the appropriate table
                                             description build routine.     */
    int status;                           /* Return status of the description 
                                             build routines */
    size_t total_size;                    /* Total size of the one record */
    GRID_BAND_RECORD band[grid->nbands*grid->maximum_nscas]; 
                                          /* Subset of the model structure 
                                             containing members that go into a 
                                             general table. */
    GRID_BAND_RECORD *record_ptr;         /* pointer to a record */
    int band_index;
    int sca_index;
    int total_records = 0;

    memset(band, 0, sizeof(band));

    /* Copy the input grid information to the internal structure members to be 
       written to the table fields in the HDF5 file */
    record_ptr = band;
    for (band_index = 0; band_index < grid->nbands; band_index++)
    {
        for (sca_index = 0; sca_index < grid->scas_per_band[band_index];
             sca_index++)
        {
            record_ptr->band_index 
              = grid->gridbands[band_index][sca_index].band_index;
            record_ptr->sca_number 
              = grid->gridbands[band_index][sca_index].sca_number;
            record_ptr->grid_cell_size.line 
              = grid->gridbands[band_index][sca_index].grid_cell_size.line;
            record_ptr->grid_cell_size.samp 
              = grid->gridbands[band_index][sca_index].grid_cell_size.samp;
            record_ptr->inv_cell_lines 
              = grid->gridbands[band_index][sca_index].inv_cell_lines;
            record_ptr->inv_cell_samps
              = grid->gridbands[band_index][sca_index].inv_cell_samps;
            record_ptr->pixsize
              = grid->gridbands[band_index][sca_index].pixsize;
            record_ptr->zspacing 
              = grid->gridbands[band_index][sca_index].zspacing;
            record_ptr->lines = grid->gridbands[band_index][sca_index].lines;
            record_ptr->samps = grid->gridbands[band_index][sca_index].samps;
            record_ptr->ngrid_lines 
              = grid->gridbands[band_index][sca_index].ngrid_lines;
            record_ptr->ngrid_samps 
              = grid->gridbands[band_index][sca_index].ngrid_samps;
            record_ptr->nzplanes 
              = grid->gridbands[band_index][sca_index].nzplanes;
            record_ptr->zeroplane 
              = grid->gridbands[band_index][sca_index].zeroplane;
            record_ptr->degree 
              = grid->gridbands[band_index][sca_index].degree;
            total_records++;
            record_ptr++;
        }
    }

    /* Build up the grid band record table */
    if (build_grid_band_record_description(offsets, field_names, field_type,
                field_size) == ERROR)
    {
        IAS_LOG_ERROR("A problem has occurred in building the table: %s", 
                            file->filename);
        return ERROR;
    }

    total_size = sizeof(GRID_BAND_RECORD);

    /* Build Up the Grid Band Table */
    status = H5TBmake_table(GRID_BAND_TABLE_TITLE, file->file_id, 
               GRID_BAND_TABLE_NAME, BAND_FIELDS, total_records, 
               total_size, field_names, offsets, field_type, 1, NULL, 0, band);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing band record table: %s", file->filename);
        return ERROR;
    }
   
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
    hid_t dataset_type,       /* I: dataset data type */
    const char *dataset_name, /* I: dataset name */   
    int offset,               /* I: offset for the dataset */
    int expected_dataset_size,/* I: expected dataset size to read */
    IAS_GRID_TYPE *grid       /* O: grid structure to read */
)
{
    hid_t dataset_id, dataspace_id;   /* identifiers */
    hsize_t ndims;                    /* Number of row dimensions in array */
    hsize_t dims[2];                  /* The size of each dimension */
    hsize_t data_dims[1];    /* size of the data buffer */
    hsize_t file_size[2];    /* slab size to read from the file */
    hsize_t file_offset[2];  /* location to read in the file */
    herr_t status;           /* HDF5 return status */
    hid_t memspace_id, filespace_id; 
    int sca_index;
    void **member_ptr;
    void *dataset;

    /* Open the dataset */
    dataset_id = H5Dopen(group_id, dataset_name, H5P_DEFAULT);
    if (dataset_id < 0)
    {
        IAS_LOG_ERROR("Opening dataset: %s", dataset_name);
        return ERROR;     
    }

    /* Get the data space and its dimensions */
    dataspace_id = H5Dget_space(dataset_id);
    if (dataspace_id < 0)
    {
        IAS_LOG_ERROR("Getting dataspace for dataset : %s", dataset_name);
        H5Dclose(dataset_id);
        return ERROR;     
    }

    ndims = H5Sget_simple_extent_ndims(dataspace_id);

    /* Check dataset space number of dimensions */
    if (ndims != 2)
    {
        IAS_LOG_ERROR("Incorrect %s number of dimensions %llu", dataset_name,
            ndims);
        H5Sclose(dataspace_id);
        H5Dclose(dataset_id);
        return ERROR;
    }

    status = H5Sget_simple_extent_dims(dataspace_id, dims, NULL);
    if (status < 0)
    {
        IAS_LOG_ERROR("Getting dimensions for dataset %s", dataset_name);
        H5Sclose(dataspace_id);
        H5Dclose(dataset_id);
        return ERROR;
    }

    /* Check dataset space dimension sizes */
    if (dims[0] != grid->scas_per_band[band_index] || 
        dims[1] != expected_dataset_size)
    {
        IAS_LOG_ERROR("Incorrect %s dimensions %llux%llu, should be %dx%d", 
            dataset_name, dims[0], dims[1], grid->scas_per_band[band_index], 
            expected_dataset_size);
        H5Sclose(dataspace_id);
        H5Dclose(dataset_id);
        return ERROR;
    }
    H5Sclose(dataspace_id);

    data_dims[0] = dims[1];
    /* define the memory dataspace for the buffer to be written */
    memspace_id = H5Screate_simple(1, data_dims, NULL);
    if (memspace_id < 0)
    {
        IAS_LOG_ERROR("Creating memory dataspace");
        H5Dclose(dataset_id);
        return ERROR;
    }

    /* Loop through each SCA */
    for (sca_index = 0; sca_index < grid->scas_per_band[band_index];
         sca_index++)
    {

        /* Define memory hyperslab */
        file_size[0] = 1;
        file_size[1] = dims[1];
        file_offset[0] = sca_index;
        file_offset[1] = 0;

        /* build the file space structure for the correct location */
        filespace_id = H5Dget_space(dataset_id);
        if (filespace_id < 0)
        {
            IAS_LOG_ERROR("Getting dataspace for band %d in dataset %s", 
                          band_index+1, dataset_name);
            H5Sclose(memspace_id);
            H5Dclose(dataset_id);
            return ERROR;
        }

        status = H5Sselect_hyperslab(filespace_id, H5S_SELECT_SET, 
                file_offset, NULL, file_size, NULL);
        if (status < 0)
        {
            IAS_LOG_ERROR("Getting dataspace for band %d in dataset %s", 
                          band_index+1, dataset_name); 
            H5Sclose(memspace_id);
            H5Sclose(filespace_id);
            H5Dclose(dataset_id);
            return ERROR;
        }
        
        /* Calculating the address of the particular array to read in
           the grid band structure by using offset as an indication of
           which member of the structure is being read, member_ptr is
           a void** because the dataset pointer will indicate the actual
           buffer and its type to read into */
        member_ptr = (void *)((unsigned char*)
                    &grid->gridbands[band_index][sca_index]) + offset;
        dataset = *member_ptr; 

        /* Read in one SCA of the dataset */
        status = H5Dread(dataset_id, dataset_type, memspace_id, 
                         filespace_id, H5P_DEFAULT, dataset);
        if (status < 0)
        {
            IAS_LOG_ERROR("Reading the %s dataset", dataset_name);
            H5Sclose(memspace_id);
            H5Sclose(filespace_id);
            H5Dclose(dataset_id);
            return ERROR;
        }
        H5Sclose(filespace_id);
    }

    H5Sclose(memspace_id);
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
    int band_index,         /* I: band index for current band */
    hid_t dataset_type,     /* I: dataset data type */
    char *dataset_name,     /* I: dataset name */
    int offset,             /* I: offset of dataset */
    int dataset_size,       /* I: dataset size of write out dataset */
    const IAS_GRID_TYPE *grid  /* I: grid structure to write */
)
{
    hid_t dataset_id, dataspace_id;   /* identifiers */
    hsize_t dims[2];                  /* Number of row dimensions in array */
    hsize_t data_dims[1];    /* size of the data buffer */
    hsize_t file_size[2];    /* slab size to read from the file */
    hsize_t file_offset[2];  /* location to read in the file */
    herr_t status;           /* HDF5 return status */
    hid_t memspace_id, filespace_id;  /* space handlers */
    int sca_index;                  /* scene counter */
    void **member_ptr;                /* pointer for getting offset location */
    void *dataset;                    /* dataset buffer pointer */

    /* define the dataset data space dimension sizes */
    dims[0] = grid->scas_per_band[band_index];      
    dims[1] = dataset_size;      

    /* Create the data space for dataset */
    dataspace_id = H5Screate_simple(2, dims, NULL);
    if (dataspace_id < 0)
    {
        IAS_LOG_ERROR("Creating data space for dataset %s, band index %d",
                      dataset_name, band_index);
        return ERROR;
    }

    /* Create dataset in group "B#" */
    dataset_id = H5Dcreate(group_id, dataset_name, dataset_type, 
            dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    if (dataset_id < 0)
    {
        IAS_LOG_ERROR("Creating dataset: %s", dataset_name);
        H5Sclose(dataspace_id);
        return ERROR;
    }

    H5Sclose(dataspace_id);

    data_dims[0] = dims[1];
    /* define the memory dataspace for the buffer to be written */
    memspace_id = H5Screate_simple(1, data_dims, NULL);
    if (memspace_id < 0)
    {
        IAS_LOG_ERROR("Creating memory dataspace");
        return ERROR;
    }

    /* Loop through each SCA */
    for (sca_index = 0; sca_index < grid->scas_per_band[band_index];
         sca_index++)
    {
        /* Define memory hyperslab */
        file_size[0] = 1;
        file_size[1] = dims[1];
        file_offset[0] = sca_index;
        file_offset[1] = 0;

        /* build the file space structure for the correct location */
        filespace_id = H5Dget_space(dataset_id);
        if (filespace_id < 0)
        {
            IAS_LOG_ERROR("Getting dataspace for band %d in dataset %s", 
                          band_index+1, dataset_name);
            H5Sclose(memspace_id);
            H5Dclose(dataset_id);
            return ERROR;
        }

        status = H5Sselect_hyperslab(filespace_id, H5S_SELECT_SET, 
                file_offset, NULL, file_size, NULL);
        if (status < 0)
        {
            IAS_LOG_ERROR("Getting dataspace for band %d in dataset %s", 
                          band_index+1, dataset_name); 
            H5Sclose(memspace_id);
            H5Sclose(filespace_id);
            H5Dclose(dataset_id);
            return ERROR;
        }
        
        /* Calculating the address of the particular array to read in
           the grid band structure by using offset as an indication of
           which member of the structure is being read, member_ptr is
           a void** because the dataset pointer will indicate the actual
           buffer and its type to read into */
        member_ptr = (void *)((unsigned char*)
                     &grid->gridbands[band_index][sca_index]) + offset; 
        dataset = *member_ptr;

        /* Write one SCA data into the dataset. */
        status = H5Dwrite(dataset_id, dataset_type, memspace_id, 
                          filespace_id, H5P_DEFAULT, dataset);
        if (status < 0)
        {
            IAS_LOG_ERROR("Writing the dataset: %s", dataset_name);
            H5Sclose(memspace_id);
            H5Sclose(filespace_id);
            H5Dclose(dataset_id);
            return ERROR;
        }
        H5Sclose(filespace_id);
    }

    H5Sclose(memspace_id);
    /* Close the data space for the second dataset. */
    H5Dclose(dataset_id);
    return SUCCESS;
}
    
/*************************************************************************

NAME: ias_grid_band_pointers_read

PURPOSE: Read out grid band pointer data from HDF5 datasets

RETURNS: SUCCESS -- successfully read grid band pointers
         ERROR -- error in reading grid band pointers

**************************************************************************/
int ias_grid_band_pointers_read
(
    const char *grid_filename,/* I: Grid file name */
    IAS_GRID_TYPE *grid,      /* I/O: Grid structure to read into */
    int band_number           /* I: Band number to read from the grid file */
)
{
    hid_t file_id;                    /* File handle for HDF */
    hid_t group_id;                   /* Group handler */
    int status;                       /* Return status */
    char group_name[25];              /* Group name */
    int offset;                       /* Offset of dataset */
    int band_index;

    /* convert the band number to an index and make sure it is legal */
    band_index = ias_sat_attr_convert_band_number_to_index(band_number);
    if ((band_index == ERROR) || (band_index > grid->nbands))
    {
        IAS_LOG_ERROR("Invalid band index for band number %d", band_number);
        return ERROR;
    }

    /* Open the input file */
    file_id =  H5Fopen(grid_filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file_id < 0)
    {
        IAS_LOG_ERROR("Opening grid file: %s", grid_filename);
        return ERROR;
    }

    /* Allocate memory for the requested band */
    status = ias_grid_malloc_band(grid, band_number);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Allocating Memory for Grid: %s", grid_filename);
        H5Fclose(file_id);
        return ERROR;
    } 

    /* Open "B#" group */
    sprintf(group_name, "/B%d", band_number);
    group_id = H5Gopen(file_id, group_name, H5P_DEFAULT);
    if (group_id < 0) 
    { 
        IAS_LOG_ERROR("Opening %s in %s", group_name, grid_filename);
        ias_grid_free(grid);
        H5Fclose(file_id);
        return ERROR; 
    } 

    /* Read in INPUT_LINES dataset */
    offset = offsetof(struct ias_grid_band_type, in_lines);
    status = read_one_dataset(group_id, band_index, H5T_NATIVE_INT32,
                      INPUT_LINES, offset, 
                      grid->gridbands[band_index][0].ngrid_lines, grid);
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading dataset %s", INPUT_LINES);
        ias_grid_free(grid);
        H5Gclose(group_id);
        H5Fclose(file_id);
        return ERROR;
    } 

    /* Read in INPUT_SAMPLES dataset */
    offset = offsetof(struct ias_grid_band_type, in_samps);
    status = read_one_dataset(group_id, band_index, H5T_NATIVE_INT32,
                      INPUT_SAMPLES, offset, 
                      grid->gridbands[band_index][0].ngrid_samps, grid);
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading dataset %s", INPUT_SAMPLES);
        ias_grid_free(grid);
        H5Gclose(group_id);
        H5Fclose(file_id);
        return ERROR;
    } 

    /* Read in OUTPUT_LINES dataset */
    offset = offsetof(struct ias_grid_band_type, out_lines);
    status = read_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                     OUTPUT_LINES, offset, 
                     grid->gridbands[band_index][0].ngrid_lines
                     *grid->gridbands[band_index][0].ngrid_samps
                     *grid->gridbands[band_index][0].nzplanes, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading dataset %s", OUTPUT_LINES);
        ias_grid_free(grid);
        H5Gclose(group_id);
        H5Fclose(file_id);
        return ERROR;
    } 

    /* Read in OUTPUT_SAMPLES dataset */
    offset = offsetof(struct ias_grid_band_type, out_samps);
    status = read_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                     OUTPUT_SAMPLES, offset, 
                     grid->gridbands[band_index][0].ngrid_lines
                     *grid->gridbands[band_index][0].ngrid_samps
                     *grid->gridbands[band_index][0].nzplanes, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading dataset %s", OUTPUT_SAMPLES);
        ias_grid_free(grid);
        H5Gclose(group_id);
        H5Fclose(file_id);
        return ERROR;
    }

    /* Read in DELTA_LINE_ODD_EVEN dataset */
    offset = offsetof(struct ias_grid_band_type, delta_line_oe);
    status = read_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                     DELTA_LINE_ODD_EVEN, offset, 
                     2*grid->gridbands[band_index][0].ngrid_lines
                     *grid->gridbands[band_index][0].ngrid_samps, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading dataset %s", DELTA_LINE_ODD_EVEN);
        ias_grid_free(grid);
        H5Gclose(group_id);
        H5Fclose(file_id);
        return ERROR;
    } 

    /* Read in DELTA_SAMPLE_ODD_EVEN dataset */
    offset = offsetof(struct ias_grid_band_type, delta_samp_oe);
    status = read_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                     DELTA_SAMPLE_ODD_EVEN, offset, 
                     2*grid->gridbands[band_index][0].ngrid_lines
                     *grid->gridbands[band_index][0].ngrid_samps, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading dataset %s", DELTA_SAMPLE_ODD_EVEN);
        ias_grid_free(grid);
        H5Gclose(group_id);
        H5Fclose(file_id);
        return ERROR;
    } 

    /* Read in SATELLITE_TO_PROJECTION dataset */
    offset = offsetof(struct ias_grid_band_type, sattoproj);
    status = read_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                     SATELLITE_TO_PROJECTION, offset, 2*COEFS_SIZE*
                     (grid->gridbands[band_index][0].ngrid_lines - 1)
                     * (grid->gridbands[band_index][0].ngrid_samps - 1)
                     * grid->gridbands[band_index][0].nzplanes, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading dataset %s", SATELLITE_TO_PROJECTION);
        ias_grid_free(grid);
        H5Gclose(group_id);
        H5Fclose(file_id);
        return ERROR;
    } 

    /* Read in PROJECTION_TO_SATELLITE dataset */
    offset = offsetof(struct ias_grid_band_type, projtosat);
    status = read_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                     PROJECTION_TO_SATELLITE, offset, 2*COEFS_SIZE*
                     (grid->gridbands[band_index][0].ngrid_lines - 1)
                     * (grid->gridbands[band_index][0].ngrid_samps - 1)
                     * grid->gridbands[band_index][0].nzplanes, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading dataset %s", PROJECTION_TO_SATELLITE);
        ias_grid_free(grid);
        H5Gclose(group_id);
        H5Fclose(file_id);
        return ERROR;
    } 

    /* Read in POLYNOMIAL_LINES dataset */
    offset = offsetof(struct ias_grid_band_type, poly_lines);
    status = read_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                     POLYNOMIAL_LINES, offset, 
                     (grid->gridbands[band_index][0].degree+1)
                     *(grid->gridbands[band_index][0].degree+1)
                     *grid->gridbands[band_index][0].nzplanes, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading dataset %s", POLYNOMIAL_LINES);
        ias_grid_free(grid);
        H5Gclose(group_id);
        H5Fclose(file_id);
        return ERROR;
    } 

    /* Read in POLYNOMIAL_SAMPLES dataset */
    offset = offsetof(struct ias_grid_band_type, poly_samps);
    status = read_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                     POLYNOMIAL_SAMPLES, offset,  
                     (grid->gridbands[band_index][0].degree+1)
                     *(grid->gridbands[band_index][0].degree+1)
                     *grid->gridbands[band_index][0].nzplanes, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading dataset %s", POLYNOMIAL_SAMPLES);
        ias_grid_free(grid);
        H5Gclose(group_id);
        H5Fclose(file_id);
        return ERROR;
    } 

    /* Read in LINE_SENSITIVITY dataset */
    offset = offsetof(struct ias_grid_band_type, line_sensitivity);
    status = read_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                 LINE_SENSITIVITY, offset, NUM_SENSITIVITY_DIRECTIONS 
                 * grid->gridbands[band_index][0].ngrid_lines
                 * grid->gridbands[band_index][0].ngrid_samps
                 * grid->gridbands[band_index][0].nzplanes, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading dataset %s", LINE_SENSITIVITY);
        ias_grid_free(grid);
        H5Gclose(group_id);
        H5Fclose(file_id);
        return ERROR;
    } 

    /* Read in SAMPLE_SENSITIVITY dataset */
    offset = offsetof(struct ias_grid_band_type, samp_sensitivity);
    status = read_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                 SAMPLE_SENSITIVITY, offset, NUM_SENSITIVITY_DIRECTIONS
                 * grid->gridbands[band_index][0].ngrid_lines
                 * grid->gridbands[band_index][0].ngrid_samps
                 * grid->gridbands[band_index][0].nzplanes, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Reading dataset %s", SAMPLE_SENSITIVITY);
        ias_grid_free(grid);
        H5Gclose(group_id);
        H5Fclose(file_id);
        return ERROR;
    } 

    /* Close the group */
    H5Gclose(group_id);        

    /* Close the HDF5 File */
    H5Fclose(file_id);

    return SUCCESS;
}

/*************************************************************************

NAME: ias_grid_band_pointers_write

PURPOSE: Write grid data into HDF5 datasets

RETURNS: SUCCESS -- successfully write grid band pointers 
         ERROR -- error in writing grid band pointers

**************************************************************************/
int ias_grid_band_pointers_write
(
    IAS_GRID_FILE *file,        /* I: Grid file pointer */
    const IAS_GRID_TYPE *grid,  /* I: Grid structure to write */
    int band_number             /* I: Band number to write */
)
{
    hid_t group_id;                   /* Dataset group handler */     
    int status;                       /* Return status */
    int band_index;                   /* Current band index */
    char group_name[25];              /* Group name */ 
    int offset;                       /* Group offset */

    /* Convert the band number to an index */
    band_index = ias_sat_attr_convert_band_number_to_index(band_number);
    if (band_index == ERROR)
    {
        IAS_LOG_ERROR("Unable to convert band number %d to an index",
            band_number);
        return ERROR;
    }

    /* Verify the band is present in the grid */
    if (!grid->bands_present[band_index])
    {
        IAS_LOG_ERROR("Attempting to write band number %d to the grid when it"
            " isn't present", band_number);
        return ERROR;
    }

    /* Create group "B#" in the root group */
    sprintf(group_name, "/B%d", band_number);
    group_id = H5Gcreate(file->file_id, group_name, H5P_DEFAULT, 
                   H5P_DEFAULT, H5P_DEFAULT);
    if (group_id < 0) 
    { 
        IAS_LOG_ERROR("Creating %s in %s", group_name, file->filename);
        return ERROR;
    } 

    /* Write out INPUT_LINES dataset */
    offset = offsetof(struct ias_grid_band_type, in_lines);
    status = write_one_dataset(group_id, band_index, H5T_NATIVE_INT32,
                       INPUT_LINES, offset, 
                       grid->gridbands[band_index][0].ngrid_lines, grid);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing dataset %s", INPUT_LINES);
        H5Gclose(group_id);
        return ERROR;
    } 

    /* Write out INPUT_SAMPLES dataset */
    offset = offsetof(struct ias_grid_band_type, in_samps);
    status = write_one_dataset(group_id, band_index, H5T_NATIVE_INT32,
                       INPUT_SAMPLES, offset, 
                       grid->gridbands[band_index][0].ngrid_samps, grid);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing dataset %s", INPUT_SAMPLES);
        H5Gclose(group_id);
        return ERROR;
    } 

    /* Write out OUTPUT_LINES dataset */
    offset = offsetof(struct ias_grid_band_type, out_lines);
    status = write_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                     OUTPUT_LINES, offset, 
                     grid->gridbands[band_index][0].ngrid_lines
                     *grid->gridbands[band_index][0].ngrid_samps     
                     *grid->gridbands[band_index][0].nzplanes, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing dataset %s", OUTPUT_LINES);
        H5Gclose(group_id);
        return ERROR;
    } 

    /* Write out OUTPUT_SAMPLES dataset */
    offset = offsetof(struct ias_grid_band_type, out_samps);
    status = write_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                     OUTPUT_SAMPLES, offset, 
                     grid->gridbands[band_index][0].ngrid_lines
                     *grid->gridbands[band_index][0].ngrid_samps     
                     *grid->gridbands[band_index][0].nzplanes, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing dataset %s", OUTPUT_SAMPLES);
        H5Gclose(group_id);
        return ERROR;
    } 

    /* Write out DELTA_LINE_ODD_EVEN dataset */
    offset = offsetof(struct ias_grid_band_type, delta_line_oe);
    status = write_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                     DELTA_LINE_ODD_EVEN, offset, 
                     2*grid->gridbands[band_index][0].ngrid_lines
                     *grid->gridbands[band_index][0].ngrid_samps, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing dataset %s", DELTA_LINE_ODD_EVEN);
        H5Gclose(group_id);
        return ERROR;
    } 

    /* Write out DELTA_SAMPLE_ODD_EVEN dataset */
    offset = offsetof(struct ias_grid_band_type, delta_samp_oe);
    status = write_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                     DELTA_SAMPLE_ODD_EVEN, offset, 
                     2*grid->gridbands[band_index][0].ngrid_lines
                     *grid->gridbands[band_index][0].ngrid_samps, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing dataset %s", DELTA_SAMPLE_ODD_EVEN);
        H5Gclose(group_id);
        return ERROR;
    }

    /* Write out SATELLITE_TO_PROJECTION dataset */
    offset = offsetof(struct ias_grid_band_type, sattoproj);
    status = write_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                     SATELLITE_TO_PROJECTION, offset, 2*COEFS_SIZE 
                     * (grid->gridbands[band_index][0].ngrid_lines - 1)
                     * (grid->gridbands[band_index][0].ngrid_samps - 1)
                     * grid->gridbands[band_index][0].nzplanes, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing dataset %s", SATELLITE_TO_PROJECTION);
        H5Gclose(group_id);
        return ERROR;
    }

    /* Write out PROJECTION_TO_SATELLITE dataset */
    offset = offsetof(struct ias_grid_band_type, projtosat);
    status = write_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                     PROJECTION_TO_SATELLITE, offset, 2*COEFS_SIZE 
                     * (grid->gridbands[band_index][0].ngrid_lines - 1)
                     * (grid->gridbands[band_index][0].ngrid_samps - 1)
                     * grid->gridbands[band_index][0].nzplanes, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing dataset %s", PROJECTION_TO_SATELLITE);
        H5Gclose(group_id);
        return ERROR;
    } 

    /* Write out POLYNOMIAL_LINES dataset */
    offset = offsetof(struct ias_grid_band_type, poly_lines);
    status = write_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                     POLYNOMIAL_LINES, offset, 
                     (grid->gridbands[band_index][0].degree+1)
                     *(grid->gridbands[band_index][0].degree+1)
                     *grid->gridbands[band_index][0].nzplanes, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing dataset %s", POLYNOMIAL_LINES);
        H5Gclose(group_id);
        return ERROR;
    }

    /* Write out POLYNOMIAL_SAMPLES dataset */
    offset = offsetof(struct ias_grid_band_type, poly_samps);
    status = write_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                     POLYNOMIAL_SAMPLES, offset,  
                     (grid->gridbands[band_index][0].degree+1)
                     *(grid->gridbands[band_index][0].degree+1)
                     *grid->gridbands[band_index][0].nzplanes, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing dataset %s", POLYNOMIAL_SAMPLES);
        H5Gclose(group_id);
        return ERROR;
    } 

    /* Write out LINE_SENSITIVITY dataset */
    offset = offsetof(struct ias_grid_band_type, line_sensitivity);
    status = write_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                     LINE_SENSITIVITY, offset, NUM_SENSITIVITY_DIRECTIONS 
                     * grid->gridbands[band_index][0].ngrid_lines
                     * grid->gridbands[band_index][0].ngrid_samps
                     * grid->gridbands[band_index][0].nzplanes, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing dataset %s", LINE_SENSITIVITY);
        H5Gclose(group_id);
        return ERROR;
    } 

    /* Write out SAMPLE_SENSITIVITY dataset */
    offset = offsetof(struct ias_grid_band_type, samp_sensitivity);
    status = write_one_dataset(group_id, band_index, H5T_NATIVE_DOUBLE,
                     SAMPLE_SENSITIVITY, offset, NUM_SENSITIVITY_DIRECTIONS
                     * grid->gridbands[band_index][0].ngrid_lines
                     * grid->gridbands[band_index][0].ngrid_samps
                     * grid->gridbands[band_index][0].nzplanes, grid); 
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing dataset %s", SAMPLE_SENSITIVITY);
        H5Gclose(group_id);
        return ERROR;
    } 

    /* Close the group. */
    H5Gclose(group_id); 

    return SUCCESS;
}

/*************************************************************************

NAME: ias_grid_read

PURPOSE: Read all the grid data from HDF5 format file

RETURNS: SUCCESS -- successfully read in grid
         ERROR -- error in reading grid

**************************************************************************/
int ias_grid_read
(
    const char *grid_filename,/* I: HDF input file name */
    const int *band_numbers,  /* I: Array of bands to read */
    int nbands,               /* I: number of bands in band_numbers or zero for
                                  all bands (in which case, band_numbers can
                                  be NULL) */
    IAS_GRID_TYPE *grid       /* O: Grid structure to read */
)
{
    hid_t file_id; /* File handle for HDF */
    int grid_format_version = GRID_FORMAT_VERSION;
    int band_index;

    if (ias_grid_initialize(grid, 0, 0) != SUCCESS)
    {
        IAS_LOG_ERROR("Attempting to initialize grid for reading");
        return ERROR;
    }

    /* Open the input file */
    file_id = H5Fopen(grid_filename, H5F_ACC_RDONLY, H5P_DEFAULT);
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

    /* Close the HDF5 File */
    H5Fclose(file_id);

    /* Read grid header */
    if (ias_grid_header_read(grid_filename, grid) != SUCCESS)
    {
        IAS_LOG_ERROR("Reading grid header: %s", grid_filename);
        return ERROR;
    }

    /* Read grid band header */
    if (ias_grid_band_header_read(grid_filename, grid) != SUCCESS)
    {
        IAS_LOG_ERROR("Reading grid band header: %s", grid_filename);
        return ERROR;
    }

    /* Read the grid band pointers for all the bands */
    for (band_index = 0; band_index < grid->nbands; band_index++)
    {
        int band_number = ias_sat_attr_convert_band_index_to_number(band_index);
        int band_wanted = 0;
        int index;

        if (band_number == ERROR)
        {
            IAS_LOG_ERROR("Invalid band number for band index: %d", band_index);
            return ERROR;
        }

        /* check whether the band was requested */
        for (index = 0; index < nbands; index++)
        {
            if (band_number == band_numbers[index])
            {
                band_wanted = 1;
                break;
            }
        }

        /* if the band is wanted and available, read it */
        if (grid->bands_available[band_index] && (band_wanted || (nbands == 0)))
        {
            if (ias_grid_band_pointers_read(grid_filename, grid, band_number)
                != SUCCESS)
            {
                IAS_LOG_ERROR("Reading grid band #%d pointers: %s", band_number,
                              grid_filename);
                return ERROR;
            }
        }
        else if (band_wanted)
        {
            /* a requested band is not available, so that is an error */
            IAS_LOG_ERROR("Requested band number %d is not in %s", 
                          band_number, grid_filename);
            return ERROR;
        }
    }

    return SUCCESS;
}

/*************************************************************************
NAME: ias_grid_create

PURPOSE: Creates a new grid file for writing and returns a pointer to it

RETURNS:
    NULL if an ERROR happens; Pointer to file structure if it succeeds

**************************************************************************/
IAS_GRID_FILE *ias_grid_create
(
    const char *grid_filename   /* I: grid file name to create */
)
{
    int grid_format_version = GRID_FORMAT_VERSION;
    IAS_GRID_FILE *file;

    /* Allocate space for the grid file structure */
    file = malloc(sizeof(*file));
    if (!file)
    {
        IAS_LOG_ERROR("Could not allocate memory for grid file structure");
        return NULL;
    }

    /* Copy the filename for later use in messages */
    file->filename = strdup(grid_filename);
    if (!file->filename)
    {
        IAS_LOG_ERROR("Could not allocate memory for file name");
        free(file);
        return NULL;
    }

    /* Create the output file */
    file->file_id = H5Fcreate(grid_filename, H5F_ACC_TRUNC, H5P_DEFAULT,
        H5P_DEFAULT);
    if (file->file_id < 0)
    {
        IAS_LOG_ERROR("Could not create grid file: %s", grid_filename);
        free(file->filename);
        free(file);
        return NULL;
    }

    /* Create a grid format version */
    if (H5LTset_attribute_int(file->file_id, "/", "Grid Format Version",
            &grid_format_version, 1) < 0)
    {
        IAS_LOG_ERROR("Could not create grid format version: %s",grid_filename);
        H5Fclose(file->file_id);
        free(file->filename);
        free(file);
        return NULL;
    }

    /* Create a file grid type */
    if (H5LTset_attribute_string(file->file_id, "/", "File Type", FILE_TYPE)
        < 0)
    {
        IAS_LOG_ERROR("Could not create file type: %s",grid_filename);
        H5Fclose(file->file_id);
        free(file->filename);
        free(file);
        return NULL;
    }

    return file;
}
/*************************************************************************
NAME: ias_grid_close

PURPOSE: Closes a grid created with the ias_grid_create routine.

RETURNS:
    SUCCESS or ERROR
**************************************************************************/
int ias_grid_close
(
    IAS_GRID_FILE *file     /* I: pointer to the grid to close */
)
{
    int status = SUCCESS;

    /* Make sure the pointer is valid */
    if (file)
    {
        /* Close the file and free the memory */
        if (H5Fclose(file->file_id) < 0)
        {
            IAS_LOG_ERROR("Error closing grid file %s", file->filename);
            status = ERROR;
        }
        free(file->filename);
        free(file);
    }
    return status;
}

/*************************************************************************

NAME: ias_grid_write

PURPOSE: Write the entire grid to the grid file

RETURNS: SUCCESS -- successfully write grid
         ERROR -- error in writing grid

**************************************************************************/
int ias_grid_write
(
    const char *grid_filename,  /* I: HDF input file name */
    const IAS_GRID_TYPE *grid   /* I: Grid structure to write */
)
{
    IAS_GRID_FILE *file;
    int band_index;

    /* Create the output file */
    file = ias_grid_create(grid_filename);
    if (!file)
    {
        IAS_LOG_ERROR("Could not create grid file: %s", grid_filename);
        return ERROR;
    }

    /* Write the grid band pointer info to the file */
    for (band_index = 0; band_index < grid->nbands; band_index++)
    {
        int band_number;

        if (!grid->bands_present[band_index])
        {
            /* band not present, so just skip it */
            continue;
        }

        /* Convert the band index to a number */
        band_number = ias_sat_attr_convert_band_index_to_number(band_index);
        if (band_number == ERROR)
        {
            IAS_LOG_ERROR("Unabled to convert band index %d to a band number",
                band_index);
            ias_grid_close(file);
            return ERROR;
        }

        /* Write grid band pointers */
        if (ias_grid_band_pointers_write(file, grid, band_number) != SUCCESS)
        {
            IAS_LOG_ERROR("Writing grid band pointers: %s", grid_filename);
            ias_grid_close(file);
            return ERROR;
        }
    }

    /* Write grid header */
    if (ias_grid_header_write(file, grid) != SUCCESS)
    {
        IAS_LOG_ERROR("Writing grid header: %s", grid_filename);
        ias_grid_close(file);
        return ERROR;
    }

    /* Write grid band header */
    if (ias_grid_band_header_write(file, grid) != SUCCESS)
    {
        IAS_LOG_ERROR("Writing grid band header: %s", grid_filename);
        ias_grid_close(file);
        return ERROR;
    }

    /* Close the HDF5 File */
    if (ias_grid_close(file) != SUCCESS)
    {
        IAS_LOG_ERROR("Error closing grid file: %s", grid_filename);
        return ERROR;
    }

    return SUCCESS;
}  /* END ias_grid_write */

/*************************************************************************

NAME: free_partial_grid

PURPOSE:free memory for partial grid band pointer data, the routine is
        called when allocating memory failed to free the already partial
        allocated memory space
        
RETURNS: None

**************************************************************************/
static void free_partial_grid 
(
    int band_index,           /* I: Number of band of data to free */
    int sca_number,           /* I: Number of SCAs of data to free in 
                                    the partially allocated band  */
    IAS_GRID_TYPE *grid   /* I: Grid structure to free   */
)
{
    int sca_index;
    int temp_band_index;

    /* Loop through each SCA, freeing memory for the current band up to the
       current SCA */
    for (sca_index = 0; sca_index < sca_number; sca_index++)
    {
        free(grid->gridbands[band_index][sca_index].in_lines);
        grid->gridbands[band_index][sca_index].in_lines = NULL;
        free(grid->gridbands[band_index][sca_index].in_samps);
        grid->gridbands[band_index][sca_index].in_samps = NULL;
        free(grid->gridbands[band_index][sca_index].out_lines);
        grid->gridbands[band_index][sca_index].out_lines = NULL;
        free(grid->gridbands[band_index][sca_index].out_samps);
        grid->gridbands[band_index][sca_index].out_samps = NULL;
        free(grid->gridbands[band_index][sca_index].delta_line_oe);
        grid->gridbands[band_index][sca_index].delta_line_oe = NULL;
        free(grid->gridbands[band_index][sca_index].delta_samp_oe);
        grid->gridbands[band_index][sca_index].delta_samp_oe = NULL;
        free(grid->gridbands[band_index][sca_index].sattoproj);
        grid->gridbands[band_index][sca_index].sattoproj = NULL;
        free(grid->gridbands[band_index][sca_index].projtosat);
        grid->gridbands[band_index][sca_index].projtosat = NULL;
        free(grid->gridbands[band_index][sca_index].poly_lines);
        grid->gridbands[band_index][sca_index].poly_lines = NULL;
        free(grid->gridbands[band_index][sca_index].poly_samps);
        grid->gridbands[band_index][sca_index].poly_samps = NULL;
        free(grid->gridbands[band_index][sca_index].line_sensitivity);
        grid->gridbands[band_index][sca_index].line_sensitivity = NULL;
        free(grid->gridbands[band_index][sca_index].samp_sensitivity);
        grid->gridbands[band_index][sca_index].samp_sensitivity = NULL;
    }

    /* Free the memory for all SCAs for the previous bands */
    for (temp_band_index = 0; temp_band_index < band_index; temp_band_index++)
    {   
        for (sca_index = 0; sca_index < grid->scas_per_band[temp_band_index];
             sca_index++)
        {
            free(grid->gridbands[temp_band_index][sca_index].in_lines);
            grid->gridbands[temp_band_index][sca_index].in_lines = NULL;
            free(grid->gridbands[temp_band_index][sca_index].in_samps);
            grid->gridbands[temp_band_index][sca_index].in_samps = NULL;
            free(grid->gridbands[temp_band_index][sca_index].out_lines);
            grid->gridbands[temp_band_index][sca_index].out_lines = NULL;
            free(grid->gridbands[temp_band_index][sca_index].out_samps);
            grid->gridbands[temp_band_index][sca_index].out_samps = NULL;
            free(grid->gridbands[temp_band_index][sca_index].delta_line_oe);
            grid->gridbands[temp_band_index][sca_index].delta_line_oe = NULL;
            free(grid->gridbands[temp_band_index][sca_index].delta_samp_oe);
            grid->gridbands[temp_band_index][sca_index].delta_samp_oe = NULL;
            free(grid->gridbands[temp_band_index][sca_index].sattoproj);
            grid->gridbands[temp_band_index][sca_index].sattoproj = NULL;
            free(grid->gridbands[temp_band_index][sca_index].projtosat);
            grid->gridbands[temp_band_index][sca_index].projtosat = NULL;
            free(grid->gridbands[temp_band_index][sca_index].poly_lines);
            grid->gridbands[temp_band_index][sca_index].poly_lines = NULL;
            free(grid->gridbands[temp_band_index][sca_index].poly_samps);
            grid->gridbands[temp_band_index][sca_index].poly_samps = NULL;
            free(grid->gridbands[temp_band_index][sca_index].line_sensitivity);
            grid->gridbands[temp_band_index][sca_index].line_sensitivity = NULL;
            free(grid->gridbands[temp_band_index][sca_index].samp_sensitivity);
            grid->gridbands[temp_band_index][sca_index].samp_sensitivity = NULL;
        }
    }  
}

/*************************************************************************

NAME: ias_grid_malloc_band

PURPOSE: Allocate memory for a gridband structure

RETURNS: SUCCESS -- successfully allocated memory
         ERROR -- error in allocating memory

**************************************************************************/
int ias_grid_malloc_band
(
    IAS_GRID_TYPE *grid,        /* I/O: grid to allocate band in */
    int band_number             /* I: band number to allocate memory for */
)
{
    int band_index;
    int sca_index;

    band_index = ias_sat_attr_convert_band_number_to_index(band_number);
    if (band_index == ERROR)
    {
        IAS_LOG_ERROR("Invalid band index for band number: %d", band_number);
        return ERROR;
    }

    for (sca_index = 0; sca_index < grid->scas_per_band[band_index];
         sca_index++)
    {
        grid->gridbands[band_index][sca_index].in_lines = (int *)malloc(
           grid->gridbands[band_index][sca_index].ngrid_lines *sizeof(int)); 
        if (grid->gridbands[band_index][sca_index].in_lines == NULL)
        {
            IAS_LOG_ERROR("Allocating Memory");
            /* Free previously malloced pointers. */
            free_partial_grid(band_index, sca_index, grid);
            return ERROR;
        }
        grid->gridbands[band_index][sca_index].in_samps = (int *)malloc(
            grid->gridbands[band_index][sca_index].ngrid_samps *sizeof(int)); 
        if (grid->gridbands[band_index][sca_index].in_samps == NULL)
        {
            IAS_LOG_ERROR("Allocating Memory");
            /* Free previously malloced pointers. */
            free_partial_grid(band_index, sca_index, grid);
            return ERROR;
        } 
        grid->gridbands[band_index][sca_index].out_lines = (double *)malloc(
            grid->gridbands[band_index][sca_index].ngrid_lines
           *grid->gridbands[band_index][sca_index].ngrid_samps
           *grid->gridbands[band_index][sca_index].nzplanes *sizeof(double)); 
        if (grid->gridbands[band_index][sca_index].out_lines == NULL)
        {
            IAS_LOG_ERROR("Allocating Memory");
            /* Free previously malloced pointers. */
            free_partial_grid(band_index, sca_index, grid);
            return ERROR;
        }         
        grid->gridbands[band_index][sca_index].out_samps = (double *)malloc(
            grid->gridbands[band_index][sca_index].ngrid_lines
           *grid->gridbands[band_index][sca_index].ngrid_samps
           *grid->gridbands[band_index][sca_index].nzplanes *sizeof(double)); 
        if (grid->gridbands[band_index][sca_index].out_samps == NULL)
        {
            IAS_LOG_ERROR("Allocating Memory");
            /* Free previously malloced pointers. */
            free_partial_grid(band_index, sca_index, grid);
            return ERROR;
        }         
        grid->gridbands[band_index][sca_index].delta_line_oe 
            = (double *)malloc(
           2*grid->gridbands[band_index][sca_index].ngrid_lines
           *grid->gridbands[band_index][sca_index].ngrid_samps *sizeof(double));
        if (grid->gridbands[band_index][sca_index].delta_line_oe 
                               == NULL)
        {
            IAS_LOG_ERROR("Allocating Memory");
            /* Free previously malloced pointers. */
            free_partial_grid(band_index, sca_index, grid);
            return ERROR;
        }         
        grid->gridbands[band_index][sca_index].delta_samp_oe 
            = (double *)malloc(
           2*grid->gridbands[band_index][sca_index].ngrid_lines
           *grid->gridbands[band_index][sca_index].ngrid_samps
           *sizeof(double)); 
        if (grid->gridbands[band_index][sca_index].delta_samp_oe == NULL)
        {
            IAS_LOG_ERROR("Allocating Memory");
            /* Free previously malloced pointers. */
            free_partial_grid(band_index, sca_index, grid);
            return ERROR;
        }         
        grid->gridbands[band_index][sca_index].sattoproj 
            = (IAS_COEFFICIENTS *)malloc(
          (grid->gridbands[band_index][sca_index].ngrid_lines - 1)
          * (grid->gridbands[band_index][sca_index].ngrid_samps - 1)
          * grid->gridbands[band_index][sca_index].nzplanes
          * sizeof(IAS_COEFFICIENTS)); 
        if (grid->gridbands[band_index][sca_index].sattoproj == NULL)
        {
            IAS_LOG_ERROR("Allocating Memory");
            /* Free previously malloced pointers. */
            free_partial_grid(band_index, sca_index, grid);
            return ERROR;
        }         
        grid->gridbands[band_index][sca_index].projtosat
            = (IAS_COEFFICIENTS *)malloc(
              (grid->gridbands[band_index][sca_index].ngrid_lines - 1)
              * (grid->gridbands[band_index][sca_index].ngrid_samps - 1)
              * grid->gridbands[band_index][sca_index].nzplanes
              * sizeof(IAS_COEFFICIENTS)); 
        if (grid->gridbands[band_index][sca_index].projtosat == NULL)
        {
            IAS_LOG_ERROR("Allocating Memory");
            /* Free previously malloced pointers. */
            free_partial_grid(band_index, sca_index, grid);
            return ERROR;
        }         
        grid->gridbands[band_index][sca_index].poly_lines = (double *)malloc(
          (grid->gridbands[band_index][sca_index].degree+1)
          *(grid->gridbands[band_index][sca_index].degree+1)
          *grid->gridbands[band_index][sca_index].nzplanes *sizeof(double)); 
        if (grid->gridbands[band_index][sca_index].poly_lines == NULL)
        {
            IAS_LOG_ERROR("Allocating Memory");
            /* Free previously malloced pointers. */
            free_partial_grid(band_index, sca_index, grid);
            return ERROR;
        }         
        grid->gridbands[band_index][sca_index].poly_samps = (double *)malloc(
          (grid->gridbands[band_index][sca_index].degree+1)
          *(grid->gridbands[band_index][sca_index].degree+1)
          *grid->gridbands[band_index][sca_index].nzplanes *sizeof(double)); 
        if (grid->gridbands[band_index][sca_index].poly_samps == NULL)
        {
            IAS_LOG_ERROR("Allocating Memory");
            /* Free previously malloced pointers. */
            free_partial_grid(band_index, sca_index, grid);
            return ERROR;
        }         
        grid->gridbands[band_index][sca_index].line_sensitivity 
            = (IAS_VECTOR *)malloc(
          grid->gridbands[band_index][sca_index].ngrid_lines
          * grid->gridbands[band_index][sca_index].ngrid_samps
          * grid->gridbands[band_index][sca_index].nzplanes
          * sizeof(IAS_VECTOR)); 
        if (grid->gridbands[band_index][sca_index].line_sensitivity == NULL)
        {
            IAS_LOG_ERROR("Allocating Memory");
            /* Free previously malloced pointers. */
            free_partial_grid(band_index, sca_index, grid);
            return ERROR;
        }         
        grid->gridbands[band_index][sca_index].samp_sensitivity 
            = (IAS_VECTOR *)malloc(
          grid->gridbands[band_index][sca_index].ngrid_lines
          * grid->gridbands[band_index][sca_index].ngrid_samps
          * grid->gridbands[band_index][sca_index].nzplanes
          * sizeof(IAS_VECTOR)); 
        if (grid->gridbands[band_index][sca_index].samp_sensitivity == NULL)
        {
            IAS_LOG_ERROR("Allocating Memory");
            /* Free previously malloced pointers. */
            free_partial_grid(band_index, sca_index, grid);
            return ERROR;
        }         
    }

    grid->bands_present[band_index] = 1;
    grid->bands_available[band_index] = 1;

    return SUCCESS;
}

/*************************************************************************

NAME: ias_grid_malloc

PURPOSE: Allocate memory grid band memory for a list of bands

RETURNS: SUCCESS -- successfully malloc memory
         ERROR -- error in mallocing memory

**************************************************************************/
int ias_grid_malloc 
(
    IAS_GRID_TYPE *grid,    /* I/O: Grid structure to be malloced */
    const int *band_numbers,/* I: Array of bands to allocate memory for */
    int nbands              /* I: number of bands in band_numbers or zero for
                                  all bands (in which case, band_numbers can
                                  be NULL) */
)
{
    int index;
    const int *band_ptr;
    int local_bands[IAS_MAX_NBANDS];

    if (nbands == 0)
    {
        nbands = grid->nbands;
        for (index = 0; index < nbands; index++)
        {
            local_bands[index]
                = ias_sat_attr_convert_band_index_to_number(index);
            if (local_bands[index] == ERROR)
            {
                IAS_LOG_ERROR("Invalid band number for band index: %d", index);
                return ERROR;
            }
        }
        band_ptr = local_bands;
    }
    else
        band_ptr = band_numbers;
        
    if ((nbands < 1) || (nbands > grid->nbands))
    {
        IAS_LOG_ERROR("Invalid number of bands requested");
        return ERROR;
    }

    /* Loop through the bands mallocing the correct reference bands. */
    for (index = 0; index < nbands; index++)
    {
        int band_number = band_ptr[index];

        if (ias_grid_malloc_band(grid, band_number) != SUCCESS)
        {
            IAS_LOG_ERROR("Allocating memory for grid");
            return ERROR;
        }
    }

    return SUCCESS;
}

/*************************************************************************

NAME: ias_grid_free_band

PURPOSE: Free memory for single band in the grid

RETURNS: SUCCESS -- successfully freed memory
         ERROR -- error in freeing memory

**************************************************************************/
int ias_grid_free_band
(
    IAS_GRID_TYPE *grid,        /* I/O: grid to free band in */
    int band_number             /* I: band number to free */
)
{
    int sca_index;
    int band_index;
    IAS_GRID_BAND_TYPE *gridband;

    band_index = ias_sat_attr_convert_band_number_to_index(band_number);
    if (band_index == ERROR)
    {
        IAS_LOG_ERROR("Invalid band index for band number: %d", band_number);
        return ERROR;
    }
    gridband = grid->gridbands[band_index];

    for (sca_index = 0; sca_index < grid->scas_per_band[band_index];
         sca_index++)
    {
        free(gridband[sca_index].in_lines);
        gridband[sca_index].in_lines = NULL;
        free(gridband[sca_index].in_samps);
        gridband[sca_index].in_samps = NULL;
        free(gridband[sca_index].out_lines);
        gridband[sca_index].out_lines = NULL;
        free(gridband[sca_index].out_samps);
        gridband[sca_index].out_samps = NULL;
        free(gridband[sca_index].delta_line_oe);
        gridband[sca_index].delta_line_oe = NULL;
        free(gridband[sca_index].delta_samp_oe);
        gridband[sca_index].delta_samp_oe = NULL;
        free(gridband[sca_index].sattoproj);
        gridband[sca_index].sattoproj = NULL;
        free(gridband[sca_index].projtosat);
        gridband[sca_index].projtosat = NULL;
        free(gridband[sca_index].poly_lines);
        gridband[sca_index].poly_lines = NULL;
        free(gridband[sca_index].poly_samps);
        gridband[sca_index].poly_samps = NULL;
        free(gridband[sca_index].line_sensitivity);
        gridband[sca_index].line_sensitivity = NULL;
        free(gridband[sca_index].samp_sensitivity);
        gridband[sca_index].samp_sensitivity = NULL;
    }

    grid->bands_present[band_index] = 0;

    return SUCCESS;
}

/*************************************************************************

NAME: ias_grid_free

PURPOSE: Free memory for grid band pointer data

RETURNS: SUCCESS -- successfully freed memory
         ERROR -- error in freeing memory

**************************************************************************/
int ias_grid_free 
(
    IAS_GRID_TYPE *grid     /* I/O: Grid structure to be freed */
)
{
    int band_index;
  
    /* Loop through the bands freeing the correct reference bands. */
    for (band_index = 0; band_index < grid->nbands; band_index++)
    {
        if (grid->bands_present[band_index])
        {
            int band_number 
                = ias_sat_attr_convert_band_index_to_number(band_index);
            if (band_number == ERROR)
            {
                IAS_LOG_ERROR("Invalid band number for band index: %d", 
                    band_index);
                continue;
            }

            ias_grid_free_band(grid, band_number);
        }
    }

    /* free the 2d gridbands array */
    if (ias_misc_free_2d_array((void **)grid->gridbands) != SUCCESS)
    {
        IAS_LOG_ERROR("Freeing the gridbands array");
        return ERROR;
    }
    grid->gridbands = NULL;

    return SUCCESS;
}

/*************************************************************************

NAME: ias_grid_is_grid_file

PURPOSE: Judge whether the file a grid file 

RETURNS: TRUE -- is a grid file
         FALSE -- is not a grid file

**************************************************************************/
int ias_grid_is_grid_file
(
    const char *grid_filename /* I: HDF input file name */
)
{
    return ias_misc_check_file_type(grid_filename, FILE_TYPE);
}
