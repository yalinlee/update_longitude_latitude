/******************************************************************************
NAME:           ias_gcp_read_correlation_results

PURPOSE:        Get all records from the GCP data file and convert the 
                lat/long/height coordinate to Cartesian coordinates.

RETURN VALUE: 
Type = int
Value           Description
-----           -----------
SUCCESS         Successfully retrieved all GCP data 
ERROR           Error occurred; no valid GCP data returned

NOTES:          This function allocates memory for all the GCPs read from the
                file.  The calling function is responsible for freeing this
                memory.

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "ias_const.h"
#include "ias_miscellaneous.h"
#include "ias_math.h"
#include "ias_logging.h"
#include "ias_geo.h"
#include "ias_gcp.h"

#define RESULTS_SIZE 300   /* Max length of a GCP results line */
#define INITIAL_RECORDS 50 /* Number of records to initially allocate memory 
                              for */

int ias_gcp_read_correlation_results 
(
    const char *gcp_data_filename,  /* I: GCP data file name */
    const double semi_major_axis,   /* I: CPF earth semi major axis constant */
    const double ellipticity,       /* I: CPF earth ellipticity constant */
    IAS_GCP_RESULTS **gcp_data,     /* O: GCP data records */
    int *num_gcp                    /* O: Number of GCPs read */
)
{
    IAS_GCP_RESULTS gcp_tmp;         /* Temporary GCP data record */
    IAS_GCP_RESULTS *gcp_tmp_ptr;    /* Temporary GCP data pointer */
    FILE *gcp_file_ptr;              /* GCP data file pointer */
    char dataline[RESULTS_SIZE];     /* Line from GCP data file */
    char *string_pointer;            /* Pointer for thread-safe strtok_r */
    char *token;                     /* Pointer to token string used
                                        by strtok_r */
    double radians_per_degree;       /* Calculated value of radians/degree */
    double longitude_radians;        /* Longitude in radians */
    double latitude_radians;         /* Latitude in radians */
    int status;                      /* sscanf return status */
    int current_gcp;                 /* Current gcp structure array element */
    int realloc_multiplier;          /* Total for how many times realloc 
                                        doubled allocated memory */
    int line_count = 0;              /* Current line being processed */
    int parse_error_count = 0;       /* Parsing error counter */
    int dtype;                       /* GCP data type */

    radians_per_degree = ias_math_get_radians_per_degree();
    /* Initialize the number of GCPs read and the array of structures */
    *num_gcp = 0;
    current_gcp = 0;
    realloc_multiplier = 1;

    /* Allocate INITIAL_RECORDS records worth of memory to start with */
    *gcp_data = malloc(sizeof(**gcp_data) * INITIAL_RECORDS);
    if (*gcp_data == NULL)
    {
        IAS_LOG_ERROR("Allocating initial memory");
        return ERROR;
    }

    /* Open the file */
    gcp_file_ptr = fopen(gcp_data_filename, "r");
    if (gcp_file_ptr == NULL)
    {
        IAS_LOG_ERROR("Opening GCP file %s", gcp_data_filename);
        free(*gcp_data);
        *gcp_data = NULL;
        return ERROR;
    }

    /* Read until NULL is returned by fgets */
    while (fgets(dataline, RESULTS_SIZE, gcp_file_ptr) != NULL)
    {
        if (strlen(dataline) == 0)
            break;
        line_count++;
        memset(&gcp_tmp, 0, sizeof(gcp_tmp));

        /* Replace newline character with NULL */
        int dataline_length = strlen(dataline);
        if (dataline[dataline_length - 1] == '\n') 
        {
            dataline[dataline_length - 1] = '\0';
        }

        /* Verify there are no empty fields (two commas together)
           because strtok doesn't handle that as desired */
        token = strstr(dataline, ",,");
        if (token != NULL)
        {
            IAS_LOG_ERROR("A field value is missing");
            parse_error_count++;
        }

        token = strtok_r(dataline, ",", &string_pointer);
        if (token == NULL)
        {
            parse_error_count++;
        }
        else
        {
            status = snprintf(gcp_tmp.point_id, sizeof(gcp_tmp.point_id), "%s",
                        token);
            if (status < 0 || status >= sizeof(gcp_tmp.point_id))
            {
                parse_error_count++;
            }
        }
        token = strtok_r(NULL, ",", &string_pointer);
        if (token == NULL)
        {
            parse_error_count++;
        }
        else
        {
            status = sscanf(token, "%lf", &gcp_tmp.reference_line);
            if (status != 1) /* expected to get a value */
            {
                parse_error_count++;
            }
        }
        token = strtok_r(NULL, ",", &string_pointer);
        if (token == NULL)
        {
            parse_error_count++;
        }
        else
        {
            status = sscanf(token, "%lf", &gcp_tmp.reference_sample);
            if (status != 1) /* expected to get a value */
            {
                parse_error_count++;
            }
        }
        token = strtok_r(NULL, ",", &string_pointer);
        if (token == NULL)
        {
            parse_error_count++;
        }
        else
        {
            status = sscanf(token, "%lf", &gcp_tmp.latitude);
            if (status != 1) /* expected to get a value */
            {
                parse_error_count++;
            }
        }
        token = strtok_r(NULL, ",", &string_pointer);
        if (token == NULL)
        {
            parse_error_count++;
        }
        else
        {
            status = sscanf(token, "%lf", &gcp_tmp.longitude);
            if (status != 1) /* expected to get a value */
            {
                parse_error_count++;
            }
        }
        token = strtok_r(NULL, ",", &string_pointer);
        if (token == NULL)
        {
            parse_error_count++;
        }
        else
        {
            status = sscanf(token, "%lf", &gcp_tmp.elevation);
            if (status != 1) /* expected to get a value */
            {
                parse_error_count++;
            }
        }
        token = strtok_r(NULL, ",", &string_pointer);
        if (token == NULL)
        {
            parse_error_count++;
        }
        else
        {
            status = sscanf(token, "%lf", &gcp_tmp.predicted_search_line);
            if (status != 1) /* expected to get a value */
            {
                parse_error_count++;
            }
        }
        token = strtok_r(NULL, ",", &string_pointer);
        if (token == NULL)
        {
            parse_error_count++;
        }
        else
        {
            status = sscanf(token, "%lf", &gcp_tmp.predicted_search_sample);
            if (status != 1) /* expected to get a value */
            {
                parse_error_count++;
            }
        }
        token = strtok_r(NULL, ",", &string_pointer);
        if (token == NULL)
        {
            parse_error_count++;
        }
        else
        {
            status = sscanf(token, "%lf", &gcp_tmp.fit_line_offset);
            if (status != 1) /* expected to get a value */
            {
                parse_error_count++;
            }
        }
        token = strtok_r(NULL, ",", &string_pointer);
        if (token == NULL)
        {
            parse_error_count++;
        }
        else
        {
            status = sscanf(token, "%lf", &gcp_tmp.fit_sample_offset);
            if (status != 1) /* expected to get a value */
            {
                parse_error_count++;
            }
        }
        token = strtok_r(NULL, ",", &string_pointer);
        if (token == NULL)
        {
            parse_error_count++;
        }
        else
        {
            status = sscanf(token, "%d", &gcp_tmp.accept_flag);
            if (status != 1) /* expected to get a value */
            {
                parse_error_count++;
            }
        }
        token = strtok_r(NULL, ",", &string_pointer);
        if (token == NULL)
        {
            parse_error_count++;
        }
        else
        {
            status = sscanf(token, "%lf", &gcp_tmp.correlation_coefficient);
            if (status != 1) /* expected to get a value */
            {
                parse_error_count++;
            }
        }
        token = strtok_r(NULL, ",", &string_pointer);
        if (token == NULL)
        {
            parse_error_count++;
        }
        else
        {
            status = sscanf(token, "%d", &gcp_tmp.search_band);
            if (status != 1) /* expected to get a value */
            {
                parse_error_count++;
            }
        }
        token = strtok_r(NULL, ",", &string_pointer);
        if (token == NULL)
        {
            parse_error_count++;
        }
        else
        {
            status = sscanf(token, "%d", &gcp_tmp.search_sca);
            if (status != 1) /* expected to get a value */
            {
                parse_error_count++;
            }
        }
        token = strtok_r(NULL, ",", &string_pointer);
        if (token == NULL)
        {
            parse_error_count++;
        }
        else
        {
            status = snprintf(gcp_tmp.chip_source, sizeof(gcp_tmp.chip_source),
                    "%s", token);
            if (status < 0 || status >= sizeof(gcp_tmp.chip_source))
            {
                parse_error_count++;
            }
        }
        token = strtok_r(NULL, ",", &string_pointer);
        if (token == NULL)
        {
            parse_error_count++;
        }
        else
        {
            status = sscanf(token, "%d", &gcp_tmp.reference_band);
            if (status != 1) /* expected to get a value */
            {
                parse_error_count++;
            }
        }
        token = strtok_r(NULL, ",", &string_pointer);
        if (token == NULL)
        {
            parse_error_count++;
        }
        else
        {
            status = sscanf(token, "%d", &gcp_tmp.reference_sca);
            if (status != 1) /* expected to get a value */
            {
                parse_error_count++;
            }
        }
        token = strtok_r(NULL, ",", &string_pointer);
        if (token == NULL)
        {
            parse_error_count++;
        }
        else
        {
            /* Save data type in GCP record as IAS_DATA_TYPE enum type */
            status = ias_misc_convert_string_to_data_type(token, &dtype);
            if (status != SUCCESS)
            {
                IAS_LOG_ERROR("Getting GCP data type for %s on line %d "
                        "of %s", token, line_count, gcp_data_filename);
                parse_error_count++;
            }
            else
            {
                gcp_tmp.chip_data_type = dtype;
            }
        }
        /* It's okay for this field to be empty */
        token = strtok_r(NULL, ",", &string_pointer);
        if (token != NULL)
        {
            sscanf(token, "%s", gcp_tmp.reference_name);
        }
        else
        {
            strcpy(gcp_tmp.reference_name, "");
        }
        /* It's okay for this field to be empty */
        token = strtok_r(NULL, ",", &string_pointer);
        if (token != NULL)
        {
            sscanf(token, "%s", gcp_tmp.search_name);
        }
        else
        {
            strcpy(gcp_tmp.search_name, "");
        }

        if (parse_error_count != 0)
        {
            IAS_LOG_ERROR("Reading GCP data from line %d of %s", line_count,
                    gcp_data_filename);
            /* Close file and free data */
            fclose(gcp_file_ptr);
            free(*gcp_data);
            *gcp_data = NULL;
            return ERROR;
        }

        /* Calculate the Cartesian coordinate for the point */
        longitude_radians = gcp_tmp.longitude * radians_per_degree;
        latitude_radians = gcp_tmp.latitude * radians_per_degree;
        ias_geo_convert_geod2cart(latitude_radians,longitude_radians,
                gcp_tmp.elevation, semi_major_axis, ellipticity,
                &gcp_tmp.gcp_position);

        /* Check if outlier */
        if (gcp_tmp.accept_flag == TRUE)
        {
            (*num_gcp)++;
            /* Check to see if gcp_data has space for another record */
            if (*num_gcp > INITIAL_RECORDS * realloc_multiplier)
            {
                realloc_multiplier = realloc_multiplier * 2;
                /* If not, double gcp_data */
                gcp_tmp_ptr = realloc(*gcp_data, sizeof(**gcp_data) 
                        * INITIAL_RECORDS * realloc_multiplier);
                if (gcp_tmp_ptr == NULL)
                {
                    IAS_LOG_ERROR("Allocating memory for GCP %d", *num_gcp);
                    fclose(gcp_file_ptr);
                    free(*gcp_data);
                    *gcp_data = NULL;
                    *num_gcp = 0;
                    return ERROR;
                }
                *gcp_data = gcp_tmp_ptr;
            }

            memcpy(&(*gcp_data)[current_gcp], &gcp_tmp, sizeof(gcp_tmp));
            current_gcp++;
        }
    }

    /* No GCPs were found. Free up any memory that was allocated and return */
    if (*num_gcp == 0)
    {
        free(*gcp_data);
        *gcp_data = NULL;
        fclose(gcp_file_ptr);
        return SUCCESS;
    }

    /* Realloc memory to correct size */
    gcp_tmp_ptr = realloc(*gcp_data, *num_gcp * sizeof(**gcp_data));
    if (gcp_tmp_ptr == NULL)
    {
        IAS_LOG_ERROR("Allocating memory for GCP %d", *num_gcp);
        fclose(gcp_file_ptr);
        free(*gcp_data);
        *gcp_data = NULL;
        free(gcp_tmp_ptr);
        *num_gcp = 0;
        return ERROR;
    }
    *gcp_data = gcp_tmp_ptr;

    /* Close the file */
    status = fclose(gcp_file_ptr);
    if (status != 0)
    {
        IAS_LOG_ERROR("Closing GCP results file %s", gcp_data_filename);
        free(*gcp_data);
        *gcp_data = NULL;
        *num_gcp = 0;
        return ERROR;
    }

    return SUCCESS;
}
