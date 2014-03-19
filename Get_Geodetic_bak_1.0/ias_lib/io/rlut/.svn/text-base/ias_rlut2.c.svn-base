/*==============================================================================
NAME:
    ias_rlut2

PURPOSE:
    Provide the RLUT IO support for the alternate Response linearization
    LookUp Table (RLUT2).

ALGORITHM REFERENCES:
    Alternate Response Linearization ADD
==============================================================================*/

/* System Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* COTS Includes */
#include "hdf5.h"
#include "hdf5_hl.h"

/* IAS Includes */
#include "ias_const.h"
#include "ias_types.h"
#include "ias_logging.h"
#include "ias_miscellaneous.h"
#include "ias_rlut.h"
#include "ias_rlut2.h"
#include "ias_satellite_attributes.h"

/* Local Includes */
#include "ias_rlut_private.h"

/* Define that identifies an RLUT2 file */
#define IAS_RLUT2_FILE_TYPE      "L%d RLUT2 Lookup File"
#define IAS_RLUT2_FORMAT_VERSION 1

/*------------------------------------------------------------------------------
 * Defines that make up the group and data set names within the HDF5 RLUT2 file.
 * The are two different types of corrections that are applied in the
 * alternative RLUT algorthim; the linearity (LIN) and Non-uniformity (NUC).
 * 
 * Both of these corrections use a lookup table to locate a DN value from the
 * satellite and then use a parallel array to find the corresponding correction
 * value that matches that input value.
 * 
 * Each band and SCA has a different set of lookup and correction values for
 * both the LIN and NUC corrections.  These values are organized within the HDF5
 * file as follows:
 *
 *     /
 *     |-- Linearity
 *     |   |-- BAND01
 *     |   |   |-- SCA01
 *     |   |   |   |-- DN   (Data set of DN lookup values)
 *     |   |   |   `-- LIN  (Data set of Linearity correction values)
 *     |   |   |-- SCA02
 *     |   |   |   |-- DN
 *     |   |   |   `-- LIN
 *     |   |   |
 *     |   |   |   ... all SCA's ...
 *     |   |   |
 *     |   |   `-- SCA14
 *     |   |       |-- DN
 *     |   |       `-- LIN
 *     |   |
 *     |   |   ... all normal bands ...
 *     |   |
 *     |   `-- Band11
 *     |       |-- SCA01
 *     |       |   |-- DN
 *     |       |   `-- LIN
 *     |       |-- SCA02
 *     |       |   |-- DN
 *     |       |   `-- LIN
 *     |       `-- SCA03
 *     |           |-- DN
 *     |           `-- LIN
 *     |
 *     `-- Nonuniformity
 *         |-- BAND01
 *         |   |-- SCA01
 *         |   |   |-- DN   (Data set of DN lookup values)
 *         |   |   `-- NUC  (Data set of nonuniformity correction values)
 *         |   |-- SCA02
 *         |   |   |-- DN
 *         |   |   `-- NUC
 *         |   |
 *         |   |   ... all SCA's ...
 *         |   |
 *         |   `-- SCA14
 *         |       |-- DN
 *         |       `-- NUC
 *         |
 *         |   ... all normal bands ...
 *         |
 *         `-- Band11
 *             |-- SCA01
 *             |   |-- DN
 *             |   `-- NUC
 *             |-- SCA02
 *             |   |-- DN
 *             |   `-- NUC
 *             `-- SCA03
 *                 |-- DN
 *                 `-- NUC
 *
 *----------------------------------------------------------------------------*/
#define RLUT2_BAND          "BAND"
#define RLUT2_DN            "DN"
#define RLUT2_LIN           "LIN"
#define RLUT2_LINEARITY     "/Linearity"
#define RLUT2_NONUNIFORMITY "/Nonuniformity"
#define RLUT2_NUC           "NUC"
#define RLUT2_SCA           "SCA"


/*------------------------------------------------------------------------------
NAME: util_get_names

PURPOSE:
    Create the group name and data set names for an RLUT2 HDF5 file.  The data
    set names used in the RLUT2 HDF5 file contain three groups like
    "/Linearity/BANDxx/SCAxx/DN" or "/Nonuniformity/BANDxx/SCAxx/DN".

RETURNS:
    This function returns a status of SUCCESS if the names are successfully
    created, ERROR if an error is countered.
------------------------------------------------------------------------------*/
static int util_get_names
(
    int is_linearity,             /* I: Values to read, TRUE=LIN, FALSE=NUC */
    int band_num,                 /* I: Band number of interest */
    int sca_num,                  /* I: SCA number of interest */
    char groupName[PATH_MAX],     /* O: Group name for the band and SCA */
    char dataSetName[2][PATH_MAX] /* O: Data set names */
)
{
    int bytes;

    bytes = snprintf(groupName, PATH_MAX, "%s/%s%02d/%s%02d",
        (is_linearity ? RLUT2_LINEARITY : RLUT2_NONUNIFORMITY),
        RLUT2_BAND, band_num, RLUT2_SCA, sca_num);
    if ((bytes < 0) || (bytes >= PATH_MAX))
    {
        IAS_LOG_ERROR("Buffer for group name too small, requires %d characters",
            bytes);
        return ERROR;
    }

    bytes = snprintf(dataSetName[0], PATH_MAX, "%s/%s", groupName, RLUT2_DN);
    if ((bytes < 0) || (bytes >= PATH_MAX))
    {
        IAS_LOG_ERROR(
            "Buffer for data set name too small, requires %d characters",
            bytes);
        return ERROR;
    }

    bytes = snprintf(dataSetName[1], PATH_MAX, "%s/%s", groupName,
        (is_linearity ? RLUT2_LIN : RLUT2_NUC));
    if ((bytes < 0) || (bytes >= PATH_MAX))
    {
        IAS_LOG_ERROR(
            "Buffer for data set name too small, requires %d characters",
            bytes);
        return ERROR;
    }
    return SUCCESS;
}


/*------------------------------------------------------------------------------
NAME: util_write_array

PURPOSE:
    This utility function will write a 2D array of values to a data set in an
    HDF5 file for the RLUT2 functions.  This function can be used to write the
    lookup DN values as well as the LIN and NUC correction values.

RETURNS:
    This function will return SUCCESS if the values are successfully written to
    the RLUT2 file, ERROR if an error is encountered.
------------------------------------------------------------------------------*/
static int util_write_array
(
    hid_t hdfIdGroup,     /* I: HDF5 group ID being written to */
    hsize_t dims[2],      /* I: Dimensions of the 'values' parameter */
    const char *hdfPath,  /* I: Full path of the data set */
    float *values         /* I: 2D array to be written to the file */
)
{
    herr_t hdfStatus;      /* HDF5 status */
    hid_t  hdfIdDataSet;
    hid_t  hdfIdDataSpace;

    /* Create a new HDF5 dataspace for the table of values.  The first argument
       indicates the number of dimensions in the data space.  The last argument
       is the maximum dimensions of the data space, but by passing in a NULL
       pointer, the maximum dimensions will be set to the same as the initial
       dimensions. */
    hdfIdDataSpace = H5Screate_simple(2, dims, NULL);
    if (hdfIdDataSpace < 0)
    {
        IAS_LOG_ERROR("Creating an HDF5 data space for %s", hdfPath);
        return ERROR;
    }

    /* Create the data set for the table of values within the HDF5 file */
    hdfIdDataSet = H5Dcreate(hdfIdGroup, hdfPath, H5T_IEEE_F32LE,
        hdfIdDataSpace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (hdfIdDataSet < 0)
    {
        IAS_LOG_ERROR("Creating an HDF5 data space for %s", hdfPath);
        H5Sclose(hdfIdDataSpace);
        return ERROR;
    }

    /* Write the 2D array of values to the HDF5 file */
    hdfStatus = H5Dwrite(hdfIdDataSet, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
        H5P_DEFAULT, values);
    if (hdfStatus < 0)
    {
        IAS_LOG_ERROR("Writing to HDF5 data space for %s", hdfPath);
        H5Dclose(hdfIdDataSet);
        H5Sclose(hdfIdDataSpace);
        return ERROR;
    }

    /* Close the data set we just wrote */
    hdfStatus = H5Dclose(hdfIdDataSet);
    if (hdfStatus < 0)
    {
        IAS_LOG_ERROR("Closing data set for %s", hdfPath);
        H5Sclose(hdfIdDataSpace);
        return ERROR;
    }

    /* Close the data space we created for the table */
    hdfStatus = H5Sclose(hdfIdDataSpace);
    if (hdfStatus < 0)
    {
        IAS_LOG_ERROR("Closing data space for %s", hdfPath);
        return ERROR;
    }

    return SUCCESS;
}


/*------------------------------------------------------------------------------
NAME: ias_rlut2_close_file

PURPOSE:
    Close an open RLUT2 file.

RETURNS:
    This function returns a status of SUCCESS if the file is successfully
    closed, ERROR if an error is encountered.
------------------------------------------------------------------------------*/
int ias_rlut2_close_file
(
    IAS_RLUT2_IO *rlut /* I: An open alternate RLUT2 file */
)
{
    int status = SUCCESS;

    if (rlut == NULL)
        return status;

    if ((rlut->file_id >= 0) && (H5Fclose(rlut->file_id) < 0))
    {
        IAS_LOG_ERROR("Closing RLUT2 file: %s", rlut->filename);
        status = ERROR;
    }

    free(rlut->filename);
    free(rlut);
    return status;
}


/*------------------------------------------------------------------------------
NAME: ias_rlut2_free_table

PURPOSE:
    This function will free up all of the memory associated with an RLUT2 table.
    The pointer passed in will be set to NULL if the function completes
    successfully.

    The specified table should have been allocated with a call to
    ias_rlut2_realloc_table().

RETURNS:
    N/A
------------------------------------------------------------------------------*/
void ias_rlut2_free_table
(
    IAS_RLUT2_TABLE *table /* I/O: An allocated RLUT2 table */
)
{
    if (table == NULL)
        return;
    free(table->input_dn);
    free(table->output_correction);
    free(table);
}


/*------------------------------------------------------------------------------
NAME: ias_rlut2_is_rlut2_file

PURPOSE:
    Examine the file type attribute in an HDF5 file and ensure it is an RLUT2
    file.  This function requires that the satellite attributes library be
    initialized.

RETURNS:
    This function returns a status of TRUE if the file is an RLUT2 file, FALSE
    otherwise.
------------------------------------------------------------------------------*/
int ias_rlut2_is_rlut2_file
(
    const char *path /* I: Path to an RLUT2 file */
)
{
    char fileType[256];
    int  bytes;
    int  satNum;

    satNum = ias_sat_attr_get_satellite_number();
    if (satNum == ERROR)
    {
        IAS_LOG_ERROR("Satellite attributes library not initialized");
        return FALSE;
    }

    bytes = snprintf(fileType, sizeof(fileType), IAS_RLUT2_FILE_TYPE, satNum);
    if ((bytes < 0) || (bytes >= sizeof(fileType)))
    {
        IAS_LOG_ERROR("Buffer size exceeded, requires %d characters", bytes);
        return FALSE;
    }
    return ias_misc_check_file_type(path, fileType);
}


/*------------------------------------------------------------------------------
NAME: ias_rlut2_open_read

PURPOSE:
    Open an alternate response linearization lookup table for reading.

    The calling function is responsible for calling ias_rlut2_close_file() so
    that the file is properly closed and all dynamically allocate memory is
    freed properly.

RETURNS:
    Upon successfully opening the file, this function will return a pointer to a
    dynamically allocated IAS_RLUT2_IO structure.  If an error is encountered,
    this function will return a NULL pointer.
------------------------------------------------------------------------------*/
IAS_RLUT2_IO *ias_rlut2_open_read
(
    const char *path /* I: Path to the RLUT2 file */
)
{
    IAS_RLUT2_IO *fd = NULL;
    int status;

    /* Ensure it is a valid RLUT2 file */
    if (!ias_rlut2_is_rlut2_file(path))
    {
        IAS_LOG_ERROR("File is not a valid RLUT2: %s", path);
        return NULL;
    }

    /* Allocate a structure to track the file */
    fd = malloc(sizeof(*fd));
    if (fd == NULL)
    {
        IAS_LOG_ERROR("Allocating memory for IAS_RLUT2_IO object");
        return NULL;
    }

    /* Copy the specified path into our structure */
    fd->filename = strdup(path);
    if (fd->filename == NULL)
    {
        IAS_LOG_ERROR("Allocating memory for IAS_RLUT2_IO object");
        free(fd);
        return NULL;
    }

    /* Open the file for reading */
    fd->file_id = H5Fopen(path, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (fd->file_id < 0)
    {
        IAS_LOG_ERROR("Opening RLUT2 for reading: %s", path);
        free(fd->filename);
        free(fd);
        return NULL;
    }

    /* Read and check the file version */
    status = H5LTget_attribute_int(fd->file_id, "/", "FILE_FORMAT_VERSION",
        &fd->file_format_version);
    if (status < 0)
    {
        IAS_LOG_ERROR("Retrieving RLUT2 file format version");
        ias_rlut2_close_file(fd);
        return NULL;
    }

    if (fd->file_format_version != IAS_RLUT2_FORMAT_VERSION)
    {
        IAS_LOG_ERROR("Invalid format file version: %d, expecting %d",
            fd->file_format_version, IAS_RLUT2_FORMAT_VERSION);
        ias_rlut2_close_file(fd);
        return NULL;
    }
    return fd;
}


/*------------------------------------------------------------------------------
NAME: ias_rlut2_open_write

PURPOSE:
    Open an alternate response linearization lookup table for writing.  This
    function will initialize the file with all of the data groups that are
    expected within the file.  The ias_rlut2_write() function can then be used
    to write DN lookup tables and correction values to the file.

    This function requires that the satellite attributes library be initialized.

    The calling function is responsible for calling ias_rlut2_close_file() so
    that the file is properly closed and all dynamically allocate memory is
    freed properly.

RETURNS:
    Upon successfully opening the file, this function will return a pointer to a
    dynamically allocated IAS_RLUT2_IO structure.  If an error is encountered,
    this function will return a NULL pointer.
------------------------------------------------------------------------------*/
IAS_RLUT2_IO *ias_rlut2_open_write
(
    const char *path /* I: Path to the RLUT2 file */
)
{
    IAS_RLUT2_IO *fd = NULL;
    char fileType[256];
    int root;
    int satNum;
    int status;
    int bandList[IAS_MAX_NBANDS];
    int nBands;
    hid_t file_attr_group_id = -1;

    /* Obtain the satellite number */
    if ((satNum = ias_sat_attr_get_satellite_number()) == ERROR)
    {
        IAS_LOG_ERROR("Satellite attributes library not initialized");
        return NULL;
    }

    /* Get the list of bands to include in the RLUT file.  Get them early in
       the routine to avoid needing to do cleanup of a number of things if it
       fails. */
    status = ias_sat_attr_get_sensor_band_numbers(IAS_MAX_SENSORS,
                IAS_NORMAL_BAND, 0, bandList, IAS_MAX_NBANDS, &nBands);
    if ((status != SUCCESS) || (nBands < 1))
    {
        IAS_LOG_ERROR("Retrieving list of normal bands");
        return NULL;
    }

    /* Allocate a structure to track the file */
    fd = malloc(sizeof(*fd));
    if (fd == NULL)
    {
        IAS_LOG_ERROR("Allocating memory for IAS_RLUT2_IO object");
        return NULL;
    }

    /* Copy the specified path into our structure */
    fd->filename = strdup(path);
    if (fd->filename == NULL)
    {
        IAS_LOG_ERROR("Allocating memory for IAS_RLUT2_IO object");
        free(fd);
        return NULL;
    }

    /* Set the file version */
    fd->file_format_version = IAS_RLUT2_FORMAT_VERSION;

    /* Open the file for writing */
    fd->file_id = H5Fcreate(path, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (fd->file_id < 0)
    {
        IAS_LOG_ERROR("Opening RLUT2 for writing: %s", path);
        free(fd->filename);
        free(fd);
        return NULL;
    }

    /* Create an attribute string identifying this file as an RLUT2 file and
       write it to the HDF5 output file. */
    status = snprintf(fileType, sizeof(fileType), IAS_RLUT2_FILE_TYPE, satNum);
    if ((status < 0) || (status >= sizeof(fileType)))
    {
        IAS_LOG_ERROR("Buffer size exceeded, requires %d characters", status);
        ias_rlut2_close_file(fd);
        return NULL;
    }
    status = H5LTset_attribute_string(fd->file_id, "/", IAS_FILE_TYPE_ATTRIBUTE,
        fileType);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing file type attribute to RLUT2 file: %s", path);
        ias_rlut2_close_file(fd);
        return NULL;
    }

    /* Write an attribute for the file format version */
    status = H5LTset_attribute_int(fd->file_id, "/", "FILE_FORMAT_VERSION",
        &fd->file_format_version, 1);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing file format version to RLUT2 file: %s", path);
        ias_rlut2_close_file(fd);
        return NULL;
    }

    /* Create an empty root file attributes group, then close it */
    file_attr_group_id = H5Gcreate(fd->file_id,
        FILE_ATTRIBUTES_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT,
        H5P_DEFAULT);
    if (file_attr_group_id < 0)
    {
        IAS_LOG_ERROR("Creating root file attributes group in %s", path);
        ias_rlut2_close_file(fd);
        return NULL;
    }
    status = H5Gclose(file_attr_group_id);
    if (status < 0)
    {
        IAS_LOG_ERROR("Closing root FILE_ATTRIBUTES group in %s", path);
        ias_rlut2_close_file(fd);
        return NULL;
    }

    /*-------------------------------------------------------------------------
     * Create all of the expected groups within the HDF5 file.  The actual data
     * sets will be written to the file by calls to ias_rlut2_write().
     * 
     * The group heirarchy is created in this function because the HDF5 library
     * spews error messages if you try to create a group that already exists or
     * attempt to access a group that does not exist.  This makes creating just
     * the groups that are necessary from within ias_rlut2_write() a little
     * messy and confusing for the end user.
     *-----------------------------------------------------------------------*/

    /* Iterate through both of the root groups /Linearity and /Nonuniformity */
    for (root = 0; root < 2; root++)
    {
        char nameBand[PATH_MAX];
        char nameRoot[PATH_MAX];
        char nameSca[PATH_MAX];
        herr_t hdfStatus;          /* HDF5 status */
        hid_t hdfIdBand;           /* Band group /Linearity/BANDXX */
        hid_t hdfIdRoot;           /* Root group /Linearity */
        hid_t hdfIdSca;            /* SCA group  /Linearity/BANDXX/SCAXX */
        int bandIndex;
        int bandNum;
        int nScas;
        int sca;

        /* Create the root group (e.g. /Linearity or /Nonuniformity) */
        strcpy(nameRoot, ((root == 0) ? RLUT2_LINEARITY : RLUT2_NONUNIFORMITY));
        hdfIdRoot = H5Gcreate(fd->file_id, nameRoot, H5P_DEFAULT, H5P_DEFAULT,
            H5P_DEFAULT);
        if (hdfIdRoot < 0)
        {
            IAS_LOG_ERROR("Creating root group %s", nameRoot);
            ias_rlut2_close_file(fd);
            return NULL;
        }

        /* Iterate through all of the bands */
        for (bandIndex = 0; bandIndex < nBands; bandIndex++)
        {
            bandNum = bandList[bandIndex];

            /* Determine the number of SCAs for this band */
            nScas = ias_sat_attr_get_scas_per_band(bandNum);
            if (nScas == ERROR)
            {
                IAS_LOG_ERROR("Obtaining number of SCAs for band %d", bandNum);
                H5Gclose(hdfIdRoot);
                ias_rlut2_close_file(fd);
                return NULL;
            }

            /* Create the name of this group */
            status = snprintf(nameBand, sizeof(nameBand), "%s/%s%02d", nameRoot,
                RLUT2_BAND, bandNum);
            if ((status < 0) || (status >= sizeof(nameBand)))
            {
                IAS_LOG_ERROR("Buffer size too small, requires %d characters",
                    status);
                H5Gclose(hdfIdRoot);
                ias_rlut2_close_file(fd);
                return NULL;
            }

            /* Create the band's group within the HDF5 file */
            hdfIdBand = H5Gcreate(hdfIdRoot, nameBand, H5P_DEFAULT, H5P_DEFAULT,
                H5P_DEFAULT);
            if (hdfIdBand < 0)
            {
                IAS_LOG_ERROR("Creating band group %s", nameBand);
                H5Gclose(hdfIdRoot);
                ias_rlut2_close_file(fd);
                return NULL;
            }

            /* For every SCA in this band, create a separate group */
            for (sca = 0; sca < nScas; sca++)
            {
                /* Create the name of the group */
                status = snprintf(nameSca, sizeof(nameSca), "%s/%s%02d",
                    nameBand, RLUT2_SCA, sca+1);
                if ((status < 0) || (status >= sizeof(nameSca)))
                {
                    IAS_LOG_ERROR(
                        "Buffer size too small, requires %d characters",
                        status);
                    H5Gclose(hdfIdBand);
                    H5Gclose(hdfIdRoot);
                    ias_rlut2_close_file(fd);
                    return NULL;
                }

                /* Create the group in the HDF5 file */
                hdfIdSca = H5Gcreate(hdfIdBand, nameSca, H5P_DEFAULT,
                    H5P_DEFAULT, H5P_DEFAULT);
                if (hdfIdSca < 0)
                {
                    IAS_LOG_ERROR("Creating SCA group %s", nameSca);
                    H5Gclose(hdfIdBand);
                    H5Gclose(hdfIdRoot);
                    ias_rlut2_close_file(fd);
                    return NULL;
                }

                /* Close the SCA group since we won't be adding data at this
                   time */
                hdfStatus = H5Gclose(hdfIdSca);
                if (hdfStatus < 0)
                {
                    IAS_LOG_ERROR("Closing SCA group %s", nameSca);
                    H5Gclose(hdfIdBand);
                    H5Gclose(hdfIdRoot);
                    ias_rlut2_close_file(fd);
                    return NULL;
                }
            }

            /* Close the band's group */
            hdfStatus = H5Gclose(hdfIdBand);
            if (hdfStatus < 0)
            {
                IAS_LOG_ERROR("Closing band group %s", nameBand);
                H5Gclose(hdfIdRoot);
                ias_rlut2_close_file(fd);
                return NULL;
            }
        }

        /* Close the root group */
        hdfStatus = H5Gclose(hdfIdRoot);
        if (hdfStatus < 0)
        {
            IAS_LOG_ERROR("Closing root group %s", nameRoot);
            ias_rlut2_close_file(fd);
            return NULL;
        }
    }

    return fd;
}


/*------------------------------------------------------------------------------
NAME: ias_rlut2_read

PURPOSE:
    This function will read either a linearity or non-uniformity set of
    correction values from an RLUT2 file for a specified band and SCA and return
    those values in an IAS_RLUT2_TABLE structure.

    This function will call ias_rlut2_realloc_table() to resize 'table'
    appropriately for the requested data that is contained within the RLUT2
    file.  The 'table' parameter may be a NULL pointer upon the first call to
    this function.  The calling application is responsible for calling
    ias_rlut2_free_table() when processing is complete to free the dynamically
    allocated memory pointed to by 'table' when finished.

RETURNS:
    This function will return a status of SUCCESS if the IAS_RLUT2_TABLE is
    successfully returned, ERROR if an error is encountered.
------------------------------------------------------------------------------*/
int ias_rlut2_read
(
    IAS_RLUT2_IO *fd,        /* I/O: File descriptor opened for writing */
    int is_linearity,        /* I:   Values to read, TRUE=LIN, FALSE=NUC */
    int band_num,            /* I:   Band number of interest */
    int sca_num,             /* I:   SCA number of interest */
    IAS_RLUT2_TABLE **table  /* I:   RLUT2 table */
)
{
    char dataSetName[2][PATH_MAX];
    char groupName[PATH_MAX];
    herr_t hdfStatus;                /* HDF5 status */
    hid_t hdfIdDataSet;              /* HDF5 Data Set ID */
    hid_t hdfIdDataSpace;
    hsize_t dims[2];

    /* Ensure the file is open */
    if ((fd == NULL) || (fd->file_id < 0))
    {
        IAS_LOG_ERROR("RLUT2 file is not open");
        return ERROR;
    }

    /* Create the RLUT2 HDF5 group and data set names */
    if (util_get_names(is_linearity, band_num, sca_num, groupName, dataSetName)
        != SUCCESS)
    {
        IAS_LOG_ERROR("Creating RLUT2 HDF5 object names");
        return ERROR;
    }

    /*--------------------------------------------------------------------------
     * Read the DN Lookup Values
     *------------------------------------------------------------------------*/

    /* Open the data set for reading */
    hdfIdDataSet = H5Dopen(fd->file_id, dataSetName[0], H5P_DEFAULT);
    if (hdfIdDataSet < 0)
    {
        IAS_LOG_ERROR("Opening data set %s", dataSetName[0]);
        return ERROR;
    }

    /* Get the data space for the data set */
    hdfIdDataSpace = H5Dget_space(hdfIdDataSet);
    if (hdfIdDataSpace < 0)
    {
        IAS_LOG_ERROR("Obtaining data space for data set: %s", dataSetName[0]);
        H5Dclose(hdfIdDataSet);
        return ERROR;
    }

    /* Obtain the dimensions of the data set */
    if (H5Sget_simple_extent_ndims(hdfIdDataSpace) != 2)
    {
        IAS_LOG_ERROR("Data set is not 2 dimensional: %s", dataSetName[0]);
        H5Sclose(hdfIdDataSpace);
        H5Dclose(hdfIdDataSet);
        return ERROR;
    }
    if (H5Sget_simple_extent_dims(hdfIdDataSpace, dims, NULL) < 0)
    {
        IAS_LOG_ERROR("Obtaining dimensions of data set: %s", dataSetName[0]);
        H5Sclose(hdfIdDataSpace);
        H5Dclose(hdfIdDataSet);
        return ERROR;
    }

    /* Allocate/reallocate the IAS_RLUT2_TABLE to the dimensions of the tables.
       This operation will not perform any actions unless the table dimensions
       have actually changed. */
    if (ias_rlut2_realloc_table(band_num, sca_num, dims[1], table) != SUCCESS)
    {
        IAS_LOG_ERROR("Allocating memory for RLUT2 table");
        H5Sclose(hdfIdDataSpace);
        H5Dclose(hdfIdDataSet);
        return ERROR;
    }

    /* Read the DN lookup table */
    hdfStatus = H5Dread(hdfIdDataSet, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
        H5P_DEFAULT, (*table)->input_dn);
    if (hdfStatus < 0)
    {
        IAS_LOG_ERROR("Reading DN lookup values for: %s", dataSetName[0]);
        H5Sclose(hdfIdDataSpace);
        H5Dclose(hdfIdDataSet);
        return ERROR;
    }

    /* Close the data space for the DN lookup data set */
    hdfStatus = H5Sclose(hdfIdDataSpace);
    if (hdfStatus < 0)
    {
        IAS_LOG_ERROR("Closing data space for: %s", dataSetName[0]);
        H5Dclose(hdfIdDataSet);
        return ERROR;
    }

    /* Close the data set for the DN lookup values */
    hdfStatus = H5Dclose(hdfIdDataSet);
    if (hdfStatus < 0)
    {
        IAS_LOG_ERROR("Closing data set: %s", dataSetName[0]);
        return ERROR;
    }

    /*--------------------------------------------------------------------------
     * Read the LIN or NUC Correction Values
     *------------------------------------------------------------------------*/

    /* Open the data set for reading */
    hdfIdDataSet = H5Dopen(fd->file_id, dataSetName[1], H5P_DEFAULT);
    if (hdfIdDataSet < 0)
    {
        IAS_LOG_ERROR("Opening data set %s", dataSetName[1]);
        return ERROR;
    }

    /* Get the data space for the data set */
    hdfIdDataSpace = H5Dget_space(hdfIdDataSet);
    if (hdfIdDataSpace < 0)
    {
        IAS_LOG_ERROR("Obtaining data space for data set: %s", dataSetName[1]);
        H5Dclose(hdfIdDataSet);
        return ERROR;
    }

    /* Obtain the dimensions of the data set */
    if (H5Sget_simple_extent_ndims(hdfIdDataSpace) != 2)
    {
        IAS_LOG_ERROR("Data set is not 2 dimensional: %s", dataSetName[1]);
        H5Sclose(hdfIdDataSpace);
        H5Dclose(hdfIdDataSet);
        return ERROR;
    }
    if (H5Sget_simple_extent_dims(hdfIdDataSpace, dims, NULL) < 0)
    {
        IAS_LOG_ERROR("Obtaining dimensions of data set: %s", dataSetName[1]);
        H5Sclose(hdfIdDataSpace);
        H5Dclose(hdfIdDataSet);
        return ERROR;
    }

    /* Verify that the dimensions of the correction data set are the same as the
       DN lookup values.  If these values are not the same, the RLUT2 file has
       an error in it. */
    if ((dims[0] != (*table)->num_detectors) ||
        (dims[1] != (*table)->num_values))
    {
        IAS_LOG_ERROR("Dimension mismatch for data set %s:\n"
            "    DN lookup values:  %d x %d\n"
            "    Correction values: %Lu x %Lu\n",
            groupName, (*table)->num_detectors, (*table)->num_values, dims[0],
            dims[1]);
        H5Sclose(hdfIdDataSpace);
        H5Dclose(hdfIdDataSet);
        return ERROR;
    }

    /* Read the DN lookup table */
    hdfStatus = H5Dread(hdfIdDataSet, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
        H5P_DEFAULT, (*table)->output_correction);
    if (hdfStatus < 0)
    {
        IAS_LOG_ERROR("Reading output correction values for: %s",
            dataSetName[1]);
        H5Sclose(hdfIdDataSpace);
        H5Dclose(hdfIdDataSet);
        return ERROR;
    }

    /* Close the data space for the DN lookup data set */
    hdfStatus = H5Sclose(hdfIdDataSpace);
    if (hdfStatus < 0)
    {
        IAS_LOG_ERROR("Closing data space for: %s", dataSetName[1]);
        H5Dclose(hdfIdDataSet);
        return ERROR;
    }

    /* Close the data set for the DN lookup values */
    hdfStatus = H5Dclose(hdfIdDataSet);
    if (hdfStatus < 0)
    {
        IAS_LOG_ERROR("Closing data set: %s", dataSetName[1]);
        return ERROR;
    }
    return SUCCESS;
}


/*------------------------------------------------------------------------------
NAME: ias_rlut2_realloc_table

PURPOSE:
    This function with either allocate a new IAS_RLUT2_TABLE (if table == NULL)
    or reallocate the memory within an existing table to account for the band,
    SCA, and number of values specified.  The reallocation is only performed if
    necessary to save execution time.
  
    The calling application should initialize its IAS_RLUT2_TABLE pointer to
    NULL prior to calling this function and it should free up the memory
    associated with the table by calling ias_rlut2_free_table() when it has
    finished working with the table.  The application should NOT call the free
    function prior to each call of this function because that will disable the
    time saving reallocation features.

RETURNS:
    This function will return SUCCESS if the table is successfully allocated or
    resized for the specified band.  If an error is encountered all of the
    dynamically allocated memory will be freed, the table pointer will be set to
    NULL, and the function will return a status of ERROR.
------------------------------------------------------------------------------*/
int ias_rlut2_realloc_table
(
    int band_num,            /* I: Band number of interest */
    int sca_num,             /* I: SCA number of interest */
    int num_values,          /* I: Number of values per detector */
    IAS_RLUT2_TABLE **table  /* I/O: Ptr to a dynamically allocate table */
)
{
    int nDetectors;
    int nScas;
    size_t bytes;

    /* Obtain valid information from the satellite attributes library */
    nDetectors = ias_sat_attr_get_detectors_per_sca(band_num);
    nScas = ias_sat_attr_get_scas_per_band(band_num);
    if ((nDetectors == ERROR) || (nScas == ERROR))
    {
        IAS_LOG_ERROR("Invalid band number specified %d", band_num);
        return ERROR;
    }

    /* Did the calling function specify a valid SCA? */
    if ((sca_num < 1) || (sca_num > nScas))
    {
        IAS_LOG_ERROR("Invalid SCA specified %d", sca_num);
        return ERROR;
    }

    /* Allocate a structure if one hasn't been already */
    if (*table == NULL)
    {
        *table = malloc(sizeof(IAS_RLUT2_TABLE));
        if (*table == NULL)
        {
            IAS_LOG_ERROR("Allocating an RLUT2 table");
            return ERROR;
        }

        /* Initialize the new table */
        (*table)->input_dn = NULL;
        (*table)->output_correction = NULL;
        (*table)->band_num = 0;
        (*table)->num_detectors = 0;
        (*table)->num_values = 0;
        (*table)->sca_num = 0;
    }

    /* Accept the band number and SCA number as valid */
    (*table)->band_num = band_num;
    (*table)->sca_num = sca_num;

    /* If the tables are already allocated and are the same size as what they
       need to be, we're done */
    if (((*table)->input_dn != NULL) && ((*table)->output_correction != NULL)
        && ((*table)->num_detectors == nDetectors)
        && ((*table)->num_values == num_values))
    {
        return SUCCESS;
    }

    /* We need to free memory for the DN lookup and correction tables and
       initialize the structure just in case we encounter an error while
       allocating memory. */
    free((*table)->input_dn);
    free((*table)->output_correction);
    (*table)->input_dn = NULL;
    (*table)->output_correction = NULL;
    (*table)->num_detectors = 0;
    (*table)->num_values = 0;

    /* Allocate memory for the two arrays */
    bytes = nDetectors * num_values * sizeof(float);
    (*table)->input_dn = malloc(bytes);
    if ((*table)->input_dn == NULL)
    {
        long int my_size = (long int)bytes;
        IAS_LOG_ERROR("Allocating %ld bytes of memory for DN lookup", my_size);
        free(*table);
        *table = NULL;
        return ERROR;
    }
    (*table)->output_correction = malloc(bytes);
    if ((*table)->output_correction == NULL)
    {
        long int my_size = (long int)bytes;
        IAS_LOG_ERROR("Allocating %ld bytes of memory for output correction",
            my_size);
        free((*table)->input_dn);
        free(*table);
        *table = NULL;
        return ERROR;
    }
    (*table)->num_detectors = nDetectors;
    (*table)->num_values = num_values;
    return SUCCESS;
}


/*------------------------------------------------------------------------------
NAME: ias_rlut2_write

PURPOSE:
    This function will write a set of DN lookup values and their corresponding
    correction values for either a Linearity (LIN) correction or a Nonuniformity
    (NUC) correction.

RETURNS:
    This function will return SUCCESS if the values are successfully written to
    the RLUT2 file, ERROR if an error is encountered.
------------------------------------------------------------------------------*/
int ias_rlut2_write
(
    IAS_RLUT2_IO *fd,       /* I/O: File descriptor opened for writing */
    int is_linearity,       /* I:   TRUE=LIN values, FALSE=NUC values */
    IAS_RLUT2_TABLE *table  /* I:   RLUT2 table */
)
{
    char dataSetName[2][PATH_MAX];
    char groupName[PATH_MAX];
    herr_t hdfStatus;                /* HDF5 status */
    hid_t hdfIdGroup;                /* HDF5 Group ID */
    hsize_t dims[2];
    int status;

    /* Ensure the file is open */
    if ((fd == NULL) || (fd->file_id < 0))
    {
        IAS_LOG_ERROR("RLUT2 file is not open");
        return ERROR;
    }

    /* Validate the RLUT2 table */
    if (table == NULL)
    {
        IAS_LOG_ERROR("RLUT2 table not allocated");
        return ERROR;
    }
    if (table->num_detectors !=
        ias_sat_attr_get_detectors_per_sca(table->band_num))
    {
        IAS_LOG_ERROR("RLUT2 table contains an invalid number of detectors (%d)"
           " for band %d", table->num_detectors,
           ias_sat_attr_get_detectors_per_sca(table->band_num));
        return ERROR;
    }

    /* Initialize the dimensions of the data that we'll be writing */
    dims[0] = table->num_detectors;
    dims[1] = table->num_values;

    /* Create the RLUT2 HDF5 group and data set names */
    if (util_get_names(is_linearity, table->band_num, table->sca_num, groupName,
        dataSetName) != SUCCESS)
    {
        IAS_LOG_ERROR("Creating RLUT2 HDF5 object names");
        return ERROR;
    }

    /* Open the group so that the data sets can be added */
    hdfIdGroup = H5Gopen(fd->file_id, groupName, H5P_DEFAULT);
    if (hdfIdGroup < 0)
    {
        IAS_LOG_ERROR("Opening group %s", groupName);
        return ERROR;
    }

    /* Write the DN lookup values to the HDF5 file */
    status = util_write_array(hdfIdGroup, dims, dataSetName[0],
        table->input_dn);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Writing DN values for %s", groupName);
        H5Gclose(hdfIdGroup);
        return ERROR;
    }

    /* Write the correction values to the HDF5 file */
    status = util_write_array(hdfIdGroup, dims, dataSetName[1],
        table->output_correction);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Writing output correction values for %s", groupName);
        H5Gclose(hdfIdGroup);
        return ERROR;
    }

    /* Close the open group */
    hdfStatus = H5Gclose(hdfIdGroup);
    if (hdfStatus < 0)
    {
        IAS_LOG_ERROR("Closing group %s", groupName);
        return ERROR;
    }
    return SUCCESS;
}
