/******************************************************************************
NAME:        ias_gcp_write_correlation_results

PURPOSE:     Write all GCP records to the specified file

RETURN VALUE: 
Type = int
Value        Description
-----        -----------
SUCCESS      Successfully wrote out GCP data
ERROR        Error occurred; GCP data not added to file

NOTES:       This function writes out a set of GCP data records.
             If the file exists it will be overwritten.

******************************************************************************/
#include <string.h>
#include "ias_const.h"
#include "ias_miscellaneous.h"
#include "ias_logging.h"
#include "ias_gcp.h"


int ias_gcp_write_correlation_results 
(
    const char *gcp_data_filename,    /* I: Name of GCP data file to add to */
    const IAS_GCP_RESULTS gcp_data[], /* I: GCP data structures */
    int num_gcp                       /* I: Number of GCPs */
)
{
    FILE *gcp_fptr;                   /* GCP data file pointer */
    int status;                       /* Function return status */
    int curpt;                        /* Current GCP */
    const IAS_GCP_RESULTS *gcp_data_ptr;   /* Pointer to GCP data structure */
    char tmp_reference_name[PATH_MAX];/* Temp string for optional ref field */
    char tmp_search_name[PATH_MAX];  /* Temp string for optional search field */
    const char *data_type;            /* GCP data type */
    gcp_data_ptr = NULL;

    /* Open the file */
    gcp_fptr = fopen(gcp_data_filename, "w");
    if (gcp_fptr == NULL)
    {
        IAS_LOG_ERROR("Opening gcp data file %s", gcp_data_filename);
        return ERROR;
    }

    /* Write all records */
    for (curpt = 0; curpt < num_gcp; curpt++)
    {
        gcp_data_ptr = &gcp_data[curpt];

        /* Write a string representation of the data type to the file */
        status = ias_misc_convert_data_type_to_string(
                gcp_data_ptr->chip_data_type, &data_type);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Getting string for GCP data type %d for point %d", 
                    gcp_data_ptr->chip_data_type, curpt);
            fclose(gcp_fptr);
            return ERROR;
        }

        /* Make sure we print a space if the optional field has no value */
        if (strcmp(gcp_data_ptr->reference_name, "") == 0)
        {
            strcpy(tmp_reference_name, " ");
        }
        else
        {
            strcpy(tmp_reference_name, gcp_data_ptr->reference_name);
        }

        /* Make sure we print a space if the optional field has no value */
        if (strcmp(gcp_data_ptr->search_name, "") == 0)
        {
            strcpy(tmp_search_name, " ");
        }
        else
        {
            strcpy(tmp_search_name, gcp_data_ptr->search_name);
        }

        /* Write the line's information */
        status = fprintf(gcp_fptr,
                "%s,%lf,%lf,%.12lf,%.12lf,%lf,%lf,%lf,%.4lf,%.4lf,%d,%.3lf,%d,"
                "%d,%s,%d,%d,%s,%s,%s\n",
                gcp_data_ptr->point_id,
                gcp_data_ptr->reference_line,
                gcp_data_ptr->reference_sample,
                gcp_data_ptr->latitude,
                gcp_data_ptr->longitude,
                gcp_data_ptr->elevation,
                gcp_data_ptr->predicted_search_line,
                gcp_data_ptr->predicted_search_sample,
                gcp_data_ptr->fit_line_offset,
                gcp_data_ptr->fit_sample_offset,
                gcp_data_ptr->accept_flag,
                gcp_data_ptr->correlation_coefficient,
                gcp_data_ptr->search_band,
                gcp_data_ptr->search_sca,
                gcp_data_ptr->chip_source,
                gcp_data_ptr->reference_band,
                gcp_data_ptr->reference_sca,
                data_type,
                tmp_reference_name,
                tmp_search_name);

        if (status < 0)
        {
            IAS_LOG_ERROR("Writing to gcp data file %s", 
                    gcp_data_filename);
            fclose(gcp_fptr);
            return ERROR;
        }
    }

    /* Close the file */
    status = fclose(gcp_fptr);
    if (status != 0)
    {
        IAS_LOG_ERROR("Closing gcp data file %s", gcp_data_filename);
        return ERROR;
    }

    return SUCCESS;
}

