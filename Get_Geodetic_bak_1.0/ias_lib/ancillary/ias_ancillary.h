#ifndef IAS_ANCILLARY_H
#define IAS_ANCILLARY_H

/*****************************************************************************
Name: ias_ancillary.h

Purpose: Implements the shared Ancillary library routine(s). 

Notes: Some notation regarding the differences between the terms IMU, IRU, 
SIRU, and gyro.
- IRU is an old term and will not be used in this module.
- IMU is a generic term for a device which measures and reports a craft's 
  velocity and orientation using a combination of accelerometers and gyros.
- SIRU is a trademarked term for a device built by Northrop Grumman. It reports
  orientation in 4 axes, in contrast with an IMU which traditionally has 3 axes.
- Gyro is an abbreviation for gyroscope, a direction-determining, 
  orientation-stabilizing device.
- Landsat 7 has an IMU; Landsat 8 has a SIRU. A routine in this module converts 
  the 4 axes of the SIRU to 3 axes.

Acronym references:
IRU: Inertial Reference Unit
IMU: Inertial Measurement Unit
SIRU: Scalable Inertial Reference Unit

*****************************************************************************/

#include "ias_l0r.h"
#include "ias_cpf.h"
#include "ias_structures.h"
#include "ias_ancillary_io.h"

/* Function prototypes */
int ias_ancillary_preprocess
(
    IAS_CPF *cpf,                     /* I: CPF structure */
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
    IAS_ANC_ATTITUDE_DATA **anc_attitude_data,/* O: pointer to attitude data */
    IAS_ANC_EPHEMERIS_DATA **anc_ephemeris_data,/* O: pointer to ephem data */
    int *invalid_ephemeris_count,    /* O: Number of invalid ephemeris points 
                                           detected */
    int *invalid_attitude_count      /* O: Number of invalid attitude points 
                                           detected */
);



int ias_ancillary_get_start_stop_frame_times
(
    IAS_L0R_OLI_FRAME_HEADER *oli_frame_headers,   /* I: OLI frame headers */
    IAS_L0R_TIRS_FRAME_HEADER *tirs_frame_headers, /* I: OLI frame headers */
    int oli_frame_header_size,   /* I: Number of OLI frame header records */
    int tirs_frame_header_size,  /* I: Number of TIRS frame header records */
    double *oli_start_frame_time,/* O: OLI start frame time */
    double *oli_stop_frame_time, /* O: OLI stop frame time */
    double *tirs_start_frame_time,/* O: OLI start frame index */
    double *tirs_stop_frame_time, /* O: OLI stop frame index */
    int *oli_start_frame_index,   /* O: TIRS start frame time */
    int *oli_stop_frame_index,    /* O: TIRS stop frame time */
    int *tirs_start_frame_index,  /* O: TIRS start frame index */
    int *tirs_stop_frame_index    /* O: TIRS start frame index */
);

int ias_ancillary_preprocess_ephemeris
(
    IAS_CPF *cpf,                      /* I: CPF structure */
    const IAS_L0R_EPHEMERIS *l0r_ephemeris,/* I: L0R ephemeris structure */
    int l0r_ephemeris_count,           /* I: number of records in L0R data */
    const double *interval_start_time, /* I: interval start YEAR, DOY, SOD */
    const double *interval_stop_time,  /* I: interval stop YEAR, DOY, SOD */
    IAS_ACQUISITION_TYPE acquisition_type, /* I: Image acquisition type */
    IAS_ANC_EPHEMERIS_DATA **anc_ephemeris_data, /* O: pointer to ephem data */
    int *invalid_ephemeris_count       /* O: number of invalid ephemeris points 
                                             detected */
);

////////////////////////////////////////////////////////////////////////////////////
//add by LYL at 2014\3\5
int ias_ancillary_preprocess_ephemeris_for_MWD
(
    IAS_CPF *cpf,                      /* I: CPF structure */
    const IAS_L0R_EPHEMERIS *l0r_ephemeris,/* I: L0R ephemeris structure */
    int l0r_ephemeris_count,           /* I: number of records in L0R data */
    IAS_ACQUISITION_TYPE acquisition_type, /* I: Image acquisition type */
    IAS_ANC_EPHEMERIS_DATA **anc_ephemeris_data, /* O: pointer to ephem data */
    int *invalid_ephemeris_count,       /* O: number of invalid ephemeris points
                                             detected */
    double *ephemeris_start_time,		/* O: first valid ephemeris data time */
    double *ephemeris_end_time			/* O: last valid ephemeris data time */

);
//////////////////////////////////////////////////////////////////////////////////

int ias_ancillary_identify_quaternion_outliers
(
    IAS_CPF *cpf,                    /* I: CPF structure */
    const IAS_L0R_ATTITUDE *l0r_attitude, /* I: L0R quaternion structure */
    int l0r_attitude_count,          /* I: Number of ACS records in L0Ra */
    double *quaternion_time_data,    /* O: Array of quaternion times */
    IAS_QUATERNION *quaternion_data, /* O: Array of valid quaternions */
    int *valid_quaternion_flag,      /* O: Array of quaternion quality flag */
    int quaternion_array_count,      /* I: entry count in quaternion arrays */
    int *valid_quaternion_count,      /* O: valid entry count in quat arrays */
    int *interpolate_quaternions_flag/* O: flag that indicates there are
                                           missing quaternions that need to be
                                           interpolated */
);

int ias_ancillary_convert_imu_to_acs
(
    IAS_CPF *cpf,                    /* I: CPF structure */
    const IAS_L0R_IMU *source_l0r_imu,      /* I: IMU data from L0R */
    int l0r_imu_count,               /* I: IMU record count from L0R */
    int valid_quaternion_count,      /* I: Number of valid quaternions */
    double *quaternion_time_data,    /* I: Array of quaternion times */
    IAS_VECTOR *imu_data,            /* O: Array of IMU records */
    double *imu_time_data,           /* O: Array of IMU seconds since j2000 */
    int *valid_imu_flag,             /* O: Array of IMU quality flags */
    int *imu_valid,                  /* O: Flag for valid/invalid IMU data */
    int *imu_count                   /* O: Number of IMU records (IMU data
                                           arrays are of this size) */
);

int ias_ancillary_extract_valid_imu_data_window
(
    const IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data, /* I: Pointer to ephem 
                                                            data */
    double *imu_time_data,           /* I/O: Array of IMU seconds since j2000 */
    IAS_VECTOR *imu_data,            /* I/O: Array of IMU records */
    int *valid_imu_flag,             /* I/O: Array of IMU quality flag */
    int *imu_count,                  /* I/O: Number of IMU records (size of
                                             IMU data arrays) */
    double *imu_epoch,               /* O: Year, DOY, SOD for start of IMU */
    double *eph_start_time,          /* O: Ephemeris start time */
    double *eph_stop_time,           /* O: Ephemeris stop time */
    int *invalid_imu_count           /* O: Number of invalid IMU samples */
);

int ias_ancillary_extract_valid_quaternion_window
(
    const double *imu_time_data,          /* I: Array of IMU seconds since 
                                             j2000 */
    int imu_count,                        /* I: Number of IMU records */
    IAS_QUATERNION *valid_quaternion_data,/* I/O: Array of valid quaternions */
    double *quaternion_time_data,         /* I/O: Array of quaternion times */
    int *valid_quaternion_flag,           /* I/O: Array of quaternion quality 
                                             flags which track valid data */
    int *valid_quaternion_count,          /* I/O: Valid quaternion totals (size
                                             of quaternion data arrays) */
    int *invalid_quaternion_count         /* O: Number of invalid quaternions */
);

IAS_L0R_IMU *ias_ancillary_process_imu_times
(
    const IAS_L0R_IMU *source_l0r_imu, /* I: Ancillary IMU records */
    double imu_offset_scale,    /* I: CPF IMU clock scale factor, in seconds
                                      per IMU offset count */
    int imu_count,              /* I: Number of records */
    int *imu_valid,             /* O: Flag to indicate whether valid IMU data
                                      was found (mainly the sync event was
                                      present).  1 = valid, 0 = invalid */
    double *imu_time            /* O: IMU sample times (SC epoch) */
);

int ias_ancillary_get_attitude_at_time
(
   const IAS_ANC_ATTITUDE_DATA *att,    /* I: Attitude structure */
   double att_time,                     /* I: Actual time (sec from epoch) */
   double *roll,                        /* O: Output roll (radians) */
   double *pitch,                       /* O: Output pitch (radians) */
   double *yaw                          /* O: Output yaw (radians) */
);

#endif /* IAS_ANCILLARY_H */