/*************************************************************************
Name: ias_los_model_apply_precision_params

Purpose: Executes the precision correction routines for the model attitude
    and ephemeris. Applies the precision parameters (stored in the model) to
    the attitude and ephemeris sample records (in the model).

Returns: SUCCESS or ERROR

**************************************************************************/
#include <stdio.h>
#include "logging_channel.h" /* define the debug logging channel */
#include "ias_logging.h"
#include "ias_los_model.h"

static void log_los_precision_values(IAS_SPACECRAFT_MODEL *model);

int ias_los_model_apply_precision_params
(
    IAS_SC_PRECISION_MODEL *precision_params, /* I: Precision params to apply
        or NULL to use parameters stored in the model. If precision_params
        is all zeros, essentially a no-op. */
    IAS_LOS_MODEL *model                      /* O: Updated att/eph with prec */
)
{
    IAS_SENSOR_MODEL *sensor;
    IAS_SENSOR_ID sensor_to_base_image_epoch_on = IAS_OLI;
    int i;                      /* Generic looping counter */
    int status;                 /* Routine return status */
    double *image_epoch = NULL;
    int found_image_epoch = 0;
    IAS_SC_PRECISION_MODEL *params;
    sensor = &model->sensor;

    /* Determine what sensor(s) present */
    for (i = 0; i < IAS_MAX_SENSORS; i++)
    {
        /* Setting the sensor to use in this way sets it to IAS_OLI first,
           if it is an OLI-only or OLI/TIRS combined image. The sensor to
           base the image epoch time on would only be set to TIRS in the
           case of a TIRS-only collect. */
        if (sensor->sensors[i].sensor_present)
        {
            sensor_to_base_image_epoch_on = sensor->sensors[i].sensor_id;
            break;
        }
    }

    /* Get the image epoch time from the first band for the sensor
       determined above */
    for (i = 0; i < sensor->band_count; i++)
    {
        IAS_SENSOR_BAND_MODEL *band;
        band = &sensor->bands[i];

        if (band->band_present
                && band->sensor->sensor_id == sensor_to_base_image_epoch_on)
        {
            image_epoch = band->utc_epoch_time;
            found_image_epoch = 1;
            break;
        }
    }

    if (!found_image_epoch)
    {
        IAS_LOG_ERROR("Image epoch time could not be determined");
        return ERROR;
    }

    if (precision_params == NULL)
        params = &model->spacecraft.precision;
    else
        params = precision_params;

    /* Apply precision parameters to correct the attitude */
    status = ias_sc_model_correct_attitude(image_epoch, params,
            &model->spacecraft.attitude);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Correcting and converting attitude samples");
        return ERROR;
    }

    /* Apply precision paramters to correct ephemeris and also convert
       ephemeris from ECEF to ECI */
    status = ias_sc_model_correct_ephem_convert_ecef2eci(
            model->earth.pole_wander_x, model->earth.pole_wander_y,
            model->earth.ut1_utc_correction, image_epoch, params,
            &model->spacecraft.ephemeris);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Correcting and converting ephemeris samples");
        return ERROR;
    }

    /* Only does something if log level set to IAS_LOG_LEVEL_DEBUG */
    if (IAS_LOG_DEBUG_ENABLED())
        log_los_precision_values(&model->spacecraft);

    return SUCCESS;
}

/*************************************************************************
Name: log_los_precision_values

Purpose: Prints debugging log data for the precision spacecraft values.

Returns:
    nothing

Notes:
**************************************************************************/
static void log_los_precision_values
(
    IAS_SPACECRAFT_MODEL *sc          /* I: Spacecraft model structure */
)
{
    int sample;

    /*== DEBUG LOGGING ======================================================*/
    IAS_LOG_DEBUG("====> APPLY PRECISION PARAMS TO MODEL REPORT <====");
    /*== END DEBUG ==========================================================*/

    /*== DEBUG LOGGING ======================================================*/
    for (sample = 0; sample < sc->ephemeris.sample_count; sample++)
    {
        IAS_LOG_DEBUG("Ephemeris Precision ECI X/Y/Z position for sample %d: "
                "%lf / %lf / %lf", sample+1,
                sc->ephemeris.sample_records[sample].precision_eci_position.x,
                sc->ephemeris.sample_records[sample].precision_eci_position.y,
                sc->ephemeris.sample_records[sample].precision_eci_position.z);
        IAS_LOG_DEBUG("Ephemeris Precision ECI X/Y/Z velocity for sample %d: "
                "%lf / %lf / %lf", sample+1,
                sc->ephemeris.sample_records[sample].precision_eci_velocity.x,
                sc->ephemeris.sample_records[sample].precision_eci_velocity.y,
                sc->ephemeris.sample_records[sample].precision_eci_velocity.z);
        IAS_LOG_DEBUG("Ephemeris Precision ECEF X/Y/Z position for sample %d: "
                "%lf / %lf / %lf", sample+1,
                sc->ephemeris.sample_records[sample].precision_ecef_position.x,
                sc->ephemeris.sample_records[sample].precision_ecef_position.y,
                sc->ephemeris.sample_records[sample].precision_ecef_position.z);
        IAS_LOG_DEBUG("Ephemeris Precision ECEF X/Y/Z velocity for sample %d: "
                "%lf / %lf / %lf", sample+1,
                sc->ephemeris.sample_records[sample].precision_ecef_velocity.x,
                sc->ephemeris.sample_records[sample].precision_ecef_velocity.y,
                sc->ephemeris.sample_records[sample].precision_ecef_velocity.z);
    }

    for (sample = 0; sample < sc->attitude.sample_count; sample++)
    {
        IAS_LOG_DEBUG("Satellite Precision attitude roll/pitch/yaw for sample "
                "%d: %lf / %lf / %lf", sample+1,
                sc->attitude.sample_records[sample].precision_attitude.x,
                sc->attitude.sample_records[sample].precision_attitude.y,
                sc->attitude.sample_records[sample].precision_attitude.z);
    }
    /*== END DEBUG ==========================================================*/
}
