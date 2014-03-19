/**********************************************************************
Name: ias_ancillary_extract_valid_imu_data_window

Purpose: Window the IMU data so that the IMU falls inside the ephemeris data.

Returns: Integer status code
         SUCCESS:  Successful completion 
         ERROR:    Operation failed
***********************************************************************/

#include "ias_const.h"
#include "ias_logging.h"
#include "ias_structures.h"
#include "ias_math.h"
#include "ias_ancillary_io.h"
#include "ias_ancillary.h"
#include "ias_ancillary_private.h"

int ias_ancillary_extract_valid_imu_data_window
(
    const IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data,
                                     /* I: Pointer to ephem data */
    double *imu_time_data,           /* I/O: Array of IMU seconds since j2000*/
    IAS_VECTOR *imu_data,            /* I/O: Array of IMU records */
    int *valid_imu_flag,             /* I/O: Array of IMU quality flags */
    int *imu_count,                  /* I/O: Number of IMU records (size of
                                             IMU data arrays) */
    double *imu_epoch,               /* O: Year, DOY, SOD for start of IMU */
    double *eph_start_time,          /* O: Ephemeris start time */
    double *eph_stop_time,           /* O: Ephemeris stop time */
    int *invalid_imu_count           /* O: Number of invalid IMU samples */
)
{
    int start_index;     /* starting index for subsetting arrays */
    int subset_index;    /* index for subsetting arrays */
    int index;           /* loop counter */
    int status;          /* return value */
    int imu_index;       /* used to calculate the start index of data */

    /* local copies of output parameters */
    int imu_total;       /* local copy of imu_count, number of IMU records */

    imu_total = *imu_count;

    /* First step is to retrieve the ephemeris start/stop times */
    status = ias_math_convert_year_doy_sod_to_j2000_seconds(
        anc_ephemeris_data->utc_epoch_time, eph_start_time);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Converting J2000 seconds %lf to Year, DOY, SOD format",
                anc_ephemeris_data->utc_epoch_time[0]);
        return ERROR;
    }

    *eph_stop_time = anc_ephemeris_data->records[anc_ephemeris_data->
        number_of_samples - 1].seconds_from_epoch + *eph_start_time;

    IAS_LOG_INFO("Ephemeris times --- start %f, end %f -> center %f",
        *eph_start_time, *eph_stop_time,
        *eph_start_time + (*eph_stop_time - *eph_start_time) / 2.0);

    /* compute IMU start index */
    imu_index = 0;
    while (imu_time_data[imu_index] < (*eph_start_time) 
             && imu_index < imu_total - 1) 
        imu_index++;
    start_index = imu_index;

    /* Index into IMU data */
    IAS_LOG_DEBUG("IMU start index %d", start_index);

    /* compute IMU stop index */
    imu_index = imu_total - 1;
    while (imu_time_data[imu_index] > (*eph_stop_time) &&
           imu_index > 0) 
        imu_index--;

    /* Verify a start time was found in the IMU data */
    if (start_index < 0 || start_index >= imu_total || start_index >= imu_index)
    {
        IAS_LOG_ERROR("Finding start time in IMU data");
        return ERROR;
    }
    imu_total = imu_index - start_index + 1;

    IAS_LOG_DEBUG("Index into IMU: %d start_index: %d imu_total: %d "
                  "imu_time_data start: %f, imu_time_data end: %f",
                  imu_index, start_index, imu_total,
                  imu_time_data[start_index],
                  imu_time_data[imu_index]);

    status = ias_math_convert_j2000_seconds_to_year_doy_sod(
        imu_time_data[start_index], imu_epoch);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Converting J2000 seconds %lf to Year, DOY, SOD format",
                imu_time_data[start_index]);
        return ERROR;
    }

    subset_index = start_index;
    /* Subset IMU data to match the ephemeris data. */
    *invalid_imu_count = 0;
    for (index = 0; index < imu_total; index++, subset_index++)
    {
        imu_data[index].x = imu_data[subset_index].x;
        imu_data[index].y = imu_data[subset_index].y;
        imu_data[index].z = imu_data[subset_index].z;
        imu_time_data[index] =
            imu_time_data[subset_index];
        valid_imu_flag[index]  = valid_imu_flag[subset_index];
        if (valid_imu_flag[index] == 0) 
            (*invalid_imu_count)++;
    }

#if DEBUG_GENERATE_DATA_FILES == 1 
    FILE *fp = fopen("anc.imu.win.dat","w");

    for (index = 0; index < imu_total; index++)
    {
        fprintf(fp, "%d %d %f %e %e %e\n", index, imu_index,
            imu_time_data[index], imu_data[index].x, 
            imu_data[index].y, imu_data[index].z);
    }

    fclose(fp);
#endif

    /* assign local results back to output variable */
    *imu_count = imu_total;

    return SUCCESS;
}
