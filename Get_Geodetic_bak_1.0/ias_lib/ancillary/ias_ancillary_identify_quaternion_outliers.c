/**********************************************************************
Name: ias_ancillary_identify_quaternion_outliers

Purpose: Filter out quaternion outliers

***********************************************************************/
#include <math.h>
#include <string.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_structures.h"
#include "ias_math.h"
#include "ias_cpf.h"
#include "ias_ancillary_io.h"
#include "ias_ancillary.h"
#include "ias_ancillary_private.h"

/*****************************************************************************
Name: check_for_anomalies

Purpose: This routine checks for known anomalies in the ancillary data that
         can impact the quaternions and invalidates the quaternions that are
         affected.

Returns: nothing

Notes:
    - There are three types (of the known 4) of ancillary anomalies that can
      impact quaternions.  The 684 bytes refers to the spacecraft data stream
      that is assembled into the mission data file.  The checks here account
      for where those bytes map to in our structures.
    - Type 1: An entire record of 50 quaternion samples is missing.  50
         invalid quaternions are inserted which will later be interpolated by
         the Kalman filtering
    - Type 2: The second group of 684 bytes (bytes 685-1368) are duplicated
        from previous frame.  The last 22 quaternions of the record are
        affected and therefore are flagged as invalid.
    - Type 4: The first group of 684 bytes (bytes 1 - 684) are duplicated
        from previous frame.  The affected samples are flagged as invalid
        along with the first sample from the following record.
    - As of 06/06/2013 when this update is being made, no instances of the
      type 4 anomaly have been detected.

*****************************************************************************/
static void check_for_anomalies
(
    double *times,               /* I/O: Array of quaternion times */
    IAS_QUATERNION *quaternions, /* I/O: Array of valid quaternions */
    int *valid_quaternion_flag,  /* O: Array of quaternion quality flags */
    int quat_count,              /* I: entries available in the quaternion
                                       arrays */
    int *quat_index,             /* I/O: current index into the arrays (can
                                         be updated for some anomalies) */
    int *interpolate_quaternions_flag /* I/O: flag that indicates there were
                                              missing quaternions that need to
                                              be interpolated */
)
{
    int index = *quat_index;
    int subfield = index % IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD;
    int prev_index = index - IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD;
    int start_index;

    /* Nothing to do here until past the first record */
    if (index < IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD)
        return;

    /* Check for the type 1 anomaly on the second sample of the record */
    if (subfield == 1)
    {
        double two_record_time = 2 * IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD
            * IAS_ANCILLARY_IMU_TIME;

        prev_index = index - IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD;

        /* If the current and previous records have a time difference expected
           for 100 samples, a record is missing */
        if (fabs(times[index] - times[prev_index] - two_record_time)
                < IAS_ANCILLARY_IMU_TIME
            && fabs(times[index - 1] - times[prev_index - 1] - two_record_time)
                < IAS_ANCILLARY_IMU_TIME)
        {
            int i;
            int next_index;
            int fill_index;

            /* This is a type 1 anomaly, so issue a warning about it */
            IAS_LOG_WARNING("Missing record in quaternion data found at "
                "index %d", index - 1);

            /* The missing record will be inserted into the output buffers with
               the samples flagged as bad.  The bad samples will later be
               interpolated using the Kalman filtering.  So, set the flag to
               make sure the interpolation happens.  No example data has ever
               been found that has more than one missing record, so if the
               interpolate flag is already set, issue a warning since it is
               unusual. */
            if (!*interpolate_quaternions_flag)
            {
                *interpolate_quaternions_flag = 1;
                IAS_LOG_WARNING("Activating Kalman filter to interpolate "
                    "missing quaternions");
            }
            else
            {
                IAS_LOG_WARNING("More than one missing ancillary record "
                    "encountered");
            }

            /* Shift the two samples already processed to the next record if
               there is space for it.  The current record valid flag does
               not need to be copied since it will be set later. */
            next_index = index + IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD;
            if (next_index < quat_count)
            {
                quaternions[next_index - 1] = quaternions[index - 1];
                times[next_index - 1] = times[index - 1];
                valid_quaternion_flag[next_index - 1]
                    = valid_quaternion_flag[index - 1];

                quaternions[next_index] = quaternions[index];
                times[next_index] = times[index];
            }

            /* Fill in the current entire record with placeholder bad
               quaternions that will later be interpolated by the Kalman
               filtering */
            fill_index = index - 1;
            for (i = 0; i < IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD
                        && fill_index < quat_count; i++)
            {
                quaternions[fill_index].vector.x = 0.0;
                quaternions[fill_index].vector.y = 0.0;
                quaternions[fill_index].vector.z = 0.0;
                quaternions[fill_index].scalar = 0.0;
                times[fill_index] = times[fill_index - 1]
                    + IAS_ANCILLARY_IMU_TIME;
                valid_quaternion_flag[fill_index] = 0;
                fill_index++;
            }

            /* Advance the quaternion index by a full record to account for
               the inserted record */
            index += IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD;
            *quat_index = index;
        }
    }

    /* The type 2 and 4 anomalies are checked for when the last sample of the
       current record is reached */
    if (subfield == (IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD-1))
    {
        prev_index = index - IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD;

        /* Check for type 2 ancillary data anomaly:  duplicate second 684 byte
           block from previous record.  When this happens, the last 22
           quaternions of the current record are duplicates from the previous
           record so they need to be flagged as bad. */
        if (fabs(times[index] - times[prev_index]) < IAS_ANCILLARY_IMU_TIME
            && fabs(times[index - 1] - times[prev_index - 1])
                < IAS_ANCILLARY_IMU_TIME)
        {
            /* Duplicate data anomaly present */
            int i;

            for (i = index - 21; i <= index; i++)
            {
                times[i] = times[i - 1] + IAS_ANCILLARY_IMU_TIME;
                quaternions[i].vector.x = 0.0;
                quaternions[i].vector.y = 0.0;
                quaternions[i].vector.z = 0.0;
                quaternions[i].scalar = 0.0;
                valid_quaternion_flag[i] = 0;
            }
            IAS_LOG_WARNING("Duplicate quaternion data found at index %d, "
                    "deactivating 22 quaternions at %d - %d", index, index - 21,
                    index);
        }

        /* Check for type 4 ancillary data anomaly:  duplicate first 684
           byte block from previous record.  When this happens, the first
           29 quaternions of the current record are duplicates from the
           previous record so they need to be flagged as bad. */
        /* No instances of this anomaly type are known as of 06/06/2013 */

        /* Calculate the indices for the first sample of the current record and
           the previous record */
        start_index = index - IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD + 1;
        prev_index = start_index - IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD;

        /* If the first couple samples of the current and previous record are
           the same times, it is a type 4 anomaly */
        if (fabs(times[start_index] - times[prev_index])
                < IAS_ANCILLARY_IMU_TIME
            && fabs(times[start_index + 1] - times[prev_index + 1])
                < IAS_ANCILLARY_IMU_TIME)
        {
            int i;
            int start = start_index;
            int end = start_index + 29;
            /* Duplicate data anomaly present */
            for (i = start; i < end; i++)
            {
                times[i] = times[i - 1] + IAS_ANCILLARY_IMU_TIME;
                quaternions[i].vector.x = 0.0;
                quaternions[i].vector.y = 0.0;
                quaternions[i].vector.z = 0.0;
                quaternions[i].scalar = 0.0;
                valid_quaternion_flag[i] = 0;
            }
            IAS_LOG_WARNING("Duplicate quaternion data found at index %d, "
                    "deactivating 29 quaternions at %d - %d", start, start,
                    end - 1);
        }
    }
}

/**********************************************************************
Name: ias_ancillary_identify_quaternion_outliers

Purpose: Filter out quaternion outliers

Returns: SUCCESS or ERROR
***********************************************************************/
int ias_ancillary_identify_quaternion_outliers
(
	IAS_CPF *cpf,                    /* I: CPF structure */
    const IAS_L0R_ATTITUDE *l0r_attitude, /* I: L0R quaternion structure */
    int l0r_attitude_count,          /* I: Number of ACS records in L0Ra */
    double *quaternion_time_data,    /* O: Array of quaternion times */
    IAS_QUATERNION *quaternion_data, /* O: Array of valid quaternions */
    int *valid_quaternion_flag,      /* O: Array of quaternion quality flags */
    int quaternion_array_count,      /* I: entry count in quaternion arrays */
    int *valid_quaternion_count,     /* O: valid entry count in quat arrays */
    int *interpolate_quaternions_flag /* O: flag that indicates there are
										    missing quaternions that need to be
										    interpolated */
)
{
    int l0r_index;       /* loop variable for the index of the current l0r
                            quaternion */
    int quat_index;      /* count of quaternions populated in the arrays */
    double magnitude;    /* magnitude of quaternion */

    const struct IAS_CPF_ANCILLARY_QA_THRESHOLDS *ancil_data;
    double quaternion_tolerance;

#if DEBUG_GENERATE_DATA_FILES == 1
    /* Only for debugging. */
    FILE *fp;
#endif

    *valid_quaternion_count = 0;
    *interpolate_quaternions_flag = 0;

    /* Read CPF ancillary data */
    ancil_data = ias_cpf_get_ancil_qa_thresholds(cpf);
    if (ancil_data == NULL)
    {
        IAS_LOG_ERROR("Reading CPF ancillary data");
        return ERROR;
    }
    quaternion_tolerance = 
        ancil_data->quaternion_normalization_outlier_threshold;

    IAS_LOG_DEBUG("Validate Quaternion attitude and generate outliers");

    IAS_LOG_INFO("Number of attitude quaternion points %d", l0r_attitude_count);

#if DEBUG_GENERATE_DATA_FILES == 1
    fp = fopen("anc.quat.l0r.dat", "wb");
#endif

    /* The logic to identify anomalies expects to get full records of
       quaternions since it is looking for a very specific pattern.  Issue a
       warning if there isn't a multiple of the quaternions per record */
    if ((l0r_attitude_count % IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD) != 0)
    {
        IAS_LOG_WARNING("Processing %d attitude quaternions when a multiple "
            "of %d was expected.  Anomaly detection may not work correctly.",
            l0r_attitude_count, IAS_L0R_NUM_SAMPLES_PER_IMU_RECORD);
    }

    /* loop through L0R quaternions keeping any whose magnitude falls
       within the tolerance */
    for (l0r_index = 0, quat_index = 0;
         l0r_index < l0r_attitude_count && quat_index < quaternion_array_count;
         l0r_index++)
    {
        /* copy it to the valid quaternion list */
        quaternion_data[quat_index].vector =
            l0r_attitude[l0r_index].inertial_to_body.vector;
        quaternion_data[quat_index].scalar =
            l0r_attitude[l0r_index].inertial_to_body.scalar;

        /* set the current time for this quaternion */
        quaternion_time_data[quat_index] =
            IAS_L0R_CONVERT_TIME_TO_SECONDS_SINCE_J2000(
                l0r_attitude[l0r_index].l0r_time);

        /* Check of anomalies in the data.  The routine can modify the data
           in the quaternion arrays, including zero filling a missing record. */
        check_for_anomalies(quaternion_time_data, quaternion_data,
                    valid_quaternion_flag, quaternion_array_count, &quat_index,
                    interpolate_quaternions_flag);

        /* figure out the magnitude */
        magnitude = ias_math_compute_quaternion_magnitude(
                        &quaternion_data[quat_index]);

        if (fabs(magnitude - 1.0) < quaternion_tolerance)
        {
            valid_quaternion_flag[quat_index] = 1;
#if DEBUG_GENERATE_DATA_FILES == 1
            fprintf(fp,
                "%d %f    %e -> %f    %e -> %f     %e -> %f    %e -> %f\n",
                l0r_index, quaternion_time_data[quat_index],
                quaternion_data[quat_index].vector.x,
                l0r_attitude[l0r_index].inertial_to_body.vector.x,
                quaternion_data[quat_index].vector.y,
                l0r_attitude[l0r_index].inertial_to_body.vector.y,
                quaternion_data[quat_index].vector.z,
                l0r_attitude[l0r_index].inertial_to_body.vector.z,
                quaternion_data[quat_index].scalar,
                l0r_attitude[l0r_index].inertial_to_body.scalar);
#endif
        }
        else
        {
            valid_quaternion_flag[quat_index] = 0;
            IAS_LOG_DEBUG("Invalid Quaternion - index: %d x:%f y:%f z:%f "
                    "scalar:%f -> magnitude: %f", l0r_index,
                    quaternion_data[quat_index].vector.x,
                    quaternion_data[quat_index].vector.y,
                    quaternion_data[quat_index].vector.z,
                    quaternion_data[quat_index].scalar,
                    magnitude);
        }

        quat_index++;
    }

#if DEBUG_GENERATE_DATA_FILES == 1
    fclose(fp);
#endif

    *valid_quaternion_count = quat_index;

    return SUCCESS;
}
