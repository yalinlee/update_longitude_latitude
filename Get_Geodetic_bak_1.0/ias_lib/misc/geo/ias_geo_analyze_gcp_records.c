/*****************************************************************************
NAME: ias_geo_analyze_gcp_records

PURPOSE: 
    Calculates the mean, RMS error, and standard deviation of the  
    along- and across-track GCP residuals as well as the correlation
    coefficient between the across- and along-track residuals.

RETURN VALUE:
    Type = int
    Value    Description
    -----    -----------
    SUCCESS  statistics were calculated 
    ERROR    error generating statistics

*******************************************************************************/
#include "ias_logging.h"        /* ias_logging prototype */
#include "ias_const.h"          /* SUCCESS/ERROR values */
#include "ias_math.h"           /* statistics function prototypes */
#include "ias_geo.h"            /* statistics function prototypes */

static int calc_corr_coeff
(
    const double *x_sample,     /* I: pointer to the x data */
    const double *y_sample,     /* I: pointer to the y data */
    int  nsamps,                /* I: number of samples in x and in y */
    double x_mean,              /* I: mean of the x data */
    double y_mean,              /* I: mean of the y data */
    double x_std_dev,           /* I: standard deviation of the x data */
    double y_std_dev,           /* I: standard deviation of the y data */
    double *corr_coeff          /* O: correlation coefficient between 
                                      the x and y data */
);


int ias_geo_analyze_gcp_records 
(
    const double *GCP_lat,          /* I: residual data latitudes (degrees) */
    const double *GCP_lon,          /* I: residual data longitudes (degrees) */
    const double *along_track_res,  /* I: along-track residual data */
    const double *across_track_res, /* I: across-track residual data */
    int  num_valid,                 /* I: number of valid GCP residuals */
    GCP_RES_STATS *res_stats        /* O: GCP residual statistics */
)
{
    int status;                     /* return status of functions */

    /* Calculate the mean of the GCP residuals latitude and longitude. */
    status = ias_math_compute_mean(GCP_lat, num_valid, &res_stats->mean_lat);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Calculating the mean latitude");
        return ERROR;
    }

    status = ias_math_compute_mean(GCP_lon, num_valid, &res_stats->mean_lon);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Calculating the mean longitude");
        return ERROR;
    }

    /* Calculate the mean of the along- and across-track residuals. */
    status = ias_math_compute_mean(along_track_res, num_valid, 
        &res_stats->mean_along_res);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Calculating mean of along-track residuals");
        return ERROR;
    }

    status = ias_math_compute_mean(across_track_res, num_valid, 
        &res_stats->mean_across_res);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Calculating mean of across-track residuals");
        return ERROR;
    }

    /* Calculate the RMS error of the along- and across-track residuals. */
    status = ias_math_compute_rmse(along_track_res, num_valid, 
        &res_stats->rmse_along_res);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Calculating the along-track RMSE");
        return ERROR;
    }

    status = ias_math_compute_rmse(across_track_res, num_valid, 
        &res_stats->rmse_across_res);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Calculating the across-track RMSE");
        return ERROR;
    }

    /* Calculate the standard deviation of the along- and across-track
       residuals. */
    status = ias_math_compute_stdev(along_track_res, num_valid, 
        res_stats->mean_along_res, &res_stats->sd_along_res);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Calculating the along-track standard deviation");
        return ERROR;
    }
    status = ias_math_compute_stdev(across_track_res, num_valid, 
        res_stats->mean_across_res, &res_stats->sd_across_res);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Calculating the across-track standard deviation");
        return ERROR;
    }

    /* Calculate the correlation coefficient between the along- and 
      across-track residuals. */
    status = calc_corr_coeff(along_track_res, across_track_res, num_valid,
        res_stats->mean_along_res, 
        res_stats->mean_across_res, res_stats->sd_along_res,
        res_stats->sd_across_res, &res_stats->corr_coeff);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Calculating the across vs along-track correlation "
            "coefficient");
        return ERROR;
    }

    res_stats->num_points = num_valid;

    return SUCCESS;
}



/*****************************************************************************
NAME: calc_corr_coeff

PURPOSE: Calculates the correlation coefficient between two data groups.


RETURN VALUE:
Type = int
Value    Description
-----    -----------
SUCCESS  correlation coefficient was calculated.
ERROR    Sample size or standard deviation was zero.

*******************************************************************************/
static int calc_corr_coeff
(
    const double *x_sample,     /* I: pointer to the x data */
    const double *y_sample,     /* I: pointer to the y data */
    int  nsamps,                /* I: number of samples in x and in y */
    double x_mean,              /* I: mean of the x data */
    double y_mean,              /* I: mean of the y data */
    double x_std_dev,           /* I: standard deviation of the x data */
    double y_std_dev,           /* I: standard deviation of the y data */
    double *corr_coeff          /* O: correlation coefficient between 
                                      the x and y data */
)
{
    int  i; /* loop variable */
    double xy_dev_sum = 0.0; /* product of x and y deviation from means */
    double covariance; /* covariance between x and y */

    /* Calculate the covariance and then the correlation coefficient 
    between the two data groups.  Return ERROR if calculations will
    result in divide by zero or value is invalid.  */
    if (nsamps <= 1 || x_std_dev <= 0.0 || y_std_dev <= 0.0)
    {
        IAS_LOG_ERROR("Sample size and/or std. deviation is invalid");
        *corr_coeff = 0.0;
        return ERROR;
    }

    for (i = 0; i < nsamps; i++)
        xy_dev_sum += (x_sample[i] - x_mean) * (y_sample[i] - y_mean);

    covariance = xy_dev_sum / (nsamps-1);

    *corr_coeff = covariance / (x_std_dev * y_std_dev);

    return SUCCESS;
}
