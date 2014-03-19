/*************************************************************************
 NAME:     ias_rlut_open_close_file

 PURPOSE:  Routines implementing a public interface to open and close
           close an RLUT file

 ROUTINES: ias_rlut_initialize_file (internal routine)
           ias_rlut_open_file       (public routine)
           ias_rlut_close_file      (public routine)
***************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "hdf5.h"                 /* Main HDF5 header */
#include "hdf5_hl.h"              /* H5LT_set/get_attribute routines */
#include "ias_logging.h"
#include "ias_miscellaneous.h"
#include "ias_rlut.h"
#include "ias_rlut_private.h"
#include "ias_types.h"
#include "ias_const.h"


/***************************************************************************
 NAME:     ias_rlut_initialize

 PURPOSE:  Internal routine that initializes a newly allocated IAS_RLUT_IO
           data structure

 RETURNS:  Nothing
****************************************************************************/
static void ias_rlut_initialize
(
    IAS_RLUT_IO *rlut                /* O:  HDF IO data structure */
)
{
    memset(rlut, 0, sizeof(*rlut));
    rlut->file_format_version = IAS_RLUT_FORMAT_VERSION;
    rlut->file_id = -1;
}



/***************************************************************************
 NAME:     ias_rlut_is_rlut_file

 PURPOSE:  Looks for the RLUT file type attribute string in an HDF5 file

 RETURNS:  1 (TRUE) if file is an RLUT file, 0 (FALSE) if it is not
****************************************************************************/
int ias_rlut_is_rlut_file
(
    const char *rlut_filename            /* Name of RLUT file */
)
{
    return ias_misc_check_file_type(rlut_filename, IAS_RLUT_FILE_TYPE);
}



/**************************************************************************
 NAME:     ias_rlut_open_file

 PURPOSE:  Opens an RLUT file for reading or writing

 RETURNS:  Pointer to a valid IAS_RLUT_IO data structure if successful;
           NULL pointer if there's an error
***************************************************************************/
IAS_RLUT_IO *ias_rlut_open_file
(
    const char *rlut_filename,            /* I: Name of RLUT file */
    IAS_ACCESS_MODE access_mode           /* I: Desired file access mode */
)
{
    IAS_RLUT_IO *rlut = NULL;             /* Pointer to return buffer */
    herr_t hdf_status;                    /* HDF5 error status */


    /* Allocate the data structure buffer */
    rlut = malloc(sizeof(*rlut));
    if (rlut == NULL)
    {
        IAS_LOG_ERROR("Allocating IAS_RLUT_IO data structure memory");
        return NULL;
    }

    /* Initialize the IAS_RLUT_IO data structure.  This is to put the
       HDF5 IDs to a known value representing an "invalid" state (-1) */
    ias_rlut_initialize(rlut);

    /* Start populating the structure with known information */
    rlut->filename = strdup(rlut_filename);
    if (rlut->filename == NULL)
    {
        IAS_LOG_ERROR("Saving RLUT file name %s", rlut_filename);
        free(rlut);
        return NULL;
    }

    /* If the mode is IAS_WRITE (re)create an empty RLUT file, and
       create the root FILE_ATTTRIBUTES and LINEARIZATION_PARAMETERS
       groups */
    if (access_mode == IAS_WRITE)
    {
        hid_t file_attr_group_id = -1;
        hid_t linearization_param_group_id = -1;

        /* Get a valid file ID */
        rlut->file_id = H5Fcreate(rlut_filename, H5F_ACC_TRUNC, H5P_DEFAULT,
            H5P_DEFAULT);
        if (rlut->file_id < 0)
        {
            IAS_LOG_ERROR("Opening RLUT file %s for writing",
                rlut_filename);
            free(rlut->filename);
            free(rlut);
            return NULL;
        }

        /* Write an attribute identifying this file as an RLUT file.  This
           will allow a validation check prior to doing anything with/to 
           this file */
        if (H5LTset_attribute_string(rlut->file_id, "/",
            IAS_FILE_TYPE_ATTRIBUTE, IAS_RLUT_FILE_TYPE) < 0)
        {
            IAS_LOG_ERROR("Recording file type attribute information to RLUT "
                "file %s", rlut_filename);
            ias_rlut_close_file(rlut);
            return NULL;
        }

        /* Write an RLUT format version attribute */
        if (H5LTset_attribute_int(rlut->file_id, "/", "FILE_FORMAT_VERSION",
            &rlut->file_format_version, 1) < 0)
        {
            IAS_LOG_ERROR("Recording RLUT file format version information");
            H5Fclose(rlut->file_id);
            free(rlut->filename);
            free(rlut);
            return NULL;
        }
 
        /* Create an empty root file attributes group, then close it */
        file_attr_group_id = H5Gcreate(rlut->file_id,
            FILE_ATTRIBUTES_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT,
            H5P_DEFAULT);
        if (file_attr_group_id < 0)
        {
            IAS_LOG_ERROR("Creating root file attributes group");
            ias_rlut_close_file(rlut);
            return NULL;
        }
        hdf_status = H5Gclose(file_attr_group_id);
        if (hdf_status < 0)
        {
            IAS_LOG_ERROR("Closing root FILE_ATTRIBUTES group");
            ias_rlut_close_file(rlut);
            return NULL;
        }

        /* Create an empty linearization parameters group, then close it */
        linearization_param_group_id = H5Gcreate(rlut->file_id,
            LINEARIZATION_PARAMS_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT,
            H5P_DEFAULT);
        if (linearization_param_group_id < 0)
        {
            IAS_LOG_ERROR("Creating root linearization parameters group");
            ias_rlut_close_file(rlut);
            return NULL;
        }
        hdf_status = H5Gclose(linearization_param_group_id);
        if (hdf_status < 0)
        {
            IAS_LOG_ERROR("Closing root linearization parameters group");
            ias_rlut_close_file(rlut);
            return NULL;
        }

        /* Create an empty TIRS secondary parameters group, then close it */
        linearization_param_group_id = H5Gcreate( rlut->file_id,
            TIRS_SECONDARY_LINEARIZATION_GROUP_NAME, H5P_DEFAULT,
            H5P_DEFAULT, H5P_DEFAULT );
        if (linearization_param_group_id < 0)
        {
            IAS_LOG_ERROR("Creating root TIRS secondary parameters group");
            ias_rlut_close_file(rlut);
            return NULL;
        }
        hdf_status = H5Gclose(linearization_param_group_id);
        if (hdf_status < 0)
        {
            IAS_LOG_ERROR("Closing root TIRS secondary parameters group");
            ias_rlut_close_file(rlut);
            return NULL;
        }

    }
    else
    {
        /* Set the HDF5 internal access mode based on the IAS access
           mode.  Read access is the only other access mode currently
           supported */
        if (access_mode != IAS_READ)
        {
            if (access_mode == IAS_UPDATE)
            {
                IAS_LOG_ERROR("Update capability is not currently supported "
                    "in the RLUT I/O Library");
            }
            else
            {
                IAS_LOG_ERROR("Invalid IAS file access mode %d", access_mode);
            }
            free(rlut->filename);
            free(rlut);
            return NULL;
        }

        /* Make sure it's a valid RLUT file */
        if (!ias_rlut_is_rlut_file(rlut_filename))
        {
            IAS_LOG_ERROR("File %s is not a valid RLUT file", rlut_filename);
            free(rlut->filename);
            free(rlut);
            return NULL;
        }

        /* Open the file */
        rlut->file_id = H5Fopen(rlut_filename, H5F_ACC_RDONLY, H5P_DEFAULT);
        if (rlut->file_id < 0)
        {
            IAS_LOG_ERROR("Opening RLUT file %s for reading", rlut_filename);
            free(rlut->filename);
            free(rlut);
            return NULL;
        }

        /* Read the RLUT file format version.  Check to make sure it's the
           "correct" version */
        if (H5LTget_attribute_int(rlut->file_id, "/", "FILE_FORMAT_VERSION",
             &rlut->file_format_version) < 0)
        {
            IAS_LOG_ERROR("Reading RLUT format version from %s",
                rlut_filename);
            ias_rlut_close_file(rlut);
            return NULL;
        }
        if (rlut->file_format_version != IAS_RLUT_FORMAT_VERSION)
        {
            IAS_LOG_ERROR("RLUT file format version %d is incorrect--"
                "should be %d", rlut->file_format_version,
                IAS_RLUT_FORMAT_VERSION);
            ias_rlut_close_file(rlut);
            return NULL;
        }
    }

    return rlut;
}   /* END ias_rlut_open_file */



/************************************************************************
 NAME:     ias_rlut_close_file

 PURPOSE:  Closes an open RLUT file and frees the allocated IAS_RLUT_IO
           block

 RETURNS:  Integer status code of SUCCESS or ERROR
*************************************************************************/
int ias_rlut_close_file
(
    IAS_RLUT_IO *rlut              /* I: RLUT IO structure to close */
)
{
    int status = SUCCESS;


    /* Verify we have a valid IAS_RLUT_IO parameter */
    if (rlut == NULL)
    {
        IAS_LOG_ERROR("NULL pointer to IAS_RLUT_IO data block");
        return ERROR;
    }

    /* Close the main RLUT file handle */
    if (rlut->file_id >= 0)
    {
        if (H5Fclose(rlut->file_id) < 0)
        {
            IAS_LOG_ERROR("Closing RLUT file %s", rlut->filename);
            status = ERROR;
        }
    }

    /* Free up the IAS_RLUT_IO internal memory */
    free(rlut->filename);
    free(rlut);

    return status;
}   /* END ias_rlut_close_file */
