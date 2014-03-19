/*************************************************************************
NAME: ias_misc_check_file_type

PURPOSE: Determine whether the file input matches the requested file type.
    This routine is utilized by several routines to verify the correct file
    type is being read.  Each of the file IO libraries that use this routine
    need to write an attribute to the root dataset with the
    IAS_FILE_TYPE_ATTRIBUTE name.

RETURNS: 1 (TRUE) if the file type matches
         0 (FALSE) if the file type does not match

Notes:
    - This routine doesn't use H5LTget_attribute_ndims or 
      H5LTget_attribute_info for the file type attribute since they don't
      appear to work properly for strings.
**************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>         /* access routine */
#include "hdf5.h"
#include "hdf5_hl.h"        /* H5LT routines */
#include "ias_const.h"
#include "ias_miscellaneous.h"
#include "ias_logging.h"

#define FILE_TYPE_MAX_LENGTH    40

int ias_misc_check_file_type
(
    const char *filename,           /* I: Input file name */
    const char *expected_file_type  /* I: Expected file type */
)
{
    hid_t file_id;                          /* HDF file ID */
    hid_t attr_id;                          /* HDF attribute ID */
    hid_t data_type_id;                     /* HDF data type ID */
    H5A_info_t attr_info;                   /* HDF attribute information */
    char file_type[FILE_TYPE_MAX_LENGTH];   /* File type read from the file */

    /* Make sure the file exists before attempting to open it to avoid
       a bunch of HDF messages */
    if (access(filename, R_OK) != 0)
    {
        IAS_LOG_WARNING("File %s does not exist", filename);
        return FALSE;
    }

    /* Open the input file */
    file_id =  H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file_id < 0)
    {
        /* Can't open the file, so simply return that it isn't the expected
           file type */
        IAS_LOG_WARNING("Unable to open file %s to test whether it is a %s",
                filename, expected_file_type);
        return FALSE;
    }

    /* Verify the file type attribute exists */
    if (!H5Aexists_by_name(file_id, "/", IAS_FILE_TYPE_ATTRIBUTE, H5P_DEFAULT))
    {
        /* Attribute doesn't exist, so can't be the expected file type */
        IAS_LOG_WARNING("File %s is not a %s", filename, expected_file_type);
        H5Fclose(file_id);
        return FALSE;
    }

    /* Open the attribute */
    attr_id = H5Aopen_by_name(file_id, "/", IAS_FILE_TYPE_ATTRIBUTE, 
            H5P_DEFAULT, H5P_DEFAULT);
    if (attr_id < 0)
    {
        IAS_LOG_WARNING("Failed to open file type attribute for %s", filename);
        H5Fclose(file_id);
        return FALSE;
    }

    /* Get the attribute info so it can be confirmed that it will fit in the
       buffer allocated */
    if (H5Aget_info(attr_id, &attr_info) < 0)
    {
        /* Couldn't get the attribute info, so consider that an error */
        IAS_LOG_WARNING("Failed to get file type attribute information for %s",
                filename);
        H5Aclose(attr_id);
        H5Fclose(file_id);
        return FALSE;
    }

    /* If the attribute is too long, it can't be our file type. Compare to
       the size of the file type string (minus 1 for the null terminator). */
    if (attr_info.data_size > (FILE_TYPE_MAX_LENGTH - 1))
    {
        IAS_LOG_WARNING("File %s is not a %s", filename, expected_file_type);
        H5Aclose(attr_id);
        H5Fclose(file_id);
        return FALSE;
    }

    /* Make sure attribute is the correct type */
    data_type_id = H5Aget_type(attr_id);
    if (H5Tget_class(data_type_id) != H5T_STRING)
    {
        /* Attribute isn't a string, so not our file type */
        IAS_LOG_WARNING("File %s is not a %s", filename, expected_file_type);
        H5Tclose(data_type_id);
        H5Aclose(attr_id);
        H5Fclose(file_id);
        return FALSE;
    }

    /* Close HDF ID that is no longer needed */
    H5Tclose(data_type_id);
    data_type_id = -1;
    H5Aclose(attr_id);
    attr_id = -1;

    /* Get the file type attribute */
    if (H5LTget_attribute_string(file_id, "/", IAS_FILE_TYPE_ATTRIBUTE,
                file_type) < 0)
    {
        IAS_LOG_WARNING("Unable to read file type attribute from %s", filename);
        H5Fclose(file_id);
        return FALSE;
    }

    /* Close the HDF5 File */
    H5Fclose(file_id);

    /* Make sure the file type agrees */
    if (strcmp(file_type, expected_file_type) != 0)
    {
        /* Issue a debug message when the file type isn't the expected one.
           This should not be a warning message since it is normal for some
           applications to query a file to figure out what type it is under
           some conditions and doing that should not result in a spew of 
           warning messages since it is normal. */
        IAS_LOG_DEBUG("File %s is not a %s", filename, expected_file_type);
        return FALSE;
    }

    return TRUE;    
}
