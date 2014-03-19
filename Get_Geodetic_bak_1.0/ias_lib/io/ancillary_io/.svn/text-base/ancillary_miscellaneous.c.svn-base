/**************************************************************************
 NAME:                     ancillary_miscellaneous

 PURPOSE:    Implements routines for the Ancillary IO library that don't
             quite fit into the other source modules

***************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>
#include <hdf5_hl.h>

#include "ias_const.h"
#include "ias_logging.h"
#include "ias_ancillary_io.h"
#include "ias_ancillary_io_private.h"


/*----------------------------------------------------------------------
 NAME:            ias_ancillary_cleanup_table_definition

 PURPOSE:  Loops through all datatype fields in an HDF5 ancillary data
           table, closing those that are open

 RETURNS:  Nothing

------------------------------------------------------------------------*/
void ias_ancillary_cleanup_table_definition
(
    hid_t fields_to_close[],             /* I: Array of open field
                                            datatypes */
    const int number_of_fields           /* I: Total number of fields
                                            in fields_to_close[] */ 
)
{
    int i;                               /* internal loop counter */

    /* "Closed" fields are represented by -1, "open" fields by a positive
       number > 0.  */
    for (i = 0; i < number_of_fields; i++)
    {
        if (fields_to_close[i] > 0)
        {
            H5Tclose(fields_to_close[i]);
            fields_to_close[i] = -1;
        }
    }
}



/*------------------------------------------------------------------------
 NAME:         ias_ancillary_is_ancillary_file

 PURPOSE:  Checks to make sure file is a 'valid' ancillary data file by
           checking for the ATTITUDE/EPHEMERIS_EPOCH_TIME and
           FILE_FORMAT_VERSION attributes.

 RETURNS:  Integer value of 0 if file is not valid, 1 if file is valid.
           These are represented by the #defined constants FALSE and TRUE,
           respectively.

--------------------------------------------------------------------------*/
int ias_ancillary_is_ancillary_file
(
    const char *ancillary_filename         /* I: Name of (potential)
                                                 ancillary file */
)
{
    hid_t hdf_file_id = -1;                /* Open HDF5 file */


    /* Do we even have a file name */
    if (strcmp(ancillary_filename, "") == 0)
    {
        IAS_LOG_DEBUG("No ancillary file name provided");
        return FALSE;
    }

    /* Open the file */
    hdf_file_id = H5Fopen(ancillary_filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (hdf_file_id < 0)
    {
        IAS_LOG_DEBUG("Cannot open file %s with HDF5 methods",
            ancillary_filename);
        return FALSE;
    }

    /* Look for the attitude epoch time data */
    if (!H5LTfind_attribute(hdf_file_id,
            ATTITUDE_EPOCH_TIME_ATTRIBUTE_NAME))
    {
        IAS_LOG_DEBUG("Cannot find attitude epoch time attribute in %s",
            ancillary_filename);
        H5Fclose(hdf_file_id);
        return FALSE;
    }

    /* Look for the ephemeris epoch time data */
    if (!H5LTfind_attribute(hdf_file_id,
            EPHEMERIS_EPOCH_TIME_ATTRIBUTE_NAME))
    {
        IAS_LOG_DEBUG("Cannot find ephemeris epoch time attribute in %s",
            ancillary_filename);
        H5Fclose(hdf_file_id);
        return FALSE;
    }

    /* On the surface at least, the ancillary file appears to be "valid".
       Close the file and return a TRUE status indicating this */
    H5Fclose(hdf_file_id);
    return TRUE;
}




