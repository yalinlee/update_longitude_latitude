/*-----------------------------------------------------------------------------

NAME: ias_l0r_compare_routines.h

PURPOSE: These routines are used to compare L0R structures.  They are
         typically used just for testing purposes.

ALGORITHM REFERENCES: NONE

NOTES:

-----------------------------------------------------------------------------*/

#ifndef IAS_L0R_COMPARE_H
#define IAS_L0R_COMPARE_H

/***************************************
*Metadata comparison routines
***************************************/
int ias_l0r_mta_compare_file
(
    const IAS_L0R_FILE_METADATA *file_metadata, /* I: 1st struct to compare */
    const IAS_L0R_FILE_METADATA *file_metadata2, /* I: 2nd struct to compare */
    int *different_elements /* O: Number of elements that differ */
);

int ias_l0r_mta_compare_interval
(
    const IAS_L0R_INTERVAL_METADATA *interval_metadata,
    /* I: 1st struct to compare */
    const IAS_L0R_INTERVAL_METADATA *interval_metadata2,
    /* I: 2nd struct to compare */
    int *different_elements /* O: Number of elements that differ */
);

int ias_l0r_mta_compare_scene
(
    const IAS_L0R_SCENE_METADATA *scene_metadata, /* I: 1st struct to compare */
    const IAS_L0R_SCENE_METADATA *scene_metadata2, /* I: 2nd struct to compare*/
    int *different_elements /* O: Number of elements that differ */
);

/***************************************
*Ancillary comparison routines
***************************************/
int ias_l0r_compare_ancillary_attitude
(
    const IAS_L0R_ATTITUDE *attitude, /* I: 1st struct to compare */
    const IAS_L0R_ATTITUDE *attitude2, /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
                                   the same, 1 or -1 otherwise */
);

int ias_l0r_compare_ancillary_attitude_filter_states
(
    const IAS_L0R_ATTITUDE_FILTER_STATES *attitude_filter,
    /* I: 1st struct to compare */
    const IAS_L0R_ATTITUDE_FILTER_STATES *attitude_filter2,
    /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
                                   the same, 1 or -1 otherwise */
);

int ias_l0r_compare_ancillary_ephemeris
(
    const IAS_L0R_EPHEMERIS *ephemeris, /* I: 1st struct to compare */
    const IAS_L0R_EPHEMERIS *ephemeris2, /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
                                   the same, 1 or -1 otherwise */
);

int ias_l0r_compare_ancillary_gps_position
(
    const IAS_L0R_GPS_POSITION *gps_position, /* I: 1st struct to compare */
    const IAS_L0R_GPS_POSITION *gps_position2, /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
                                   the same, 1 or -1 otherwise */
);

int ias_l0r_compare_ancillary_gps_range
(
    const IAS_L0R_GPS_RANGE *gps_range, /* I: 1st struct to compare */
    const IAS_L0R_GPS_RANGE *gps_range2, /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
                                   the same, 1 or -1 otherwise */
);

int ias_l0r_compare_ancillary_imu
(
    const IAS_L0R_IMU *imu_data, /* I: 1st struct to compare */
    const IAS_L0R_IMU *imu_data2, /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
                                   the same, 1 or -1 otherwise */
);

int ias_l0r_compare_ancillary_imu_latency
(
    const IAS_L0R_IMU_LATENCY *imu_latency_data, /* I: 1st struct to compare */
    const IAS_L0R_IMU_LATENCY *imu_latency_data2, /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
                                   the same, 1 or -1 otherwise */
);

int ias_l0r_compare_ancillary_oli_telemetry
(
    const IAS_L0R_OLI_TELEMETRY *telemetry_data, /* I: 1st struct to compare */
    const IAS_L0R_OLI_TELEMETRY *telemetry_data2, /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
                                   the same, 1 or -1 otherwise */
);

int ias_l0r_compare_ancillary_tirs_telemetry
(
    const IAS_L0R_TIRS_TELEMETRY *telemetry_data, /* I: 1st struct to compare */
    const IAS_L0R_TIRS_TELEMETRY *telemetry_data2,
    /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
                                   the same, 1 or -1 otherwise */
);

int ias_l0r_compare_ancillary_star_tracker_centroid
(
    const IAS_L0R_STAR_TRACKER_CENTROID *centroid,
    /* I: 1st struct to compare */
    const IAS_L0R_STAR_TRACKER_CENTROID *centroid2,
    /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
                                   the same, 1 or -1 otherwise */
);

int ias_l0r_compare_ancillary_star_tracker_quaternion
(
    const IAS_L0R_STAR_TRACKER_QUATERNION *quaternion,
    /* I: 1st struct to compare */
    const IAS_L0R_STAR_TRACKER_QUATERNION *quaternion2,
    /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
                                   the same, 1 or -1 otherwise */
);

int ias_l0r_compare_ancillary_gyro_temperature
(
    const IAS_L0R_GYRO_TEMPERATURE *temperature_gyro,
    /* I: 1st struct to compare */
    const IAS_L0R_GYRO_TEMPERATURE *temperature_gyro2,
    /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
                                   the same, 1 or -1 otherwise */
);

int ias_l0r_compare_ancillary_oli_tirs_temperature
(
    const IAS_L0R_OLI_TIRS_TEMPERATURE *temperature_oli,
    /* I: 1st struct to compare */
    const IAS_L0R_OLI_TIRS_TEMPERATURE *temperature_oli2,
    /* I: 2nd struct to compare */
    int *different_elements /* O:  Contains 0 if the structures are
                                   the same, 1 or -1 otherwise */
);

#endif
