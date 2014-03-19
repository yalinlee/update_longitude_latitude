/*************************************************************************
Name: ias_sc_model_set_ancillary

Purpose: Accepts ancillary ephemeris and attitude structures (read
    previously from the ancillary data file) and a pointer to a spacecraft
    model structure to copy the ancillary values into.

RETURNS: SUCCESS or ERROR
**************************************************************************/
#include <stdlib.h>
#include "../logging_channel.h" /* define debug logging channel */
#include "ias_logging.h"
#include "ias_ancillary_io.h"
#include "ias_spacecraft_model.h"

static void log_sc_ancillary_values(IAS_SPACECRAFT_MODEL *sc);

int ias_sc_model_set_ancillary
(
    const IAS_ANC_EPHEMERIS_DATA *anc_ephemeris, /* I: Ancillary ephemeris
                                                    pointer */
    const IAS_ANC_ATTITUDE_DATA *anc_attitude,   /* I: Ancillary attitude
                                                    pointer */
    IAS_SPACECRAFT_MODEL *sc                     /* O: The spacecraft model
                                                    containing ancillary
                                                    ephemeris and attitude */
)
{
    IAS_SC_EPHEMERIS_MODEL *model_ephemeris; /* Ephemeris model sub-structure */
    IAS_SC_ATTITUDE_MODEL *model_attitude;   /* Attitude model sub-structure */
    int sample;

    model_ephemeris = &sc->ephemeris;
    model_attitude = &sc->attitude;

    /* Set the ephemeris UTC epoch time */
    model_ephemeris->utc_epoch_time[0] = anc_ephemeris->utc_epoch_time[0];
    model_ephemeris->utc_epoch_time[1] = anc_ephemeris->utc_epoch_time[1];
    model_ephemeris->utc_epoch_time[2] = anc_ephemeris->utc_epoch_time[2];

    /* The nominal sample time is set from a constant */
    model_ephemeris->nominal_sample_time = IAS_EPHEM_SAMPLING_PERIOD;

    /* Set the ephemeris number of samples */
    model_ephemeris->sample_count = anc_ephemeris->number_of_samples;

    /* Allocate memory for the ephemeris sample records */
    model_ephemeris->sample_records
        = malloc(sizeof(*model_ephemeris->sample_records)
                * model_ephemeris->sample_count);
    if (model_ephemeris->sample_records == NULL)
    {
        IAS_LOG_ERROR("Allocating ephemeris sample records for the model");
        return ERROR;
    }

    /* Set the ephemeris sample records values */
    for (sample = 0; sample < model_ephemeris->sample_count; sample++)
    {
        IAS_SC_EPHEMERIS_RECORD *mod_eph;
        const IAS_ANC_EPHEMERIS_RECORD *anc_eph;

        mod_eph = &model_ephemeris->sample_records[sample];
        anc_eph = &anc_ephemeris->records[sample];

        mod_eph->seconds_from_epoch = anc_eph->seconds_from_epoch;

        mod_eph->eci_position.x = anc_eph->eci_position[0];
        mod_eph->eci_position.y = anc_eph->eci_position[1];
        mod_eph->eci_position.z = anc_eph->eci_position[2];

        mod_eph->eci_velocity.x = anc_eph->eci_velocity[0];
        mod_eph->eci_velocity.y = anc_eph->eci_velocity[1];
        mod_eph->eci_velocity.z = anc_eph->eci_velocity[2];

        mod_eph->ecef_position.x = anc_eph->ecef_position[0];
        mod_eph->ecef_position.y = anc_eph->ecef_position[1];
        mod_eph->ecef_position.z = anc_eph->ecef_position[2];

        mod_eph->ecef_velocity.x = anc_eph->ecef_velocity[0];
        mod_eph->ecef_velocity.y = anc_eph->ecef_velocity[1];
        mod_eph->ecef_velocity.z = anc_eph->ecef_velocity[2];

        /* Set the precision ECI/ECEF position and velocity values to the
           original values. These may get modified later. */
        mod_eph->precision_eci_position.x = anc_eph->eci_position[0];
        mod_eph->precision_eci_position.y = anc_eph->eci_position[1];
        mod_eph->precision_eci_position.z = anc_eph->eci_position[2];

        mod_eph->precision_eci_velocity.x = anc_eph->eci_velocity[0];
        mod_eph->precision_eci_velocity.y = anc_eph->eci_velocity[1];
        mod_eph->precision_eci_velocity.z = anc_eph->eci_velocity[2];

        mod_eph->precision_ecef_position.x = anc_eph->ecef_position[0];
        mod_eph->precision_ecef_position.y = anc_eph->ecef_position[1];
        mod_eph->precision_ecef_position.z = anc_eph->ecef_position[2];

        mod_eph->precision_ecef_velocity.x = anc_eph->ecef_velocity[0];
        mod_eph->precision_ecef_velocity.y = anc_eph->ecef_velocity[1];
        mod_eph->precision_ecef_velocity.z = anc_eph->ecef_velocity[2];
    }

    /* Set the attitude UTC epoch time */
    model_attitude->utc_epoch_time[0] = anc_attitude->utc_epoch_time[0];
    model_attitude->utc_epoch_time[1] = anc_attitude->utc_epoch_time[1];
    model_attitude->utc_epoch_time[2] = anc_attitude->utc_epoch_time[2];

    /* The nominal sample time is set from a constant */
    model_attitude->nominal_sample_time = IAS_IRU_SAMPLING_PERIOD;

    /* Set the attitude number of samples */
    model_attitude->sample_count = anc_attitude->number_of_samples;

    /* Allocate memory for the attitude sample records */
    model_attitude->sample_records
        = malloc(sizeof(*model_attitude->sample_records)
                * model_attitude->sample_count);
    if (model_attitude->sample_records == NULL)
    {
        /* No need to free up the ephemeris sample records allocated previously
           in this routine, since before this failure bubbles up to result in
           program termination, the model structure itself is free'd, whereby
           the ephemeris sample records will be free'd. */
        IAS_LOG_ERROR("Allocating attitude sample records for the model");
        return ERROR;
    }

    /* Set the attitude sample records values */
    for (sample = 0; sample < model_attitude->sample_count; sample++)
    {
        IAS_SC_ATTITUDE_RECORD *mod_att;
        const IAS_ANC_ATTITUDE_RECORD *anc_att;

        mod_att = &model_attitude->sample_records[sample];
        anc_att = &anc_attitude->records[sample];

        mod_att->seconds_from_epoch = anc_att->seconds_from_epoch;

        mod_att->satellite_attitude.x = anc_att->roll;
        mod_att->satellite_attitude.y = anc_att->pitch;
        mod_att->satellite_attitude.z = anc_att->yaw;

        /* Set the precision attitude values to the original values.
           These may get modified later. */
        mod_att->precision_attitude.x = anc_att->roll;
        mod_att->precision_attitude.y = anc_att->pitch;
        mod_att->precision_attitude.z = anc_att->yaw;
    }

    /* Only does something if log level set to IAS_LOG_LEVEL_DEBUG */
    if (IAS_LOG_DEBUG_ENABLED())
        log_sc_ancillary_values(sc);

    return SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////
//added by LYL at 2014/3/5
int ias_sc_model_set_ancillary_ephemeris
(
    const IAS_ANC_EPHEMERIS_DATA *anc_ephemeris, /* I: Ancillary ephemeris
                                                    pointer */
    IAS_SPACECRAFT_MODEL *sc                     /* O: The spacecraft model
                                                    containing ancillary
                                                    ephemeris and attitude */
)
{
    IAS_SC_EPHEMERIS_MODEL *model_ephemeris; /* Ephemeris model sub-structure */
    int sample;

    model_ephemeris = &sc->ephemeris;

    /* Set the ephemeris UTC epoch time */
    model_ephemeris->utc_epoch_time[0] = anc_ephemeris->utc_epoch_time[0];
    model_ephemeris->utc_epoch_time[1] = anc_ephemeris->utc_epoch_time[1];
    model_ephemeris->utc_epoch_time[2] = anc_ephemeris->utc_epoch_time[2];

    /* The nominal sample time is set from a constant */
    model_ephemeris->nominal_sample_time = IAS_EPHEM_SAMPLING_PERIOD;

    /* Set the ephemeris number of samples */
    model_ephemeris->sample_count = anc_ephemeris->number_of_samples;

    /* Allocate memory for the ephemeris sample records */
    model_ephemeris->sample_records
        = malloc(sizeof(*model_ephemeris->sample_records)
                * model_ephemeris->sample_count);
    if (model_ephemeris->sample_records == NULL)
    {
        IAS_LOG_ERROR("Allocating ephemeris sample records for the model");
        return ERROR;
    }

    /* Set the ephemeris sample records values */
    for (sample = 0; sample < model_ephemeris->sample_count; sample++)
    {
        IAS_SC_EPHEMERIS_RECORD *mod_eph;
        const IAS_ANC_EPHEMERIS_RECORD *anc_eph;

        mod_eph = &model_ephemeris->sample_records[sample];
        anc_eph = &anc_ephemeris->records[sample];

        mod_eph->seconds_from_epoch = anc_eph->seconds_from_epoch;

        mod_eph->eci_position.x = anc_eph->eci_position[0];
        mod_eph->eci_position.y = anc_eph->eci_position[1];
        mod_eph->eci_position.z = anc_eph->eci_position[2];

        mod_eph->eci_velocity.x = anc_eph->eci_velocity[0];
        mod_eph->eci_velocity.y = anc_eph->eci_velocity[1];
        mod_eph->eci_velocity.z = anc_eph->eci_velocity[2];

        mod_eph->ecef_position.x = anc_eph->ecef_position[0];
        mod_eph->ecef_position.y = anc_eph->ecef_position[1];
        mod_eph->ecef_position.z = anc_eph->ecef_position[2];

        mod_eph->ecef_velocity.x = anc_eph->ecef_velocity[0];
        mod_eph->ecef_velocity.y = anc_eph->ecef_velocity[1];
        mod_eph->ecef_velocity.z = anc_eph->ecef_velocity[2];

        /* Set the precision ECI/ECEF position and velocity values to the
           original values. These may get modified later. */
        mod_eph->precision_eci_position.x = anc_eph->eci_position[0];
        mod_eph->precision_eci_position.y = anc_eph->eci_position[1];
        mod_eph->precision_eci_position.z = anc_eph->eci_position[2];

        mod_eph->precision_eci_velocity.x = anc_eph->eci_velocity[0];
        mod_eph->precision_eci_velocity.y = anc_eph->eci_velocity[1];
        mod_eph->precision_eci_velocity.z = anc_eph->eci_velocity[2];

        mod_eph->precision_ecef_position.x = anc_eph->ecef_position[0];
        mod_eph->precision_ecef_position.y = anc_eph->ecef_position[1];
        mod_eph->precision_ecef_position.z = anc_eph->ecef_position[2];

        mod_eph->precision_ecef_velocity.x = anc_eph->ecef_velocity[0];
        mod_eph->precision_ecef_velocity.y = anc_eph->ecef_velocity[1];
        mod_eph->precision_ecef_velocity.z = anc_eph->ecef_velocity[2];
    }

    /* Only does something if log level set to IAS_LOG_LEVEL_DEBUG */
//    if (IAS_LOG_DEBUG_ENABLED())
//        log_sc_ancillary_values(sc);

    return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////




/*************************************************************************
Name: log_sc_ancillary_values

Purpose: Prints debugging log data for the ancillary source values.

RETURNS: nothing
**************************************************************************/
static void log_sc_ancillary_values
(
    IAS_SPACECRAFT_MODEL *sc
)
{
    int sample;

    /*== DEBUG LOGGING ======================================================*/
    IAS_LOG_DEBUG("====> SET ANCILLARY DATA TO MODEL REPORT <====");

    IAS_LOG_DEBUG("Ephemeris year: %lf", sc->ephemeris.utc_epoch_time[0]);
    IAS_LOG_DEBUG("Ephemeris day of year: %lf",
            sc->ephemeris.utc_epoch_time[1]);
    IAS_LOG_DEBUG("Ephemeris second of day: %lf",
            sc->ephemeris.utc_epoch_time[2]);
    IAS_LOG_DEBUG("Ephemeris nominal sample time: %lf",
            sc->ephemeris.nominal_sample_time);
    IAS_LOG_DEBUG("Number of ephemeris samples: %d",
            sc->ephemeris.sample_count);
    for (sample = 0; sample < 3; sample++)
    {
        IAS_LOG_DEBUG("Ephemeris seconds from epoch for sample %d: %lf",
                sample+1,
                sc->ephemeris.sample_records[sample].seconds_from_epoch);
        IAS_LOG_DEBUG("Ephemeris ECI X/Y/Z position for sample %d: "
                "%lf / %lf / %lf",
                sample+1, sc->ephemeris.sample_records[sample].eci_position.x,
                sc->ephemeris.sample_records[sample].eci_position.y,
                sc->ephemeris.sample_records[sample].eci_position.z);
        IAS_LOG_DEBUG("Ephemeris ECI X/Y/Z velocity for sample %d: "
                "%lf / %lf / %lf",
                sample+1, sc->ephemeris.sample_records[sample].eci_velocity.x,
                sc->ephemeris.sample_records[sample].eci_velocity.y,
                sc->ephemeris.sample_records[sample].eci_velocity.z);
        IAS_LOG_DEBUG("Ephemeris ECEF X/Y/Z position for sample %d: "
                "%lf / %lf / %lf", sample+1,
                sc->ephemeris.sample_records[sample].ecef_position.x,
                sc->ephemeris.sample_records[sample].ecef_position.y,
                sc->ephemeris.sample_records[sample].ecef_position.z);
        IAS_LOG_DEBUG("Ephemeris ECEF X/Y/Z velocity for sample %d: "
                "%lf / %lf / %lf", sample+1,
                sc->ephemeris.sample_records[sample].ecef_velocity.x,
                sc->ephemeris.sample_records[sample].ecef_velocity.y,
                sc->ephemeris.sample_records[sample].ecef_velocity.z);
    }

    IAS_LOG_DEBUG("Attitude year: %lf", sc->attitude.utc_epoch_time[0]);
    IAS_LOG_DEBUG("Attitude day of year: %lf",
            sc->attitude.utc_epoch_time[1]);
    IAS_LOG_DEBUG("Attitude second of day: %lf",
            sc->attitude.utc_epoch_time[2]);
    IAS_LOG_DEBUG("Attitude nominal sample time: %lf",
            sc->attitude.nominal_sample_time);
    IAS_LOG_DEBUG("Number of attitude samples: %d", sc->attitude.sample_count);
    for (sample = 0; sample < 3; sample++)
    {
        IAS_LOG_DEBUG("Attitude seconds from epoch for sample %d: %lf",
                sample+1,
                sc->attitude.sample_records[sample].seconds_from_epoch);
        IAS_LOG_DEBUG("Satellite attitude roll/pitch/yaw for sample %d: "
                "%lf / %lf / %lf", sample+1,
                sc->attitude.sample_records[sample].satellite_attitude.x,
                sc->attitude.sample_records[sample].satellite_attitude.y,
                sc->attitude.sample_records[sample].satellite_attitude.z);
    }
    /*== END DEBUG ==========================================================*/
}
