#ifndef IAS_ANCILLARY_IO_H
#define IAS_ANCILLARY_IO_H


/********************************************************************
   Data structure definitions for the attitude and ephemeris records
   written to/read from an ancillary data file.
*********************************************************************/
typedef struct ias_anc_attitude_record
{
    double seconds_from_epoch; /* seconds this attitude sample is from the
        epoch time for the attitude samples, stored as the
        IAS_ATTITUDE_DATA utc_epoch_time */
    double eci_quaternion[4];  /* Quaternion information */
    double ecef_quaternion[4];
    double roll;               /* Roll in radians */
    double roll_rate;          /* Roll rate in rad/s */
    double pitch;              /* Pitch in radians */
    double pitch_rate;         /* Pitch rate in rad/s */
    double yaw;                /* Yaw in radians */
    double yaw_rate;           /* Yaw rate in rad/s */
} IAS_ANC_ATTITUDE_RECORD;

typedef struct ias_anc_attitude_data
{
    double utc_epoch_time[3];            /* [0]=year, [1]=day of year,
                                            [2]=seconds of day */
    int number_of_samples;
    IAS_ANC_ATTITUDE_RECORD records[1];  /* Array of attitude records
                                            to be allocated */
} IAS_ANC_ATTITUDE_DATA;

typedef struct ias_anc_ephemeris_record
{
    double seconds_from_epoch; /* seconds this ephemeris sample is from the
        epoch time for the ephemeris samples, stored as the
        IAS_EPHEMERIS_DATA utc_epoch_time */
    double eci_position[3];    /* [0]=X, [1]=Y, [2]=Z in meters */
    double eci_velocity[3];    /* [0]=V_X, [1]=V_Y, [2]=V_Z in meters/sec */
    double ecef_position[3];   /* [0]=X, [1]=Y, [2]=Z in meters */
    double ecef_velocity[3];   /* [0]=V_X, [1]=V_Y, [2]=V_Z in meters/sec */
} IAS_ANC_EPHEMERIS_RECORD;

typedef struct ias_anc_ephemeris_data
{
    double utc_epoch_time[3];            /* [0]=year, [1]=day of year,
                                            [2]=seconds of day */
    int number_of_samples;
    IAS_ANC_EPHEMERIS_RECORD records[1]; /* Array of ephemeris records
                                            to be allocated */
} IAS_ANC_EPHEMERIS_DATA;


/*******************************************************************
   Prototypes for ancillary data access functions.  These provide
   the main interfaces between the application and the ancillary
   data.
********************************************************************/
IAS_ANC_ATTITUDE_DATA *ias_ancillary_allocate_attitude
(
    int number_of_records        /* I: total number of attitude records */
);


IAS_ANC_EPHEMERIS_DATA *ias_ancillary_allocate_ephemeris
(
    int number_of_records        /* I: total number of ephemeris records */
);

void ias_ancillary_free_attitude
(
    IAS_ANC_ATTITUDE_DATA *attitude_data    /* I: Pointer to allocated
                                               attitude data buffer */
);

void ias_ancillary_free_ephemeris
(
    IAS_ANC_EPHEMERIS_DATA *ephemeris_data  /* I: Pointer to allocated
                                               ephemeris data buffer */
);

int ias_ancillary_is_ancillary_file
(
    const char *ancillary_filename          /* I: Name of (potential)
                                                  ancillary file */
);

int ias_ancillary_write
(
    const char *ancillary_data_filename,    /* I: name of ancillary data file */
    const IAS_ANC_ATTITUDE_DATA *attitude_data,   /* I: pointer to attitude data */
    const IAS_ANC_EPHEMERIS_DATA *ephemeris_data  /* I: pointer to ephemeris data */
);

int ias_ancillary_read
(
    const char *ancillary_data_filename,     /* I: ancillary data file name */
    IAS_ANC_ATTITUDE_DATA **attitude_data,   /* IO: pointer to attitude data */
    IAS_ANC_EPHEMERIS_DATA **ephemeris_data  /* IO: pointer to ephemeris data */
);

#endif
