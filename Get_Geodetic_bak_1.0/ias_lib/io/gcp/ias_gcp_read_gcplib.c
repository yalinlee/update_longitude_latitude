/******************************************************************************

ROUTINES:       ias_gcp_read_gcplib
                ias_gcp_read_gcplib_filtered

******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_miscellaneous.h"
#include "ias_gcp.h"

#define GCP_CHIPREC 20  /* Number of chip records (fields) to read per line */
#define GCP_REC_SIZE 300  /* Max length of a GCPLib record */

/******************************************************************************
NAME:        ias_gcp_read_gcplib

PURPOSE:
Read the GCPLib file.  Put the Ground Control Points information
into the GCPLib structure.  This is the no-filters version that
retrieves all records from the GCPLib file.

RETURN VALUE:
type=int
Value        Description
-----        -----------
SUCCESS      Successfully retrieved the GCP information.
ERROR        Failure to get the GCP information.

******************************************************************************/
int ias_gcp_read_gcplib
(
    const char *gcplib_file_name, /* I: Name of the GCPLIB file */
    IAS_GCP_RECORD **gcp_lib,     /* O: Structure of chip information */
    int *num_gcp                  /* O: Number of ground control points */
)
{
    char season[IAS_GCP_NUM_SEASONS][IAS_GCP_SEASON_LEN];
    char chip_source[IAS_GCP_NUM_CHIP_SOURCES][IAS_GCP_SOURCE_SIZE];
    char chip_type[IAS_GCP_TYPE_SIZE];
    const int begin_date[] = {0, 0};
    const int end_date[] = {0, 0};
    int status;
    int i;

    /* This routine uses the underlying filtering routine to do the guts of
       reading the GCPLib file. Setup the required filter parameters, but
       define them to provide no filters so the entire set of GCP records in
       the file will be returned to the caller. */
    for (i = 0; i < IAS_GCP_NUM_SEASONS; i++)
        season[i][0] = '\0';
    for (i = 0; i < IAS_GCP_NUM_CHIP_SOURCES; i++)
        chip_source[i][0] = '\0';
    chip_type[0] = '\0';

    /* Get the GCP records */
    status = ias_gcp_read_gcplib_filtered(gcplib_file_name, begin_date,
            end_date, season, chip_source, chip_type, gcp_lib, num_gcp);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Retrieving the GCP records from the GCPLib with "
                "underlying ias_gcp_read_gcplib_filtered routine");
        return ERROR;
    }

    return SUCCESS;
}

/******************************************************************************
NAME:        ias_gcp_read_gcplib_filtered

PURPOSE:
Read the GCPLib file.  Put the Ground Control Points information
into the GCPLib structure.  Allows providing GCP filtering inputs.

RETURN VALUE:
type=int
Value        Description
-----        -----------
SUCCESS      Successfully retrieved the GCP information.
ERROR        Failure to get the GCP information.

******************************************************************************/
int ias_gcp_read_gcplib_filtered
(
    const char *gcplib_file_name, /* I: Name of the GCPLIB file */
    /* For dates, 1 = January, etc. Year is YYYY */
    const int *begin_date,        /* I: Beginning date [0] = month [1] = year */
    const int *end_date,          /* I: Ending date [0] = month [1] = year */
                                  /* I: Season of chip */
    char season[IAS_GCP_NUM_SEASONS][IAS_GCP_SEASON_LEN],
                                  /* I: Source of chip */
    char chip_source[IAS_GCP_NUM_CHIP_SOURCES][IAS_GCP_SOURCE_SIZE],
    const char *chip_type,        /* I: Type of chip */
    IAS_GCP_RECORD **gcp_lib,     /* O: Structure of chip information */
    int *num_gcp                  /* O: Number of ground control points */
)
{
    IAS_GCP_RECORD tmp_lib;     /* Temporary GCPLib info */
    IAS_GCP_RECORD *allocated_ptr; /* Pointer to allocate memory */
    FILE *gcplib_file_ptr;      /* Pointer to the GCPLib file */
    char *line_ptr;             /* Pointer to line of GCPLib file from fgets */
    char buffer[GCP_REC_SIZE];  /* String read in from file */
    char data_type[IAS_GCP_DATA_TYPE_MAX_SIZE]; /* GCP data type */
    int allocated_count;        /* Number of gcp_lib buffers allocated */
    int count_used = 0;         /* Count of chips used */
    int count_read = 0;         /* Count of chips read */
    int count_in_file;          /* Count of chips expected in the file */
    int day;                    /* Satellite day of the month */
    int month;                  /* Satellite month */
    int year;                   /* Satellite year */
    int status;                 /* Status of return from function */
    int i,j;                    /* Loop control variables */
    int season_flag = FALSE;    /* Flag indicating if seasons are specified */
    int source_flag = FALSE;    /* Flag indicating if sources are specified */
    int type_flag = FALSE;      /* Flag indicating if types are specified */
    int source_found;           /* Temporary flag in source search */
    int num_source = 0;         /* Number of chip source entered */
    int quarter[4];             /* Specific quarters */
    int use_point = TRUE;       /* Flag indicating if point should be used */

    /* Initialize the returned count read */
    *num_gcp = 0;

    /* Initialize the structure to NULL */
    *gcp_lib = NULL;

    /* Open the GCPLib file */
    gcplib_file_ptr = fopen(gcplib_file_name, "r");
    if (gcplib_file_ptr == NULL)
    {
        IAS_LOG_ERROR("Getting GCP library file");
        return ERROR;
    }

    /* This routine can be used to get the entire GCPLib file when no GCP
       filter parameters are defined. Using the filter parameters will check
       each GCP record read from the file against the filters given and gather
       all those that match the filters. Tracking the use_point status is only
       affected when filters are defined. They won't be defined when this
       routine is called by the ias_gcp_read_gcplib wrapper that doesn't
       allow providing filter definitions to it. */

    /* Initialize the quarters */
    for (i = 0; i < 4; i++)
        quarter[i] = FALSE;

    /* Check the source parameter to see if a check is necessary and get the
       number of sources */
    if (strcmp(chip_source[0], "") != 0)
    {
        source_flag = TRUE;
        while ((strcmp(chip_source[num_source], "") != 0) 
                && (num_source < IAS_GCP_NUM_CHIP_SOURCES))
        {
            num_source++;
        }
    }

    /* Check the GCP type parameter to see if a check is necessary */
    if (strcmp(chip_type, "") != 0)
        type_flag = TRUE;

    /* Read the file until end of file or BEGIN flag to bypass header */
    for (;;)
    {
        line_ptr = fgets(buffer, sizeof(buffer), gcplib_file_ptr);
        if (line_ptr == NULL) 
        {
            IAS_LOG_ERROR("Unexpected end of gcplib file");
            fclose(gcplib_file_ptr);
            return ERROR;
        }

        /* Skip the data starter mark "BEGIN" */
        if (strncmp(buffer, "BEGIN", 5) == 0)
        {
            /* Read the next line with total number of records */
            line_ptr = fgets(buffer, sizeof(buffer), gcplib_file_ptr);
            if (line_ptr == NULL)
            {
                IAS_LOG_ERROR("Reading total number of records");
                fclose(gcplib_file_ptr);
                return ERROR;
            }

            /* Get the count of GCPs expected in the file */
            count_in_file = atoi(buffer);
            break;
        }
    }

    /* Start without allocating a gcp_lib buffer */
    allocated_count = 0;

    /* Get the chip information from file */
    for (j = 0; j < count_in_file; j++)
    {
        use_point = TRUE;

        line_ptr = fgets(buffer, sizeof(buffer), gcplib_file_ptr);
        if (line_ptr == NULL)
        {
            IAS_LOG_ERROR("Reading GCPLib data: reading line %d",
                    count_read + 1);
            fclose(gcplib_file_ptr);
            free(*gcp_lib);
            *gcp_lib = NULL;
            return ERROR;
        }

        status = sscanf(line_ptr,
                "%s %s "
                "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf "
                "%s %s %s %d %s %s %s\n",
                tmp_lib.point_id,
                tmp_lib.chip_name,
                &tmp_lib.reference_line,
                &tmp_lib.reference_sample,
                &tmp_lib.latitude,
                &tmp_lib.longitude,
                &tmp_lib.projection_y,
                &tmp_lib.projection_x,
                &tmp_lib.elevation,
                &tmp_lib.pixel_size_x,
                &tmp_lib.pixel_size_y,
                &tmp_lib.chip_size_lines,
                &tmp_lib.chip_size_samples,
                tmp_lib.source,
                tmp_lib.chip_type,
                tmp_lib.projection,
                &tmp_lib.zone,
                tmp_lib.date,
                tmp_lib.absolute_or_relative,
                data_type);

        if (status == EOF) /* (unexpected) end of file or matching failure */
        {
            IAS_LOG_ERROR("Reading GCPLib data: unexpected end of file");
            fclose(gcplib_file_ptr);
            free(*gcp_lib);
            *gcp_lib = NULL;
            return ERROR;
        }

        /* Confirm matched GCP_CHIPREC values */
        if (status != GCP_CHIPREC)
        {
            IAS_LOG_ERROR("Reading GCPLib data: too few values");
            fclose(gcplib_file_ptr);
            free(*gcp_lib);
            *gcp_lib = NULL;
            return ERROR;
        }

        /* Convert the data type string to an IAS_DATA_TYPE for GCP record */
        status = ias_misc_convert_string_to_data_type(data_type,
                &tmp_lib.chip_data_type);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Getting GCP data type for %s on line %d of %s",
                    data_type, count_read + 1, gcplib_file_name);
            fclose(gcplib_file_ptr);
            free(*gcp_lib);
            *gcp_lib = NULL;
            return ERROR;
        }

        /* Check size of fields read in */
        status = strlen(tmp_lib.point_id);
        if (status >= IAS_GCP_ID_SIZE) 
        {
            IAS_LOG_ERROR("Variable 'point_id' is too long");
            fclose(gcplib_file_ptr);
            free(*gcp_lib);
            *gcp_lib = NULL;
            return ERROR;
        }

        status = strlen(tmp_lib.chip_name);
        if (status >= IAS_GCP_CHIP_NAME_SIZE) 
        {
            IAS_LOG_ERROR("Variable 'chip_name' is too long");
            fclose(gcplib_file_ptr);
            free(*gcp_lib);
            *gcp_lib = NULL;
            return ERROR;
        }

        status = strlen(tmp_lib.source);
        if (status >= IAS_GCP_SOURCE_SIZE) 
        {
            IAS_LOG_ERROR("Variable 'source' is too long");
            fclose(gcplib_file_ptr);
            free(*gcp_lib);
            *gcp_lib = NULL;
            return ERROR;
        }

        status = strlen(tmp_lib.chip_type);
        if (status >= IAS_GCP_TYPE_SIZE) 
        {
            IAS_LOG_ERROR("Variable 'chip_type' is too long");
            fclose(gcplib_file_ptr);
            free(*gcp_lib);
            *gcp_lib = NULL;
            return ERROR;
        }

        status = strlen(tmp_lib.projection);
        if (status >= IAS_GCP_PROJECTION_SIZE) 
        {
            IAS_LOG_ERROR("Variable 'projection' is too long");
            fclose(gcplib_file_ptr);
            free(*gcp_lib);
            *gcp_lib = NULL;
            return ERROR;
        }

        status = strlen(tmp_lib.date);
        if (status >= IAS_GCP_DATE_LEN) 
        {
            IAS_LOG_ERROR("Variable 'date' is too long");
            fclose(gcplib_file_ptr);
            free(*gcp_lib);
            *gcp_lib = NULL;
            return ERROR;
        }

        status = strlen(tmp_lib.absolute_or_relative);
        if (status >= IAS_GCP_ABS_REL_SIZE) 
        {
            IAS_LOG_ERROR("Variable 'absolute_or_relative' is too long");
            fclose(gcplib_file_ptr);
            free(*gcp_lib);
            *gcp_lib = NULL;
            return ERROR;
        }

        /* Check the date of the GCPLib */
        /* The format is MM-DD-YYYY */
        status = sscanf(tmp_lib.date, IAS_GCP_DATE_FORMAT, &month, &day, &year);
        if (status != 3)
        {
            IAS_LOG_ERROR("In GCPLib date \"%s\" failed date format check for "
                    "id %s", tmp_lib.date, tmp_lib.point_id);
            fclose(gcplib_file_ptr);
            free(*gcp_lib);
            *gcp_lib = NULL;
            return ERROR;
        }

        status = ias_misc_check_year_month_day(year, month, day);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("In GCPLib date \"%s\" failed date check for id %s",
                    tmp_lib.date, tmp_lib.point_id);
            fclose(gcplib_file_ptr);
            free(*gcp_lib);
            *gcp_lib = NULL;
            return ERROR;
        }

        /* Check the filters that may have been specified to reduce
           the set to only those wanted */

        /* Since the chips are grouped together, this only needs to be done
           once with the first chip */
        if (count_read == 0)
        {
            /* Get season input.  The season is dependent on the latitude 
               therefore a test of the latitude is included to determine 
               southern or northern hemisphere. */

            for (i = 0; i < IAS_GCP_NUM_SEASONS; i++)
            {
                if (strcmp(season[i], "") != 0)
                {
                    ias_misc_convert_to_uppercase(season[i]);
                    season_flag = TRUE;
                    if (strcmp(season[i], "SUMMER") == 0)
                    {
                        if (tmp_lib.latitude < 0)
                            quarter[0] = TRUE;
                        else
                            quarter[2] = TRUE;
                    }
                    else if (strcmp(season[i], "SPRING") == 0)
                    {
                        if (tmp_lib.latitude < 0)
                            quarter[3] = TRUE;
                        else
                            quarter[1] = TRUE;
                    }
                    else if (strcmp(season[i], "FALL") == 0)
                    {
                        if (tmp_lib.latitude < 0)
                            quarter[1] = TRUE;
                        else
                            quarter[3] = TRUE;
                    }
                    else if (strcmp(season[i], "WINTER") == 0)
                    {
                        if (tmp_lib.latitude < 0)
                            quarter[2] = TRUE;
                        else
                            quarter[0] = TRUE;
                    }
                    else
                    {
                        IAS_LOG_ERROR("Invalid season entered");
                        fclose(gcplib_file_ptr);
                        free(*gcp_lib);
                        *gcp_lib = NULL;
                        return(ERROR);
                    }
                }
            }
        }

        /* Test beginning and ending year and season */
        if ((begin_date[0] != 0) || (end_date[0] != 0)
                || (begin_date[1] != 0) || (end_date[1] != 0)
                || (season_flag))
        {
            if (month < begin_date[0])
                use_point = FALSE;
            else if ((month > end_date[0]) && (end_date[0] != 0))
                use_point = FALSE;
            else if (year < begin_date[1])
                use_point = FALSE;
            else if ((year > end_date[1]) && (end_date[1] != 0))
                use_point = FALSE;
            /* Test season */
            else if (season_flag)
            {
                if ((!quarter[0]) && (month <= 3))
                    use_point = FALSE;
                else if ((!quarter[1]) && (month >= 4) && (month <= 6))
                    use_point = FALSE;
                else if ((!quarter[2]) && (month >= 7) && (month <= 9))
                    use_point = FALSE;
                else if ((!quarter[3]) && (month >= 10))
                    use_point = FALSE;
            }
        }

        /* Test the source */
        if (source_flag)
        {
            source_found = FALSE;
            for (i = 0; i < num_source; i++)
            {
                if (strcasecmp(chip_source[i], tmp_lib.source) == 0)
                {
                    source_found = TRUE;
                    break;
                }
            }
            if (!source_found)
                use_point = FALSE;
        }

        /* Test the chip_type */
        if (type_flag)
        {
            if (strcasecmp(chip_type, tmp_lib.chip_type) != 0)
            {
                use_point = FALSE;
            }
        }

        /* If the point is used, save it to the gcp_lib array */
        if (use_point == TRUE)
        {
            /* If needed, allocate more memory for the GCPs being kept */
            if (count_used >= allocated_count)
            {
                if (allocated_count == 0)
                {
                    /* Start by allocating a buffer for 100 records */
                    allocated_count = 100;
                }
                else
                {
                    /* Otherwise, double the memory allocated */
                    allocated_count *= 2;
                }
                allocated_ptr = (IAS_GCP_RECORD *)realloc(*gcp_lib,
                        allocated_count * sizeof(IAS_GCP_RECORD));
                if (allocated_ptr == NULL)
                {
                    IAS_LOG_ERROR("Allocating memory");
                    fclose(gcplib_file_ptr);
                    free(*gcp_lib);
                    *gcp_lib = NULL;
                    return ERROR;
                }
                *gcp_lib = allocated_ptr;
            }

            /* Copy to final library structure */
            memcpy(&((*gcp_lib)[count_used]), &tmp_lib, sizeof(tmp_lib));

            /* Increment counter */
            count_used++;
        }

        count_read++;
    }

    fclose(gcplib_file_ptr);

    /* If any records were kept, trim the memory for the gcps to the actual
       number of GCPs used.  If the count_used is zero, no memory was
       allocated. */
    if (count_used > 0)
    {
        allocated_ptr = (IAS_GCP_RECORD *)realloc(*gcp_lib,
                count_used * sizeof(IAS_GCP_RECORD));
        if (allocated_ptr == NULL)
        {
            IAS_LOG_ERROR("Allocating memory");
            free(*gcp_lib);
            *gcp_lib = NULL;
            return ERROR;
        }
        *gcp_lib = allocated_ptr;
    }

    /* Make sure the total listed at top of GCPLib file matches GCPs counted */
    if (count_read != count_in_file)
    {
        IAS_LOG_ERROR("Count of GCPs read (%d) doesn't match the number "
                "expected (%d)", count_read, count_in_file);
        free(*gcp_lib);
        *gcp_lib = NULL;
        return ERROR;
    }

    /* Set the number of GCPs that passed the filtering */
    *num_gcp = count_used;

    return SUCCESS;
}
