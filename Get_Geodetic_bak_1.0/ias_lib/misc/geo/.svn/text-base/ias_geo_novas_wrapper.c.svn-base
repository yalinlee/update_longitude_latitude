/***************************************************************************
NAME: ias_geo_novas_wrapper

PURPOSE: This module is a wrapper for any calls that need to be made to the
    NOVAS library routines.  The ias_geo_initialize_novas should be the first
    routine called in this module and the ias_geo_shutdown_novas called to
    free up the resources allocated before an application exits.
    
RETURNS: SUCCESS or ERROR

REFERENCES:
    Bretagnon, P. and Simon, J.L. (1986).  Planetary Programs and
        Tables from -4000 to + 2800. (Richmond, VA: Willmann-Bell).


NOTES:
    1.  This routine makes calls to functions within the NAVAL Observatory
        Vector Astrometry (NOVAS) library.  The library is used by
        applications to process information contained in the Jet Propulsion
        Laboratory (JPL) planetary ephemeris dataset.

    2.  NOVAS function calls return non-zero, generally positive-valued
        status codes on an error condition.

    3.  The quoted accuracy (per reference) is

             2.0 + 0.03 * T^2 arcsec, where T is measured in
             units of 1000 years from the J2000.0 epoch.

    4.  The NOVAS library is not thread-safe.  Users of this module are
        responsible for making sure these routines are not called from more
        than one thread at a time.  If needed, optional locking could be
        added to this module.

    5.  The NOVAS library code quality is disturbingly poor, but we want to
        treat it as a COTS package if at all possible.

****************************************************************************/
#include <math.h>
#include "novas.h"
#include "novascon.h"
#include "solarsystem.h"
#include "eph_manager.h"
#include "ias_const.h"
#include "ias_math.h"
#include "ias_logging.h"
#include "ias_geo.h"
#include "local_novas_wrapper.h"

/* define some constants for the NOVAS library */
#define NOVAS_MAJOR_OBJECT_TYPE 0
#define NOVAS_SUN_OBJECT_NUM   10
#define NOVAS_MOON_OBJECT_NUM  11

/* Keep track of whether the NOVAS library has been initialized */
static int novas_initialized = 0;

/***************************************************************************
NAME: ias_geo_initialize_novas

PURPOSE: Initializes the NOVAS library for use by the routines in this
    module.  This routine needs to be called before any of the other routines
    here are called.

RETURNS:  SUCCESS or ERROR

****************************************************************************/
int ias_geo_initialize_novas()
{
    double julian_date_begin;   /* Beginning Julian date from ephemeris file */
    double julian_date_end;     /* Ending Julian date from ephemeris file */
    short int definitive_ephemeris_number; /* Number of the definitive 
                                   ephemeris file */
    int status;                 /* Function return status */

    /* If this routine has been called previously, consider it an error */
    if (novas_initialized)
    {
        IAS_LOG_ERROR("NOVAS library wrapper already initialized");
        return ERROR;
    }

    /* Confirm the JPLDE421 environment variable is set. */
    if (getenv("JPLDE421") == NULL)
    {
        IAS_LOG_ERROR("JPLDE421 environment variable needed for the NOVAS "
            "library is not set");
        return ERROR;
    }

    /* Open the JPL binary ephemeris file. */
    status = ephem_open(getenv("JPLDE421"), &julian_date_begin,
        &julian_date_end, &definitive_ephemeris_number);
    if (status != 0) 
    {
        if (status == 1)
        {
            IAS_LOG_ERROR("JPL ephemeris file not found");
        }
        else
        {
            IAS_LOG_ERROR("Reading JPL ephemeris file header");
        }
        return ERROR;
    }

    novas_initialized = 1;

    return SUCCESS;
}

/***************************************************************************
NAME: ias_geo_shutdown_novas()

PURPOSE: Frees the resources allocated by the call to initialize the NOVAS
    library.

RETURNS: SUCCESS or ERROR

****************************************************************************/
int ias_geo_shutdown_novas()
{
    if (!novas_initialized)
    {
        IAS_LOG_ERROR("NOVAS library wrapper is not initialized");
        return ERROR;
    }

    /* Close the ephemeris file */
    ephem_close();

    novas_initialized = 0;

    return SUCCESS;
}

/***************************************************************************
NAME: ias_geo_compute_sun_position

PURPOSE: Computes the equatorial spherical coordinates of the Sun with
    respect to the mean equator and equinox on the specified Julian
    date.

RETURNS: SUCCESS or ERROR

NOTES:
    1. ias_geo_initialize_novas needs to be called prior to calling this
       routine.
    2. The units on the right ascension and declination are the customary
       units for those values.

****************************************************************************/
int ias_geo_compute_sun_position 
(
    double julian_date,          /* I: Julian date on TDT or ET time scale */
    double *right_ascension_hrs, /* O: Right ascension referred to mean equator 
                                       and equinox of date (hours) */
    double *declination_deg,     /* O: Declination referred to mean equator and
                                       equinox of date (degrees) */
    double *distance_au          /* O: Geocentric distance (AU) */
)
{
    object sun;    /* NOVAS internal data structures containing information
                      representative of the sun */
    cat_entry *catalog_entry = NULL; /* Needed by make_object but not IAS; for
                      catalog entries for distant objects like stars */
    int error = 0; /* return error value */

    if (!novas_initialized)
    {
        IAS_LOG_ERROR("ias_geo_initialize_novas has not been called");
        return ERROR;
    }

    /* Set up the structure for the Sun "object" */
    memset(&sun, 0, sizeof(object));
    error = make_object(NOVAS_MAJOR_OBJECT_TYPE, NOVAS_SUN_OBJECT_NUM, "Sun", 
                catalog_entry, &sun);
    if (error != NOVAS_SUCCESS)
    {
        IAS_LOG_ERROR("NOVAS make_object() call for Sun failed: code %d", 
            error);
        return ERROR;
    }

    /* Determine the apparent position of the Sun relative to the Earth */
    error = app_planet(julian_date, &sun, NOVAS_REDUCED_ACCURACY, 
                right_ascension_hrs, declination_deg, distance_au);
    if (error != NOVAS_SUCCESS)
    {
        IAS_LOG_ERROR("NOVAS app_planet() call failed:  code %d", error);
        return ERROR;
    }

    return SUCCESS;
}

/***************************************************************************
NAME: ias_geo_get_solar_position_vector

PURPOSE: Determines the solar position vector for a specified acquisition date

RETURNS: SUCCESS or ERROR
****************************************************************************/
int ias_geo_get_solar_position_vector
(
    double julian_date,         /* I: Julian date of acquisition */
    IAS_VECTOR *solar_position  /* O: Solar position vector in km */
)
{
    double position[3];                  /* Solar position vector in AU */
    double right_ascension_hrs;          /* Solar right ascension */
    double declination_deg;              /* Solar declination */
    double distance_au;                  /* Earth-sun distance in AU */
    int status;

    status = ias_geo_compute_sun_position(julian_date, &right_ascension_hrs,
                &declination_deg, &distance_au);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Failed to calculate the sun position");
        return ERROR;
    }

    /* Convert the spherical coordinates (right_ascension, declination,
       and distance) to a "Cartesian" vector (x, y, z).  This backs out
       of the final step in the "app_planet" routine */
    radec2vector(right_ascension_hrs, declination_deg, distance_au, position);

    /* Save the ephemeris information.  By default, NOVAS generates this
       information in terms of AU (or AU/d for the velocity).  We need to
       convert these values to km (or km/s).  Do this by multiplying 
       everything by the NOVAS const AU_KM, the conversion factor between
       km and AU */
    solar_position->x = position[0] * AU_KM;
    solar_position->y = position[1] * AU_KM;
    solar_position->z = position[2] * AU_KM;

    return SUCCESS;
}

/***************************************************************************
NAME: ias_geo_compute_moon_position

PURPOSE: Computes the equatorial spherical coordinates of the Moon with
    respect to the mean equator and equinox on the specified Julian date.

RETURNS: SUCCESS or ERROR

NOTES:
    1. ias_geo_initialize_novas needs to be called prior to calling this
       routine.
    2. The units on the right ascension and declination are the customary
       units for those values.  The distance is converted to kilometers since
       that is more useful for the users. 

****************************************************************************/
int ias_geo_compute_moon_position 
(
    double julian_date,          /* I: Julian ephemeris date */
    double *right_ascension_hrs, /* O: Right ascension referred to mean equator 
                                       and equinox of date (hours) */
    double *declination_deg,     /* O: Declination referred to mean equator and
                                       equinox of date (degrees) */
    double *distance_km          /* O: Geocentric distance (kilometers) */
)
{
    short int error = 0;
    double distance_au;     /* distance in AU */

    object moon; /* Structure containing the object designation for the Moon  */
    cat_entry *catalog_entry = NULL; /* Needed by make_object but not IAS; for
                    catalog entries for distant objects like stars */

    if (!novas_initialized)
    {
        IAS_LOG_ERROR("ias_geo_initialize_novas has not been called");
        return ERROR;
    }

    /* Set up the structure containing the object designation for the Moon.  */
    error = make_object(NOVAS_MAJOR_OBJECT_TYPE, NOVAS_MOON_OBJECT_NUM, "Moon", 
            catalog_entry, &moon);
    if (error != NOVAS_SUCCESS)
    {
        IAS_LOG_ERROR("NOVAS make_object() call for the Moon failed: code %d",
            error);
        return ERROR;
    }

    /* Compute the apparent place of the Moon at the requested Julian date. */
    error = app_planet(julian_date, &moon, NOVAS_REDUCED_ACCURACY, 
                right_ascension_hrs, declination_deg, &distance_au);
    if (error != NOVAS_SUCCESS)
    {
        IAS_LOG_ERROR("NOVAS app_planet() call failed: code %d", error);
        return ERROR;
    }

    /* convert the distance to kilometers using the constant from the NOVAS
       library */
    *distance_km = distance_au * AU_KM;

    return SUCCESS;
}
