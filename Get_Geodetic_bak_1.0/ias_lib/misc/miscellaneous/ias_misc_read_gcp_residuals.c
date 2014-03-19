/*-----------------------------------------------------------------------------
NAME:    ias_misc_read_gcp_residuals 

PURPOSE: Read the residual along and across scan components for each GCP
         from the residual file.

RETURN:  SUCCESS or ERROR
-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ias_logging.h"            /* Logging library */
#include "ias_const.h"              /* Return value definitions */
#include "ias_misc_gcp_residuals.h"

int ias_misc_read_gcp_residuals
(
    const char *residuals_filename,     /* I: Precision residual file name */
    int iteration_number,               /* I: The iteration number to get */
    IAS_MISC_GCP_RESIDUAL **gcp_res,    /* O: GCP residual information */
    int *number_of_residuals            /* O: # of residuals read from file */
)
{
    FILE *res_fptr;                 /* File pointer for residual file */
    int iteration;                  /* The iteration read from the file */
    int residuals_count = 0;        /* Count of the residuals read */
    int band_number = 0;            /* Band number read from file */
    int band_flag;                  /* Flag if a band number was read from the
                                       file: 1 band read, 0 band not read. */
    char keyword[GCP_RECLEN];       /* Key word when searching file */
    char buf1[GCP_RECLEN];          /* Buffer for 1st word when getting band */
    char buf2[GCP_RECLEN];          /* Buffer for 2nd word when getting band */
    char buffer[GCP_RECLEN];        /* Buffer for data */
    IAS_MISC_GCP_RESIDUAL tmp_res;  /* Temporary GCP residual info */
    int param_num;                  /* Number of parameters per line to read */
    int num_scanned;

    /* Initialize */
    *gcp_res = NULL;
    *number_of_residuals = 0;

    /* Open the file */
    res_fptr = fopen(residuals_filename, "r");
    if (!res_fptr)
    {
        IAS_LOG_ERROR("Opening residuals file: %s", residuals_filename);
        return ERROR;
    }

    /* Find the beginning of the specific group. Check if specific number or
       final iteration was specified. */
    band_flag = 0;
    for(;;)
    {
        if (fgets(buffer, GCP_RECLEN, res_fptr) == NULL)
        {
            IAS_LOG_ERROR("Reading from file: %s", residuals_filename);
            fclose(res_fptr);
            return ERROR;
        }

        /* Get the first word and see if it's a band */
        sscanf(buffer,"%s",keyword);
        if ((strcmp(keyword, "Band") == 0) && !band_flag)
        {
            sscanf(buffer, "%s%s%d", buf1, buf2, &band_number);
            band_flag = 1;
        }
        else if (!strcmp(keyword, "Iteration") || !strcmp (keyword, "Final"))
        {
            sscanf(buffer, "%s %d", keyword, &iteration);
            if (((strcmp(keyword,"Iteration") == 0)
                        && (iteration == iteration_number))
                    || ((strcmp(keyword,"Final") == 0)
                        && (iteration_number < 0)))
            {
                break;
            }
        }
    }

    if (!band_flag)
    {
        IAS_LOG_ERROR("Band number not found in: %s", residuals_filename);
        fclose(res_fptr);
        return ERROR;
    }

    /* Loop over all GCPs to read the residual information. Break out
       of the for loop when no data is found. */
    param_num = 14;
    for(;;)
    {
        /* If all parameters aren't read break out of the loop */
        num_scanned = fscanf(res_fptr,
                "%s%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%d%s\n",
                tmp_res.point_id, &tmp_res.predicted_line,
                &tmp_res.predicted_sample, &tmp_res.seconds_from_epoch,
                &tmp_res.latitude, &tmp_res.longitude, &tmp_res.height,
                &tmp_res.across_track_angle, &tmp_res.along_track_residual,
                &tmp_res.across_track_residual, &tmp_res.residual_y,
                &tmp_res.residual_x, &tmp_res.outlier_flag, tmp_res.gcp_source);
        if (num_scanned != param_num)
            break;
        else if (!strcmp (tmp_res.point_id, "Iteration")
                || !strcmp (tmp_res.point_id, "Final"))
        {
            break;
        }
        else
        {
            *gcp_res= (IAS_MISC_GCP_RESIDUAL *)realloc(*gcp_res,
                    (residuals_count + 1) * sizeof(IAS_MISC_GCP_RESIDUAL));
            if (*gcp_res == NULL)
            {
                IAS_LOG_ERROR("Allocating memory for %d residuals",
                        residuals_count + 1);
                free(*gcp_res);
                *gcp_res = NULL;
                fclose(res_fptr);
                return ERROR;
            }

            /* Assign the band number to the structure */
            tmp_res.band_number = band_number;

            /* Copy the temporary structure to the actual structure */
            memcpy((void *)&((*gcp_res)[residuals_count]), (void *)&tmp_res,
                    sizeof(tmp_res));
        }
        residuals_count++;
    }

    *number_of_residuals = residuals_count;

    if (fclose(res_fptr) != 0)
        IAS_LOG_WARNING("Closing residuals file: %s", residuals_filename);

    return SUCCESS;
}
