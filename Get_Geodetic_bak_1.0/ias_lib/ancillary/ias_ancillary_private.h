#ifndef IAS_ANCILLARY_PRIVATE_H
#define IAS_ANCILLARY_PRIVATE_H

#include "ias_l0r.h"
#include "ias_cpf.h"
#include "ias_ancillary_io.h"

/* turn on the generation of data files for debugging
   A value of 0 turns the file generation off.
   A value of 1 turns the file generation on.
*/
#define DEBUG_GENERATE_DATA_FILES 0

/* A macro to convert(inline) an IAS_L0R_TIME structure into a value */
#define CONVERT_TIME_TO_SECONDS_SINCE_J2000(a) \
    (((a).days_from_J2000 * IAS_SEC_PER_DAY) + (a).seconds_of_day)

#define IAS_ANCILLARY_IMU_TIME   0.02   /* Units are in seconds. 1.0 or 0.02 */
#define IAS_ANCILLARY_QUAT_TIME  0.02   /* Units are in seconds. 0.1 or 0.02 */

typedef enum ias_coordinate_system
{
    IAS_ECEF,              /* Earth Centered Earth Fixed */
    IAS_ECI,               /* Earth Centered Inertial */
} IAS_COORDINATE_SYSTEM;

/* Function prototypes */
int ias_ancillary_preprocess_attitude
(
    IAS_CPF *cpf,                     /* I: CPF structure */
    const IAS_L0R_ATTITUDE *l0r_attitude, /* I: L0R attitude structure */
    int l0r_attitude_count,           /* I: Number of attitude records in L0R */
    const IAS_L0R_IMU *l0r_imu,       /* I: IMU data */
    int l0r_imu_count,                /* I: IMU record count */
    const double *interval_start_time,/* I: Interval start YEAR, DOY, SOD */
    const double *interval_stop_time, /* I: Interval stop YEAR, DOY, SOD */
    const IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data,
                                      /* I: Pointer to ephem data */
    IAS_ACQUISITION_TYPE acq_type,    /* I: Image acquisition type */
    IAS_ANC_ATTITUDE_DATA **anc_attitude_data, /* O: Pointer to ancillary 
                                                     attitude data */
    int *invalid_attitude_count       /* O: Number of invalid attitude points 
                                            detected */
);

void ias_ancillary_get_position_and_velocity_at_time
(
    const IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data,/* I: ephemeris struct */
    IAS_COORDINATE_SYSTEM coordinate_type, /* I: Coordinate system to generate
                                                 values for */
    double eph_time,         /* I: Delta time from the reference time */
    IAS_VECTOR *satpos,      /* O: New satellite position at "dtime" */
    IAS_VECTOR *satvel       /* O: New satellite velocity at "dtime" */
);

int ias_ancillary_compute_spacecraft_attitude
(
    const IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data,
                                     /* I: Pointer to ephem data */
    int *imu_count,                  /* I/O: Number of IMU records (size of
                                           IMU data arrays) */
    const IAS_QUATERNION *valid_quaternion_data,
                                     /* I: Array of valid quaternions */
    int valid_quaternion_count,      /* I: Number of valid_quaternions (size of
                                           IMU data arrays) */
    double eph_start_time,           /* I: Ephemeris start seconds from J2000 */
    double eph_stop_time,            /* I: Ephemeris stop seconds from J2000 */
    const double *interval_start_time, /* I: interval start YEAR, DOY, SOD */
    const double *interval_stop_time,  /* I: interval stop YEAR, DOY, SOD */
    IAS_ACQUISITION_TYPE acq_type,   /* I: Image acquisition type */
    int imu_valid,                   /* I: Flag for valid/invalid IMU data */
    int interpolate_quaternions_flag,/* I: flag that indicates there are
                                           missing quaternions to interpolate
                                           using Kalman filtering */
    double *imu_epoch,               /* I/O: Year, DOY, SOD for start of IMU */
    double *imu_time_data,           /* I/O: Array of IMU seconds since j2000 */
    double *quaternion_time_data,    /* I/O: Array of quaternion times */
    int *valid_imu_flag,             /* I/O: Array of IMU data quality flags */
    int *valid_quaternion_flag,      /* I/O: Array of quaternion quality flags*/
    IAS_VECTOR *imu_data,            /* O: Array of IMU records */
    double *imu_start_time           /* O: Time of first quaternion and IMU */
);

int ias_ancillary_convert_imu_to_attitude
(
    IAS_CPF *cpf,                    /* I: CPF structure */
    IAS_ACQUISITION_TYPE acq_type,   /* I: Image acquisition type */
    const IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data,
                                     /* I: Pointer to ephem data */
    const double *imu_epoch,         /* I: Year, DOY, SOD for start of IMU */
    const IAS_VECTOR *imu_data,      /* I: Array of valid IMU records */
    const double *imu_time_data,     /* I: Array of IMU seconds since 
                                              j2000 */
    int imu_count,                   /* I: Number of IMU samples (size of
                                           IMU data arrays) */
    double imu_start_time,           /* I: Time of first quaternion and IMU */
    IAS_ANC_ATTITUDE_DATA *anc_attitude_data /* O: Ancillary attitude data */
);

int ias_ancillary_correct_imu_orbital_motion
(
    IAS_ACQUISITION_TYPE acq_type, /* I: Image acquisition type */
    const IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data,
                                   /* I: Pointer to ephem data */
    const IAS_VECTOR *attitude_ref,/* I: Roll-pitch-yaw mean offset */
    int imu_count,                 /* I: Number of IMU points (array size) */
    const double *imu_time_data,   /* I: Array of IMU seconds since 
                                                 J2000 */
    IAS_VECTOR *imu_data           /* I/O: Array of IMU data */
);

int ias_ancillary_kalman_smooth_ephemeris
(
    IAS_CPF *cpf,                   /* I: CPF structure */
    int valid_ephemeris_count,      /* I: number of ephemeris points in arrays*/
    const double *ephemeris_time_data, /* I: array of ephemeris time data */
    const IAS_VECTOR *raw_pos,      /* I: array of ephemeris position data */
    const IAS_VECTOR *raw_vel,      /* I: array of ephemeris velocity data */
    double ephemeris_sampling_rate, /* I: ephemeris sampling interval */
    IAS_VECTOR *smoothed_eph_pos,   /* O: smoothed GPS position */
    IAS_VECTOR *smoothed_eph_vel    /* O: smoothed GPS velocity */
);

int ias_ancillary_kalman_smooth_imu
(
    int quaternion_count,            /* I: Number of attitude measurements */
    int imu_count,                   /* I: Number of IMU points (array size) */
    const double *quaternion_time_data, /* I: Array of quaternion times */
    IAS_VECTOR *quaternion,          /* I/O: Array of quaternion data */
    int *valid_quaternion_flag,      /* I/O: Array of quaternion quality flags*/
    double *imu_time_data,           /* I/O: Array of IMU seconds since j2000 */
    IAS_VECTOR *imu_data,            /* I/O: Array of input IMU and output 
                                            "smoothed" data */
    int *valid_imu_flag              /* I/O: Array of IMU data quality flags */
);

int ias_ancillary_correct_ephemeris_time
(
    const IAS_VECTOR *raw_pos,  /* I: array of ephemeris position data */
    const IAS_VECTOR *raw_vel,  /* I: array of ephemeris velocity data */
    int valid_ephemeris_count,  /* I: number of ephemeris points in the arrays*/
    double *ephemeris_time_data /* I/O: array of ephemeris time data */
);

int ias_ancillary_smooth_ephemeris
(
    IAS_ACQUISITION_TYPE acquisition_type, /* I: Image acquisition type */
    IAS_CPF *cpf,                      /* I: CPF structure */
    const IAS_L0R_EPHEMERIS *l0r_ephemeris, /* I: L0R ephemeris structure */
    int l0r_ephemeris_count,           /* I: number of records in L0R data */
    const double *interval_start_time, /* I: interval start YEAR, DOY, SOD */
    const double *interval_stop_time,  /* I: interval stop YEAR, DOY, SOD */
    int *valid_ephemeris_count,        /* O: number of ephemeris points in the
                                             smoothed arrays */
    int *invalid_ephemeris_count,      /* O: number of bad ephemeris points */
    double *smoothed_ephemeris_seconds_since_j2000, /* O: array of smoothed
                                             ephemeris seconds since j2000 */
    IAS_VECTOR *smoothed_eph_pos,      /* O: array of smoothed ephemeris
                                             position data */
    IAS_VECTOR *smoothed_eph_vel       /* O: array of smoothed ephemeris
                                             velocity data */
);


////////////////////////////////////////////////////////////////////////////////////
//added by LYL at 2014/3/5
int ias_ancillary_smooth_ephemeris_for_MWD
(
    IAS_ACQUISITION_TYPE acquisition_type, /* I: Image acquisition type */
    IAS_CPF *cpf,                      /* I: CPF structure */
    const IAS_L0R_EPHEMERIS *l0r_ephemeris, /* I: L0R ephemeris structure */
    int l0r_ephemeris_count,           /* I: number of records in L0R data */
    int *valid_ephemeris_count,        /* O: number of ephemeris points in the
                                             smoothed arrays */
    int *invalid_ephemeris_count,      /* O: number of bad ephemeris points */
    double *smoothed_ephemeris_seconds_since_j2000, /* O: array of smoothed
                                             ephemeris seconds since j2000 */
    IAS_VECTOR *smoothed_eph_pos,      /* O: array of smoothed ephemeris
                                             position data */
    IAS_VECTOR *smoothed_eph_vel,       /* O: array of smoothed ephemeris
                                             velocity data */
    double *ephemeris_start_time,		/* O: first valid ephemeris data time */
    double *ephemeris_end_time			/* O: last valid ephemeris data time */
);
///////////////////////////////////////////////////////////////////////////////////

int ias_ancillary_build_ephemeris
(
    IAS_CPF *cpf,                        /* I: CPF structure */
    int valid_ephemeris_count,           /* I: number of ephemeris points for
                                               the smoothed arrays */
    const double *smoothed_ephemeris_seconds_since_j2000,
                                         /* I: array of smoothed ephemeris
                                               seconds since j2000 */
    const IAS_VECTOR *smoothed_eph_pos,  /* I: array of smoothed ephemeris
                                               position data */
    const IAS_VECTOR *smoothed_eph_vel,  /* I: array of smoothed ephemeris
                                               velocity data */
    IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data /* O: pointer to ephem data */
);

int ias_ancillary_get_quaternion_at_time
(
    const IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data,
                                     /* I: Pointer to ephem data */
    IAS_ACQUISITION_TYPE acq_type,   /* I: Image acquisition type */
    IAS_COORDINATE_SYSTEM coordinate_type, /* I: Coordinate system to generate
                                                 values for */
    double delta_time,               /* I: Time from the current record */
    double error_tolerance,          /* I: Error tolerance for the conversion */
    double euler[3][3],              /* I/O: Euler rotational matrix */
    IAS_QUATERNION *euler_quat       /* O: Euler matrix in quaternion */
);

#endif /* IAS_ANCILLARY_PRIVATE_H */
