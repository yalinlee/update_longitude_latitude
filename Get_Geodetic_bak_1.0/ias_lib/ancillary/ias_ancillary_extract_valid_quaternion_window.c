/**********************************************************************
Name: ias_ancillary_extract_valid_quaternion_window

Purpose: 
    Window the quaternion data according to what was windowed for the IMU data.
    Subset the valid quaternion data to fit inside the ephemeris data.

Returns: Type = int
    Value    Description
    -----    -----------
    SUCCESS  Successful completion
    ERROR    Operation failed
***********************************************************************/

#include <math.h>
#include "ias_logging.h"
#include "ias_math.h"
#include "ias_ancillary.h"
#include "ias_ancillary_private.h"

/* tolerance for quaternion time checking */
#define QTIME_TOL 1.0e-4 

int ias_ancillary_extract_valid_quaternion_window
(
    const double *imu_time_data,    /* I: Array of IMU seconds since 
                                                 j2000 */
    int imu_count,                  /* I: Number of IMU records */
    IAS_QUATERNION *valid_quaternion_data, /* I/O: Array of valid quaternions */
    double *quaternion_time_data,   /* I/O: Array of quaternion times */
    int *valid_quaternion_flag,     /* I/O: Array of quaternion quality flags 
                                            which track valid/invalid data */
    int *valid_quaternion_count,    /* I/O: Valid quaternion totals (size of
                                            quaternion data arrays) */
    int *invalid_quaternion_count   /* O: Number of invalid quaternions */
)
{
    int scale;         /* scale in sampling steps for IMU and quaternion data */
    int count;         /* counter */
    int index;         /* counter */
    int quat_index;    /* loop index */
    int quat_start_index; /* starting index for subsetting arrays */
    int quat_end_index;   /* end index for valid quaternion data */
    int valid_quat_total; /* local variable for valid quaternion totals */

    valid_quat_total = *valid_quaternion_count;

    /* Window the quaternions. */
    scale = (int)(IAS_ANCILLARY_QUAT_TIME / IAS_ANCILLARY_IMU_TIME);

    IAS_LOG_DEBUG("Scale %d %f %f",
        scale, IAS_ANCILLARY_QUAT_TIME, IAS_ANCILLARY_IMU_TIME);

    /* Find the first valid quaternion after the first IMU point. */
    quat_start_index = 0;
    IAS_LOG_DEBUG("Search %f -> %f for a time %f",
                  quaternion_time_data[quat_start_index],
                  quaternion_time_data[valid_quat_total - 1],
                  imu_time_data[0]);

    while (quat_start_index < (valid_quat_total - 1))
    {
        if (imu_time_data[0] < quaternion_time_data[quat_start_index]
            &&
            fabs(quaternion_time_data[quat_start_index + 1]
                 - quaternion_time_data[quat_start_index]
                 - IAS_ANCILLARY_QUAT_TIME) < QTIME_TOL)
        {
            break;
        }

        quat_start_index++;
    }

    /* Make sure there really was enough quaternion data to cover the IMU
       data */
    if (quat_start_index == 0)
    {
        if (fabs(quaternion_time_data[0] - imu_time_data[0])
            > IAS_ANCILLARY_QUAT_TIME)
        {
            IAS_LOG_ERROR("Quaternion data does not cover the start of the "
                "IMU data.  IMU data starts at %f but quaternion data "
                "does not start until %f.", imu_time_data[0],
                quaternion_time_data[0]);
            return ERROR;
        }
    }

    IAS_LOG_DEBUG("Index into IMU %d", quat_start_index);
    IAS_LOG_DEBUG("Number of valid quaternions %d", valid_quat_total);

    if (quat_start_index >= (valid_quat_total - 1))
    {
        IAS_LOG_ERROR("Finding start time in quaternion data. "
                      "quaternion start index: %d "
                      "quaternion total: %d ", 
                      quat_start_index, valid_quat_total);
        return ERROR;
    }

    /* Window quaternion data according to what was windowed
       for IMU data. The quaternions should fall inside the IMU data. */
    count = imu_count/scale;

    /* Find the index in the quaternion data that matches the end time of 
       the IMU data */
    quat_end_index = quat_start_index + count;
    if (quat_end_index >= valid_quat_total) 
        quat_end_index = valid_quat_total - 1;
    while ((quaternion_time_data[quat_end_index] 
              > imu_time_data[imu_count - 1]
              || fabs(quaternion_time_data[quat_end_index] 
                      - quaternion_time_data[quat_end_index - 1] 
                      - IAS_ANCILLARY_QUAT_TIME) 
                 > QTIME_TOL) 
           && quat_end_index > quat_start_index ) 
    {
        quat_end_index--;
    }
    valid_quat_total = quat_end_index - quat_start_index + 1;

    IAS_LOG_DEBUG("Using a start index to quaternion of %d for %d points",
                 quat_start_index, valid_quat_total);

    if (valid_quat_total < 1)
    {
        IAS_LOG_ERROR("Incorrect number of quaternion calculated %d",
                      valid_quat_total);
        return ERROR;
    }

    /* Subset the valid quaternion data to fit inside the ephemeris data */
    (*invalid_quaternion_count) = 0;
    for (index = 0, quat_index = quat_start_index;
         index < valid_quat_total;
         index++, quat_index++)
    {
        valid_quaternion_data[index].vector.x 
            = valid_quaternion_data[quat_index].vector.x;
        valid_quaternion_data[index].vector.y 
            = valid_quaternion_data[quat_index].vector.y;
        valid_quaternion_data[index].vector.z 
            = valid_quaternion_data[quat_index].vector.z;
        valid_quaternion_data[index].scalar 
            = valid_quaternion_data[quat_index].scalar;
        quaternion_time_data[index] = quaternion_time_data[quat_index];
        valid_quaternion_flag[index] = valid_quaternion_flag[quat_index];
        if (valid_quaternion_flag[index] == 0) 
            (*invalid_quaternion_count)++;
    }

    *valid_quaternion_count = valid_quat_total;

    return SUCCESS;
}
