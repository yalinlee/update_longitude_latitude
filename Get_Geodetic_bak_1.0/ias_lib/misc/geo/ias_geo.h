#ifndef IAS_GEO_H
#define IAS_GEO_H

#include "ias_structures.h"
#include "ias_cpf.h"
#include "ias_l1g.h"            /* for the L1G_BAND_IO definition */
#include "ias_los_model.h"      /* for IAS_EARTH_CHARACTERISTICS */

/* Define a zone number for all projections except State Plane and UTM. */
#define NULLZONE 62

/* code for a point outside the image   */
#define OUTSIDE 99

#define WGS84_SPHEROID 12

/* Type defines for projection related structures */
typedef struct ias_geo_proj_transformation IAS_GEO_PROJ_TRANSFORMATION;
/* The ias_projection structure matches the gctp_projection structure
   definition.  The gctp_projection structure is not included here to prevent
   needing to modify the build to find gctp.h everywhere ias_geo.h is used. */
typedef struct ias_projection
{
    int proj_code;      /* Projection code */
    int zone;           /* Projection zone number - only has meaning for
                           projections like UTM and stateplane */
    int units;          /* Units of coordinates */
    int spheroid;       /* Spheroid code for the projection */
    double parameters[IAS_PROJ_PARAM_SIZE];
                        /* Array of projection parameters */
} IAS_PROJECTION;

/* For the ias_geo_analyze_gcp_records routine */
typedef struct GCP_RES_STATS  /* type is used for GCP residual stats */
{
    int  num_points;          /* number of points used to calculate the stats*/
    double mean_lat;          /* mean of the latitudes */
    double mean_lon;          /* mean of the longitudes */
    double mean_along_res;    /* mean of the along-track residuals */
    double mean_across_res;   /* mean of the across-track residuals */
    double rmse_along_res;    /* root-mean-square error of along-track res. */
    double rmse_across_res;   /* root-mean-square error of across-track res. */
    double sd_along_res;      /* standard deviation of along-track res. */
    double sd_across_res;     /* standard deviation of across-track res. */
    double corr_coeff;        /* correlation coeff. (along- vs across-track) */
} GCP_RES_STATS;

int ias_geo_analyze_gcp_records
(
    const double *GCP_lat,          /* I: residual data latitudes (degrees) */
    const double *GCP_lon,          /* I: residual data longitudes (degrees) */
    const double *along_track_res,  /* I: along-track residual data */
    const double *across_track_res, /* I: across-track residual data */
    int  num_valid,                 /* I: number of valid GCP residuals */
    GCP_RES_STATS *res_stats        /* O: GCP residual statistics */
);

int ias_geo_check_start_end_date
(
    int isdate,   /* I: start date (YYYYMMDD) */
    int iedate    /* I: end date (YYYYMMDD) */
);

/* defined in ias_geo_compute_potential.c */
double ias_geo_compute_earth_second_partial_x
(
    double semi_major_axis, /* I: Earth's semi-major axis */
    double gravity_constant,/* I: Earth's gravitational constant */
    double x,    /* I: Position in X */
    double y,    /* I: Position in Y */
    double z     /* I: Position in Z */
);

/* defined in ias_geo_compute_potential.c */
double ias_geo_compute_earth_second_partial_y
(   
    double semi_major_axis, /* I: Earth's semi-major axis */
    double gravity_constant,/* I: Earth's gravitational constant */
    double x,    /* I: Position in X */
    double y,    /* I: Position in Y */
    double z     /* I: Position in Z */ 
);

/* defined in ias_geo_compute_potential.c */
double ias_geo_compute_earth_second_partial_z
(   
    double semi_major_axis, /* I: Earth's semi-major axis */
    double gravity_constant,/* I: Earth's gravitational constant */
    double x,    /* I: Position in X */
    double y,    /* I: Position in Y */
    double z     /* I: Position in Z */
);

int ias_geo_compute_earth2orbit_transform
(
    const IAS_VECTOR *satpos, /* I: satellite position vector in Earth-fixed
                                    system, in meters */
    const IAS_VECTOR *satvel, /* I: inertial satellite velocity vector in
                                    Earth-fixed system, in m/sec */
    double transf_matrix[3][3] /* O: 3 X 3 transformation matrix, from the
                                  Earth-fixed system to the orbit-oriented
                                  cartesian system. */
);

int ias_geo_compute_forward_mappings
(
    int nzplanes,              /* I: List of planes in input space */
    const int *il,             /* I: List of lines in input space */
    const int *is,             /* I: List of samples in input space */
    const double *ol,          /* I: List of lines in output space */
    const double *os,          /* I: List of samples in output space */
    int nrows,                 /* I: Number of rows in the grid */
    int ncols,                 /* I: Number of columns in the grid */
    struct IAS_COEFFICIENTS *coef  /* O: Forward mapping coefficients */
);

int ias_geo_compute_getmjdcoords
(
    const double ephem_time[3], /* I: UTC Ephemeris time (year, doy and sod) */
    IAS_CPF *cpf,       /* I: the information from the CPF */
    double *mjd,        /* O: modified Julian date */
    double *x,          /* O: x shift pole wander in arc sec. */
    double *y,          /* O: y shift pole wander in arc sec. */
    double *ut1_utc     /* O: UT1-UTC, in seconds, due to variation of Earth's 
                              spin rate */
);

int ias_geo_compute_inverse_mappings
(
    int nzplanes,              /* I: Number of elevation levels */
    const int *il,             /* I: List of lines in input space */
    const int *is,             /* I: List of samples in input space */
    const double *ol,          /* I: List of lines in output space */
    const double *os,          /* I: List of samples in output space */
    int nrows,                 /* I: Number of rows in the grid */
    int ncols,                 /* I: Number of columns in the grid */
    struct IAS_COEFFICIENTS *coef  /* O: Inverse mapping coefficients */
);

int ias_geo_compute_map_edge 
(
    const IAS_GEO_PROJ_TRANSFORMATION *projection_transformation,
                      /* I: geographic degrees to projection transformation */
    double ul_lon,    /* I: Upper left longitude coordinate (in degrees) */
    double lr_lon,    /* I: Lower right longitude coordinate (in degrees) */
    double ul_lat,    /* I: Upper left latitude coordinate (in degrees) */
    double lr_lat,    /* I: Lower right latitude coordinate (in degrees) */
    double *pxmin,    /* O: Projection minimum in X */
    double *pxmax,    /* O: Projection maximum in X */
    double *pymin,    /* O: Projection minimum in Y */
    double *pymax     /* O: Projection maximum in Y */
);

int ias_geo_compute_orientation_matrices
(
    const IAS_VECTOR *satpos,   /* I: Satellite position (meters) */
    const IAS_VECTOR *satvel,   /* I: Satellite velocity (m/s) */
    double roll,                /* I: Corrected roll (radians) */
    double pitch,               /* I: Corrected pitch (radians) */
    double yaw,                 /* I: Corrected yaw (radians) */
    double orb2ecf[3][3],       /* O: Orbit to ECEF transformation */
    double attpert[3][3]        /* O: Attitude perturbation matrix */
);

int ias_geo_compute_proj2ls_poly 
(
    int number_samples,   /* I: Number of samples in the scene */ 
    int number_lines,     /* I: Number of lines in the scene */ 
    double corners[][4],  /* I: Lat and long for the four corners. Index 0 is
                                Y (lat) and 1 is X (long) with corners in 
                                UL, UR, LR, LL order. */
    double poly_x[4],     /* O: Projection-to-sample coefficients */
    double poly_y[4]      /* O: Projection-to-line coefficients */
);

int ias_geo_compute_proj2proj_poly 
(
    const double source_proj_x[4], /*I: source projection x corner coordinates*/
    const double source_proj_y[4], /*I: source projection y corner coordinates*/
    const double target_proj_x[4], /*I: target projection x corner coordinates*/
    const double target_proj_y[4], /*I: target projection y corner coordinates*/
    int include_xy_term,           /*I: include x*y term if not zero */
    double poly_x[4],              /*O: calculated x polynomial coefficients */
    double poly_y[4]               /*O: calculated y polynomial coefficients */
);

int ias_geo_compute_sun_position
(
    double julian_date,          /* I: Julian date on TDT or ET time scale */
    double *right_ascension_hrs, /* O: Right ascension referred to mean equator 
                                       and equinox of date (hours) */
    double *declination_deg,     /* O: Declination referred to mean equator and
                                       equinox of date (degrees) */
    double *distance_au          /* O: Geocentric distance (AU) */
);
 
int ias_geo_compute_moon_position
(
    double julian_date,          /* I: Julian ephemeris date */
    double *right_ascension_hrs, /* O: Right ascension referred to mean equator 
                                       and equinox of date (hours) */
    double *declination_deg,     /* O: Declination referred to mean equator and
                                       equinox of date (degrees) */
    double *distance_km          /* O: Geocentric distance (kilometers) */
);

int ias_geo_convert_cart2sph
(
    const IAS_VECTOR *vec, /* I: Vector containing Cartesian coords */ 
    double *lat,           /* O: Latitude in spherical coordinates */
    double *longs,         /* O: Longitude in spherical coordinates */
    double *radius         /* O: Distance from origin to the point */
);

int ias_geo_convert_deg2dms
(
    double deg,         /* I: Angle in seconds, minutes, or degrees */
    double *dms,        /* O: Angle converted to DMS */
    const char *check   /* I: Angle usage type (LAT, LON, or DEGREES) */
);

int ias_geo_convert_dms2deg
(
    double angle_dms,     /* I: Angle in DMS (DDDMMMSSS) format */
    double *angle_degrees,/* O: Angle in decimal degrees */
    const char *type      /* I: Angle usage type (LAT, LON, or DEGREES) */
);

int ias_geo_convert_geocentric_height_to_geodetic
(
    double latc,          /* I: Geocentric latitude (radians) */
    double radius,        /* I: Radius of the point (meters) */
    const IAS_EARTH_CHARACTERISTICS *earth, /* I: earth constants */
    double *latd,         /* O: Geodetic latitude (radians) */
    double *height        /* O: Height of the point (meters) */
);

void ias_geo_convert_geod2cart
(
    double latitude,    /* I: Lat of geodetic coordinates in radians */
    double longitude,   /* I: Long of geodetic coordinates in radians*/
    double height,      /* I: Height (elevation) of geodetic coord in meters */
    double semimajor,   /* I: Reference ellipsoid semi-major axis in meters */
    double flattening,  /* I: Flattening of the ellipsoid 
                           (semimajor-semiminor)/semimajor */
    IAS_VECTOR *cart    /* O: Cartesian vector for the coord */
);

int ias_geo_convert_sensor_los_to_spacecraft
(
    double oli2acs[3][3],          /* I: Sensor to attitude control reference */
    IAS_ACQUISITION_TYPE acq_type, /* I: Image acquisition type */
    const IAS_VECTOR *satpos,      /* I: Satellite position (meters) */
    const IAS_VECTOR *satvel,      /* I: Satellite velocity (m/s) */
    const IAS_VECTOR *sensor_los,  /* I: LOS to target vector (sensor coord)*/
    double roll,                   /* I: Corrected roll (radians) */
    double pitch,                  /* I: Corrected pitch (radians) */
    double yaw,                    /* I: Corrected yaw (radians) */
    double orb2ecf[3][3],          /* I: Orbit to ECEF transformation */
    double attpert[3][3],          /* I: Attitude perturbation matrix */
    IAS_VECTOR *pert_los,          /* O: Perturbed LOS (OCS-orbital coord) */
    IAS_VECTOR *new_los            /* O: New line of sight (ECEF coord) */
);

void ias_geo_convert_sph2cart
(
    double latp,     /* I: Latitude in spherical coordinates */
    double longp,    /* I: Longitude in spherical coordinates */
    double radius,   /* I: Distance from origin to the point */
    IAS_VECTOR *vec  /* O: Vector containing Cartesian coords */ 
);

int ias_geo_convert_utc2times
(
    double ut1_utc,     /* I: UT1-UTC, in seconds, due to variation of Earth's 
                              spin rate */
    const double ephem_time[3],  /* I: UTC Ephemeris time (year, doy and sod) */
    double *ut1,        /* O: Univeral Time (UT1) Time */
    double *tdb,        /* O: Barycentric Dynamical Time (TDB) */
    double *tt          /* O: Terrestrial Time (TT) */
);

void ias_geo_correct_for_center_of_mass
(
   double cm2oli[3],        /* I: sensor to center-of-mass to OLI vector */
   double orb2ecf[3][3],    /* I: Orbit to ECF transformation */
   double attpert[3][3],    /* I: Attitude perturbation matrix */
   const IAS_VECTOR *npos,  /* I: ECF position vector */
   IAS_VECTOR *cpos         /* O: ECF position vector adjusted for CM */
);

int ias_geo_correct_for_light_travel_time
(
   const IAS_VECTOR *satpos,    /* I: Satellite position (meters) */
   const IAS_EARTH_CHARACTERISTICS *earth, /* I: Earth parameters */
   const IAS_VECTOR *itarvec,   /* I: Target vector to get adjusted */
   IAS_VECTOR *ltarvec,         /* O: Target vector adjusted for LLT */
   double *tarlat,              /* O: Target latitude */
   double *tarlong,             /* O: Target longitude */
   double *tarrad               /* O: Radius of the target */
);

int ias_geo_correct_for_velocity_aberration
(
   const IAS_VECTOR *satpos,    /* I: Satellite position (meters) */
   const IAS_VECTOR *satvel,    /* I: Satellite velocity (meters/sec) */
   IAS_ACQUISITION_TYPE acq_type, /* I: Image acquisition type */
   const IAS_EARTH_CHARACTERISTICS *earth, /* I: Earth parameters */
   const IAS_VECTOR *clos,      /* I: Input LOS vector */ 
   IAS_VECTOR *vlos             /* O: New LOS vector adjusted for aberration */
);

/* defined in ias_geo_transformation_matrix.c */
int  ias_geo_create_transformation_matrix
(
    const IAS_VECTOR *satpos, /* I: pointer to sat. position vec */
    const IAS_VECTOR *satvel, /* I: pointer to sat. velocity vec */
    double eci2ob[3][3]  /* O: transformation matrix from ECI to orbit system */
);
   
int ias_geo_ecef2eci
(
    double xp,  /* I: Earth's true pole offset from mean pole, in arc second */
    double yp,  /* I: Earth's true pole offset from mean pole in arc second */
    double ut1_utc, /* I: UT1-UTC, in seconds, due to variation of Earth's 
                          spin rate */
    const IAS_VECTOR *craft_pos, /* I: Satellite position in ECR */
    const IAS_VECTOR *craft_vel, /* I: Satellite velocity in ECR */
    const double ephem_time[3],  /* I: UTC Ephemeris time (year, doy and sod) */
    IAS_VECTOR *ic_satpos,       /* O: Satellite position in ECI */
    IAS_VECTOR *ic_satvel        /* O: Satellite velocity in ECI */
);

int ias_geo_eci2ecef
(
    double xp, /* I: Earth's true pole offset from mean pole, in arc second */
    double yp, /* I: Earth's true pole offset from mean pole, in arc second */
    double ut1_utc, /* I: UT1-UTC, in seconds, due to variation of Earth's 
                          spin rate */
    const IAS_VECTOR *craft_pos, /* I: Satellite position in ECI */
    const IAS_VECTOR *craft_vel, /* I: Satellite velocity in ECI */
    const double ephem_time[3],  /* I: UTC Ephemeris time (year, doy and sod) */
    IAS_VECTOR *fe_satpos,       /* O: Satellite position in ECR */
    IAS_VECTOR *fe_satvel        /* O: Satellite velocity in ECR */
);

int ias_geo_extract_window
(
    int image_nl,           /* I: # of lines in the 1G image */
    int image_ns,           /* I: # of samples in the 1G image */
    int sca,                /* I: SCA number to extract data */
    IAS_DATA_TYPE dtype,    /* I: data type of the band */
    double pred_line,       /* I: center line of window to extract */
    double pred_samp,       /* I: center sample of window to extract */
    const int win_size[2],  /* I: size of window to extract */
    L1G_BAND_IO *l1g_band,  /* I: L1G_BAND_IO info structure for the image */
    float *img_buf,         /* O: Buffer read from 1G image */
    int *upper_line,        /* O: The upper line in the image for the chip */
    int *left_sample        /* O: The left sample in the image for the chip */
);

void ias_geo_find_ariesha 
(
    double tjd,          /* I: full julian day */
    double seconds,      /* I: seconds of the day */
    double *gha          /* O: Greenwich hour angle */
);


void ias_geo_find_deg
(
    double angle,  /* I: Angle in total degrees */
    int  *degree   /* O: Degree portion of the angle */
);


int ias_geo_find_earth_radius
(
    double latc,       /* I: Lat to find the radius at (rad) */
    const IAS_EARTH_CHARACTERISTICS *earth, /* I: earth constants */
    double *radius     /* O: Radius of earth at given lat (m) */
);

void ias_geo_find_min 
(
    double angle,       /* I: Angle in total degrees */
    int  *minute        /* O: Minute portion of the angle */
);

int ias_geo_find_mjdcoords 
(
    double mjd,        /* I: mjd value to search for (days) */
    IAS_CPF *cpf,      /* I: info from Calibration Parameter File */
    double *x,         /* O: x shift pole wander in are/sec */
    double *y,         /* O: y shift pole wander in are/sec */
    double *ut1_utc    /* O: UT1-UTC, in seconds, due to variation of Earth's 
                             spin rate */

);

void ias_geo_find_sec 
(
    double angle,       /* I: Angle in total degrees */
    double *second      /* O: Second portion of the angle */
);

int ias_geo_find_target_position
(
    const IAS_VECTOR *satpos, /* I: Satellite location vector */
    const IAS_VECTOR *losv,   /* I: Line of sight to target vector */
    const IAS_EARTH_CHARACTERISTICS *earth, /* I: Earth parameters */
    double tarelev,         /* I: Elevation of target above the ellipsoid */
    IAS_VECTOR *tarvec,     /* O: Target vector */
    double *tarlat,         /* O: Target latitude */
    double *tarlong,        /* O: Target longitude */
    double *tarrad          /* O: Radius of the target */
);

int ias_geo_get_units 
(
    const char *unit_name,   /* I: Units name */
    int *unit_num            /* O: Units number */
);

int ias_geo_does_cross_180
(
    int unit,             /* I: The angular unit of the angles passed in */
    double const corner_longitudes[4]
                          /* I: The longitude for each corner of the scene */
);

int ias_geo_add_once_around
(
    int unit,        /* I: The angular unit to use to interpret lon */
    double *lon      /* I/O: The angle to be adjusted */
);


void ias_geo_lagrange_interpolate
( 
    const double *seconds_from_ref, /* I: Array of n_pts reference times */
    const IAS_VECTOR *position, /* I: Array of n_pts position vectors */
    const IAS_VECTOR *velocity, /* I: Array of n_pts velocity vectors */
    int n_pts,           /* I: Number of points to use in interpolation */
    double delta_time,   /* I: Delta time from the reference time */
    IAS_VECTOR *interpolated_position,
                         /* O: New satellite position at delta_time */
    IAS_VECTOR *interpolated_velocity
                         /* O: New satellite velocity at delta_time */
);

int ias_geo_report_proj_err 
(
    int err              /* I: Error returned from the GCTP call */
);

int ias_geo_transform_ecef2j2k
(
    double xp, /* I: Earth's true pole offset from mean pole, in arc second */
    double yp, /* I: Earth's true pole offset from mean pole, in arc second */
    double ut1_utc, /* I: UT1-UTC, in seconds, due to variation of Earth's 
                          spin rate */
    const IAS_VECTOR *craft_pos, /* I: Satellite position in ECEF */
    const IAS_VECTOR *craft_vel, /* I: Satellite velocity in ECEF */
    const double ephem_time[3],  /* I: UTC Ephemeris time (year, doy and sod) */
    IAS_VECTOR *eci_satpos,      /* O: Satellite position in ECI */
    IAS_VECTOR *eci_satvel       /* O: Satellite velocity in ECI */
);

void ias_geo_transform_nutation_mod2tod
(
    const IAS_VECTOR *r_old, /* I: coordinates (x, y, z) in the mean-of-date
                                   system */
    double jd_tdb,     /* I: Julian date (Barycentric) for conversion */
    IAS_VECTOR *r_new  /* O: coordinates in the true-of-date equator and
                             equinox sys. */
);

void ias_geo_transform_nutation_tod2mod
(
    const IAS_VECTOR *r_old,/* I: coordinates (x, y, z) in the true-of-date
                                  system */
    double jd_tdb,     /* I: Julian date (Barycentric) for conversion */
    IAS_VECTOR *r_new  /* O: coordinates in the mean-of-date system */
);

void ias_geo_transform_polar_motion_true_pole_to_mean
(
    const IAS_VECTOR *r_old, /* I: coordinates (x, y, z) in the old system */
    double xp,        /* I: true pole position in the mean pole coords system, 
                         x-axis pointing along Greenwich meridian; in arc 
                         seconds */
    double yp,        /* I: true pole position in the mean pole coords system, 
                         y-axis pointing along west 90 degree meridian; in arc 
                         seconds */
    double jd_tdb,    /* I: Julian date (Barycentric) */
    IAS_VECTOR *r_new /* O: coordinates in the new system */
);

void ias_geo_transform_polar_motion_mean_pole_to_true
(
    const IAS_VECTOR *r_old, /* I: coordinates (x, y, z) in the old system */
    double xp,    /* I: mean pole position in the true pole coords system, 
                     x-axis pointing along Greenwich meridian; in arc seconds */
    double yp,    /* I: mean pole position in the true pole coords system, 
                     y-axis pointing along west 90 degree meridian; in arc 
                     seconds */
    double jd_tdb,/* I: Julian date (Barycentric) */
    IAS_VECTOR *r_new /* O: coordinates in the new system */
);

int ias_geo_transform_precession_j2k2mod
(
    const IAS_VECTOR *r_old,/* I: coordinates (x, y, z) in the J2000.0 system */
    double jd_tdb,    /* I: Julian date (Barycentric) for conversion */
    IAS_VECTOR *r_new /* O: coordinates in the mean-of-date equator and equinox 
                         sys. */
);

int ias_geo_transform_precession_mod2j2k
(
    const IAS_VECTOR *r_old,/* I: coordinates (x, y, z) in the mean-of-date
                                  system */
    double jd_tdb,    /* I: Julian date (Barycentric) for conversion */
    IAS_VECTOR *r_new /* O: coordinates in the J2000.0 system */
);

int ias_geo_transform_projection 
(
    int inproj,             /* I: Input projection code */
    int inunit,             /* I: Input projection units code */
    int inzone,             /* I: Input projection zone code */
    const double *inparm,   /* I: Array of 15 projection parameters--input */
    int inspheroid,         /* I: Input spheroid code */
    int outproj,            /* I: Output projection code */
    int outunit,            /* I: Output projection units code */
    int outzone,            /* I: Output projection zone code */
    const double *outparm,  /* I: Array of 15 projection parameters--output */
    int outspheroid,        /* I: Output spheroid code */
    double inx,             /* I: Input X projection coordinate */
    double iny,             /* I: Input Y projection coordinate */
    double *outx,           /* O: Output X projection coordinate */
    double *outy            /* O: Output Y projection coordinate */
);

int ias_geo_get_sidereal_time
(
    double jd_ut1,        /* I: UT1 Julian date of ephemeris time */
    double jd_tt,         /* I: TT Julian date of ephemeris time */
    double *gast          /* O: Greenwich apparent sidereal time, in rad */
);

int ias_geo_transform_sidereal_eci2ecef
(
    const IAS_VECTOR *craft_pos, /* I: Satellite position in ECI */ 
    const IAS_VECTOR *craft_vel, /* I: Satellite velocity in ECI */
    double ut1_utc, /* I: UT1-UTC, in seconds, due to variation of Earth's 
                          spin rate */
    const double ephem_time[3],  /* I: UTC Ephemeris time (year, doy and sod) */
    IAS_VECTOR *ecfe_satpos, /* O: Satellite position in ECEF */
    IAS_VECTOR *ecfe_satvel  /* O: Satellite velocity in ECEF */
);

int ias_geo_transform_sidereal_ecef2eci
(
    const IAS_VECTOR *craft_pos, /* I: Satellite position in ECEF */ 
    const IAS_VECTOR *craft_vel, /* I: Satellite velocity in ECEF */
    double ut1_utc,         /* I: UT1-UTC, in seconds, due to variation of
                                  Earth's spin rate */
    const double ephem_time[3],  /* I: UTC Ephemeris time (year, doy and sod) */
    IAS_VECTOR *eci_satpos, /* O: Satellite position in ECI */
    IAS_VECTOR *eci_satvel  /* O: Satellite velocity in ECI */
);

int ias_geo_transform_tod2j2k
(
    double ut1_utc,              /* I: UT1-UTC, in seconds, due to variation
                                       of Earth's spin rate */
    const IAS_VECTOR *ecitod_pos,/* I: Satellite position in ECITOD */
    const double ephem_time[3],  /* I: UTC Ephemeris time (year, doy and sod) */
    IAS_VECTOR *ecij2k_pos       /* O: Satellite position in ECIJ2K */
);

int ias_geo_initialize_novas();

int ias_geo_shutdown_novas();

int ias_geo_get_solar_position_vector
(
    double julian_date,         /* I: Julian date of acqusition */
    IAS_VECTOR *solar_position  /* O: Solar position vector in km */
);

void ias_geo_convert_rpy_to_matrix
(
    const IAS_VECTOR *rpy,/* I: attitude vector to convert */
    double matrix[3][3]   /* O: resulting matrix */
);

IAS_GEO_PROJ_TRANSFORMATION *ias_geo_create_proj_transformation
(
    const IAS_PROJECTION *source_projection, /* I: source projection */
    const IAS_PROJECTION *target_projection  /* I: target projection */
);

void ias_geo_destroy_proj_transformation
(
    IAS_GEO_PROJ_TRANSFORMATION *trans
);

void ias_geo_only_allow_threadsafe_transforms();

int ias_geo_transform_coordinate
(
    const IAS_GEO_PROJ_TRANSFORMATION *trans, /* I: transformation to use */
    double inx,             /* I: Input X projection coordinate */
    double iny,             /* I: Input Y projection coordinate */
    double *outx,           /* O: Output X projection coordinate */
    double *outy            /* O: Output Y projection coordinate */
);

void ias_geo_set_projection
(
    int proj_code,          /* I: input projection code */
    int zone,               /* I: input zone */
    int units,              /* I: input units */
    int spheroid,           /* I: input spheroid */
    const double *parms,    /* I: input projection parameters */
    IAS_PROJECTION *proj    /* I: target projection structure */
);

int ias_geo_transform_j2k2tod
(
    double ut1_utc,              /* I: UT1-UTC, in seconds, due to variation
                                       of Earth's spin rate */
    const IAS_VECTOR *ecij2k_pos,/* I: Satellite position in ECIJ2K */
    const double ephem_time[3],  /* I: UTC Ephemeris time (year, doy and sod) */
    IAS_VECTOR *ecitod_pos       /* O: Satellite position in ECITOD */
);

#endif
