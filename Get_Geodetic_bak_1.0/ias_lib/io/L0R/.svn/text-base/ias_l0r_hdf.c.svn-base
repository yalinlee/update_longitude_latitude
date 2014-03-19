/*-----------------------------------------------------------------------------

NAME: ias_l0r_hdf.c

PURPOSE: HDF related functions used by the library

ALGORITHM REFERENCES: NONE

NOTES:

-----------------------------------------------------------------------------*/

#include <string.h>
#include <sys/stat.h>

/* project included headers */
#include "ias_l0r.h"
#include "ias_l0r_hdf.h"
#include "ias_logging.h"
#include "ias_types.h"
#include "ias_l0r_version.h"

#define ATTRIBUTE_LIBRARY_VERSION "L0R Format Version"

/*-----------------------------------------------------------------------------
 NAME:     ias_l0r_hdf_create_group

 PURPOSE:  Creates the specified group name in the file passed

 RETURNS:  SUCCESS - Group created
           ERROR   - Unable to create group

 ALGORITHM REFERENCES: NONE
 -----------------------------------------------------------------------------*/
static int ias_l0r_hdf_create_group
(
    hid_t file_id, /* I: Structure used in I/O */
    const char *group_name /* I: Name of group to create */
)
{
    hid_t group_id;
    herr_t hdf_status;

    if (file_id < 0)
    {
       IAS_LOG_ERROR("Invalid file ID received");
       return ERROR;
    }
    if (group_name == NULL)
    {
        IAS_LOG_ERROR("NULL group name received");
        return ERROR;
    }
    else if (strcmp(group_name, "/") == 0)
    {
        IAS_LOG_ERROR("Can't create root");
        return ERROR;
    }
    else if (strlen(group_name) == 0)
    {
        IAS_LOG_ERROR("Empty group name received");
        return ERROR;
    }

    group_id = H5Gcreate(file_id,
        group_name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (group_id < 0)
    {
        IAS_LOG_ERROR("Error creating %s in file %d", group_name, file_id);
        return ERROR;
    }
    hdf_status = H5Gclose(group_id);
    if (hdf_status < 0)
    {
         IAS_LOG_ERROR("Error closing group %i", group_id);
    }
    return SUCCESS;
}

/*-----------------------------------------------------------------------------
 NAME:     ias_l0r_hdf_establish_file

 PURPOSE:  Establishes access to the specified file, if it does not exist
           it is created (if that option is set).

 RETURNS:  SUCCESS - Access established
           ERROR   - Unable to establish access

 ALGORITHM REFERENCES: NONE
 -----------------------------------------------------------------------------*/
int ias_l0r_hdf_establish_file
(
    const char *path, /* I: Path of the file */
    const char *filename, /* I: Name of the file */
    hid_t *file_id, /* O: The HDF ID of the file established */
    const IAS_ACCESS_MODE file_mode_wanted, /* I: Read Only / Write mode */
    const int create_if_absent /* I: if the file doesn't exist,
                                 should it be created */
)
{
    herr_t hdf_status;

    /* open or create the file   */
    if (*file_id <= 0)
    {
        struct stat file_info;

        char complete_filename[PATH_MAX];
        unsigned int temp_version = 0;

        /* verify the size is acceptable for the string operation      */
        if ((strlen(path) + strlen(filename) + 2) < PATH_MAX)
        {
            snprintf(complete_filename, PATH_MAX, "%s/%s", path, filename);
        }
        else
        {
            IAS_LOG_ERROR("File name of %s is to long. The complete"
                          " path can be no longer than %i",
                          complete_filename, PATH_MAX);
            return ERROR;
        }

        /* open files that exist */
        if (stat(complete_filename, &file_info) == 0)
        {
            switch (file_mode_wanted)
            {
                case IAS_READ:
                    *file_id = H5Fopen(complete_filename, H5F_ACC_RDONLY,
                        H5P_DEFAULT);
                    break;
                /* write and update both require file write access  */
                case IAS_WRITE:
                case IAS_UPDATE:
                    *file_id = H5Fopen(complete_filename, H5F_ACC_RDWR,
                        H5P_DEFAULT);
                    break;
                default:
                    IAS_LOG_ERROR("Unexpected file mode, %d, received for %s",
                        file_mode_wanted, filename);
                    return ERROR;
            }
            if (*file_id < 0)
            {
                IAS_LOG_ERROR("Error opening file %s", complete_filename);
                return ERROR;
            }

            /* Check to make sure the library and file version match */
            hdf_status = H5LTfind_attribute(*file_id,
                ATTRIBUTE_LIBRARY_VERSION);
            if(hdf_status == 1)
            {
                hdf_status = H5LTget_attribute_uint(*file_id, "/",
                    ATTRIBUTE_LIBRARY_VERSION, &temp_version);
            }
            else
            {
                hdf_status = -1;
            }
            /* If the call to get the version on the file was successful
               compare to the lib version, if they don't match log a warning */
            if ((hdf_status >= 0) && IAS_L0R_LIBRARY_VERSION != temp_version) 
            {
                IAS_LOG_WARNING("The %s: %u does "
                    "not match for file %s: %u "
                    "Difficulties in reading the file are likely",
                    ATTRIBUTE_LIBRARY_VERSION,
                    IAS_L0R_LIBRARY_VERSION,
                    complete_filename,
                    temp_version);
            }
            else if (hdf_status < 0 && temp_version == 0)
            {
                IAS_LOG_WARNING("Unable to retrieve the %s for file: %s. "
                    "Difficulties in reading the file are likely.",
                    ATTRIBUTE_LIBRARY_VERSION,
                    complete_filename);
            }
        }
        else
        {
            /*if attempting to read a non-existing file leave the id
             uninitialized but return success.  This
             condition is encountered by the _records_count routines
             which use this routine. Since empty files are required
             not to exist this is a size of 0*/
            if (file_mode_wanted == IAS_READ || create_if_absent != TRUE)
            {
                *file_id = -1;
                return SUCCESS;
            }
            /* create a new file using default properties. */
            *file_id = H5Fcreate(complete_filename, H5F_ACC_EXCL,
                H5P_DEFAULT, H5P_DEFAULT);
            if (*file_id < 0)
            {
                IAS_LOG_ERROR("Error creating file %s", filename);
                return ERROR;
            }

            /* set the library version as an attribute */
            temp_version = IAS_L0R_LIBRARY_VERSION;
            hdf_status = H5LTset_attribute_uint(*file_id, "/",
                ATTRIBUTE_LIBRARY_VERSION, &temp_version, 1);
            if (hdf_status < 0)
            {
                IAS_LOG_ERROR("Unable to set library version "
                    "attribute on file");
                H5Fclose(*file_id);
                return ERROR;
            }

        }
    }
    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_hdf_establish_groups

 PURPOSE: routine for establishing groups

 RETURNS: SUCCESS - Groups established
          FAILURE - Groups not established

 NOTES:
******************************************************************************/
int ias_l0r_hdf_establish_groups
(
    const hid_t file_id, /* I: HDF file ID */
    const char *path /* I: to establish groups at */
)
{
    char *group = NULL;
    char group_structure[IAS_L0R_HDF_PATH_MAX] = "";
    char temp_path[IAS_L0R_HDF_PATH_MAX];

    int status = -1;

    if (file_id < 0)
    {
        IAS_LOG_ERROR("Invalid file ID received, %d", file_id);
        return ERROR;
    }
    if (path == NULL)
    {
       IAS_LOG_ERROR("NULL path pointer received");
       return ERROR;
    }
    if (strlen(path) == 0)
    {
        /* empty path is root? always exists, success */
        return SUCCESS;
    }

    strncpy(temp_path, path, IAS_L0R_HDF_PATH_MAX);
    group = strtok(temp_path, "/");
 

    while (group != NULL)
    {
        strcat(group_structure, group);

        status = ias_l0r_hdf_object_exists(file_id, group_structure);
        if (status != IAS_L0R_OBJECT_FOUND)
        {
            status = ias_l0r_hdf_create_group(file_id, group_structure);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Unable to create group %s in file %d",
                    group_structure,
                    file_id);
                return ERROR;
            }
        }

        strcat(group_structure, "/");
        group = strtok(NULL, "/");
    }

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_hdf_establish_table

 PURPOSE: routine for establishing tables

 RETURNS: SUCCESS - Table established
          FAILURE - Table not established

 NOTES:
******************************************************************************/
int ias_l0r_hdf_establish_table
(
    const char *path, /* I: Path within the HDF file */
    const char *table_name, /* I: Table to create */
    const hid_t file_id, /* I: HDF ID for the file to work with */
    hid_t *table_id, /* O: Table ID for the new table */
    const size_t table_size, /* I: Number of column for table */
    const size_t struct_size, /* I: Size in bytes of source data structure */
    const char** field_names, /* I: Column names */
    const size_t *offset, /* I: Offsets in bytes of each of the columns */
    const hid_t *field_type /* I: HDF Types for each of the columns */
)
{
    char complete_table_name[PATH_MAX];
    herr_t hdf_status;
    hsize_t chunk_size = 10;
    int *fill_data = NULL;
    int compress = 0;
    int status;
 
    if (file_id < 0)
    {
        IAS_LOG_ERROR("Invalid file ID received %d", file_id);
        return ERROR;
    }
    if (path == NULL)
    {
        IAS_LOG_ERROR("NULL path received");
        return ERROR;
    }
    if (table_name == NULL)
    {
        IAS_LOG_ERROR("NULL name received");
        return ERROR;
    }

    if (*table_id > 0)
    {
        /* access for the table is already provided */
        return SUCCESS;
    }

    /* create a full path with the table name */
    status = snprintf(complete_table_name, sizeof(complete_table_name),
        "%s/%s", path, table_name);
    if (status <= 0)
    {
        IAS_LOG_ERROR("Unable to build complete path with path - %s and "
            "table_name - %s", path, table_name);
        return ERROR;
    }
 
    if ( ias_l0r_hdf_object_exists(file_id, complete_table_name) ==
                                        IAS_L0R_OBJECT_NOT_FOUND )
    {
        /*the table does not exist yet, so if must be created
        *start by establishing that the groups exist*/
        status = ias_l0r_hdf_establish_groups(file_id, path);
        if (status == ERROR)
        {
            IAS_LOG_ERROR("Error establishing the existence of %s", path);
            return ERROR;
        }

        hdf_status = H5TBmake_table(table_name, file_id, complete_table_name,
            table_size, 0,
            struct_size, field_names, offset, field_type, chunk_size, fill_data,
            compress, NULL );
        if (hdf_status < 0)
        {
            int i = 0;
            IAS_LOG_ERROR(
                "Error creating %s of size %zd", complete_table_name,
                table_size);
            while(i < table_size)
            {
                const int type_length = 32;
                char temp_type[type_length];/*string for the types printed*/

                if (field_type[i] == H5T_STD_I8LE)
                {
                    strncpy(temp_type, "int8_t", type_length);
                }
                else if (field_type[i] == H5T_STD_U8LE)
                {
                    strncpy(temp_type, "uint8_t", type_length);
                }
                else if (field_type[i] == H5T_STD_I16LE)
                {
                    strncpy(temp_type, "int16_t", type_length);
                }
                else if (field_type[i] == H5T_STD_U16LE)
                {
                    strncpy(temp_type, "uint16_t", type_length);
                }
                else if (field_type[i] == H5T_STD_I32LE)
                {
                    strncpy(temp_type, "int32_t", type_length);
                }
                else if (field_type[i] == H5T_STD_U32LE)
                {
                    strncpy(temp_type, "uint32_t", type_length);
                }
                else if (field_type[i] == H5T_IEEE_F32LE)
                {
                    strncpy(temp_type, "float", type_length);
                }
                else if (field_type[i] == H5T_IEEE_F64LE)
                {
                    strncpy(temp_type, "double", type_length);
                }
                else if (field_type[i] == H5T_C_S1)
                {
                    strncpy(temp_type, "char", type_length);
                }
                else
                {
                    snprintf(temp_type, type_length,
                    "unknown type %d", field_type[i]);
                }

                IAS_LOG_ERROR(
                    "%s offset:%zd type:%s", field_names[i], offset[i],
                    temp_type);
                i++;
            }
            return ERROR;
        }
    }

    /* at this point we have established that the table exists
       open access for future use */
    *table_id = H5Dopen(file_id, complete_table_name, H5P_DEFAULT);
    if (*table_id < 0)
    {
        IAS_LOG_ERROR("Error opening %s", table_name);
        return ERROR;
    }

    return SUCCESS;
}

/******************************************************************************
 NAME: ias_l0r_hdf_object_exists

 PURPOSE: Determines if an HDF object exists

 RETURNS: IAS_LOR_OBJECT_FOUND- Object exists
          IAS_LOR_NOT_OBJECT_FOUND- Object does not exists
          ERROR- It can not be determined if object exists
******************************************************************************/
int ias_l0r_hdf_object_exists
(
    const hid_t file_id, /* I: HDF file ID */
    const char *object_to_search_for /* I: Object Name to find */
)
{
    int len;
    char *next_slash;
    char name[1000];  /* buffer for intermediate names, sized sufficiently
                         large that nothing should have a problem */

    if (file_id < 0)
    {
        IAS_LOG_ERROR("Invalid file ID received");
        return ERROR;
    }
    if (object_to_search_for == NULL)
    {
        IAS_LOG_ERROR("NULL search string received");
        return ERROR;
    }
    len = strlen(object_to_search_for);
    if ((len == 0) || (len > sizeof(name) - 1))
    {
        IAS_LOG_ERROR("Search string \"%s\" too short or long",
                      object_to_search_for);
        return ERROR;
    }
    if (strcmp(object_to_search_for, "/") == 0)
    {
        /* root always exists */
        return IAS_L0R_OBJECT_FOUND;
    }

    /* Check for objects that exist in root as root is implied  in the format */
    if (object_to_search_for[0] == '/')
    {
        strcpy(name, &object_to_search_for[1]);
    }
    else
    {
        strncpy(name, object_to_search_for, sizeof(name));
    }

    /* make sure each of the groups/names exist */
    next_slash = name;
    do
    {
        /* find the next slash in the name */
        next_slash = strchr(next_slash, '/');
        if (next_slash)
            *next_slash = '\0';

        /* check if the current level exists */
        if (H5Lexists(file_id, name, H5P_DEFAULT) != TRUE)
        {
            /* doesn't exist, so return that it doesn't exist */
            return IAS_L0R_OBJECT_NOT_FOUND;
        }

        /* existed, so restore the slash for the next loop */
        if (next_slash)
        {
            *next_slash = '/';
            next_slash++;
        }

    } while (next_slash);

    /* object exists */
    return IAS_L0R_OBJECT_FOUND;
}

/******************************************************************************
 NAME: ias_l0r_hdf_table_records_count

 PURPOSE: Reads the size of the specified table

 RETURNS: SUCCESS - Size determined
          FAILURE - Size not determined

 NOTES:
******************************************************************************/
int ias_l0r_hdf_table_records_count
(
    const hid_t file_id, /* I: HDF file ID */
    const char *table, /* I: Table Name */
    int *size          /* O: Number of records in the table */
)
{
    herr_t hdf_status;
    hsize_t fields;
    hsize_t records;

    /* Set size to 0 in case of an error condition or table doesn't exist */
    *size = 0;

    if (file_id < 0)
    {
        IAS_LOG_ERROR("File ID provided is not valid");
        return ERROR;
    }
    if (table == NULL || strlen(table) == 0)
    {
        IAS_LOG_ERROR("Table name provided is NULL");
        return ERROR;
    }

    /* if the table doesn't exist there are no entries */
    if (ias_l0r_hdf_object_exists(file_id, table)  == IAS_L0R_OBJECT_NOT_FOUND)
    {
        return SUCCESS;
    }

    /* get table info */
    hdf_status = H5TBget_table_info(file_id, table, &fields, &records);
    if (hdf_status < 0)
    {
        IAS_LOG_ERROR("Error getting info for %s", table);
        return ERROR;
    }

    *size = (unsigned int)records;
    return SUCCESS;
}
