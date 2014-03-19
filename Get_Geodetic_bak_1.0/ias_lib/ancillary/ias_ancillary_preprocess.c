/*****************************************************************************
NAME: ias_ancillary_preprocess

PURPOSE: Preprocess the ancillary data. The processing is split into two
         paths here, one for ephemeris data and another for attitude data.

RETURN VALUE: Type = int
    Value    Description
    -----    -----------
    SUCCESS  Successful completion
    ERROR    Operation failed

MEMORY MANAGEMENT:

    Memory Passed Back To Parent Routine:
        anc_ephemeris_data
        anc_attitude_data

*****************************************************************************/

#include "ias_logging.h"
#include "ias_ancillary.h"
#include "ias_ancillary_private.h"

int ias_ancillary_preprocess
(
    IAS_CPF *cpf,                    /* I: CPF structure */
    const IAS_L0R_ATTITUDE *l0r_attitude, /* I: L0R attitude structure */
    int l0r_attitude_count,          /* I: number of attitude records in L0R */
    const IAS_L0R_EPHEMERIS *l0r_ephemeris,/* I: L0R ephemeris structure */
    int l0r_ephemeris_count,         /* I: number of (eph) records in L0R */
    const IAS_L0R_IMU *l0r_imu,      /* I: IMU data */
    int l0r_imu_count,               /* I: IMU record count */
    const double *interval_start_time, /* I: start time of the imagery in the
                                             interval (YEAR, DOY, SOD) */
    const double *interval_stop_time,  /* I: stop time of the imagery in the
                                             interval (YEAR, DOY, SOD) */
    IAS_ACQUISITION_TYPE acq_type,   /* I: image acquisition type */
    IAS_ANC_ATTITUDE_DATA **anc_attitude_data, /* O: pointer to attitude data */
    IAS_ANC_EPHEMERIS_DATA **anc_ephemeris_data, /* O: pointer to ephem data */
    int *invalid_ephemeris_count,    /* O: Number of bad ephemeris points 
                                           detected */
    int *invalid_attitude_count      /* O: Number of bad attitude points 
                                           detected */
)
{
    /* Preprocess the ephemeris data. */
    if (ias_ancillary_preprocess_ephemeris(cpf, l0r_ephemeris, 
            l0r_ephemeris_count, interval_start_time, 
            interval_stop_time, acq_type, anc_ephemeris_data, 
            invalid_ephemeris_count) != SUCCESS)
    {
        IAS_LOG_ERROR("Processing ephemeris data");
        return ERROR;
    }

    /* Processes the attitude data. */
    if (ias_ancillary_preprocess_attitude(cpf, l0r_attitude, 
            l0r_attitude_count, l0r_imu, l0r_imu_count,
            interval_start_time, interval_stop_time, *anc_ephemeris_data,
            acq_type, anc_attitude_data, invalid_attitude_count) != SUCCESS)
    {
        IAS_LOG_ERROR("Processing attitude data");
        return ERROR;
    }

    return SUCCESS;
}

