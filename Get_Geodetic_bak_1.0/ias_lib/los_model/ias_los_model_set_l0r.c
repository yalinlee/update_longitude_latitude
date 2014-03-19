/*************************************************************************
Name: ias_los_model_set_l0r

Purpose: Set the L0R values into the model structure. Most of this is done
    by a sensor model library routine.

Returns:
    SUCCESS or ERROR

Notes:
    Meant to be used in conjunction with ias_los_model_set_cpf()
    Dependencies:
    ias_los_model_set_l0r() calls ias_sensor_set_l0r(). ias_sensor_set_l0r()
    uses info from the model set into it previously by ias_sensor_set_cpf().
    Therefore, ias_los_model_set_l0r() must be called after
    ias_los_model_set_cpf().
**************************************************************************/
#include "ias_geo.h"
#include "logging_channel.h" /* define debug logging channel */
#include "ias_logging.h"
#include "ias_los_model.h"

static void log_l0r_values(IAS_LOS_MODEL *model);

int ias_los_model_set_l0r
(
    IAS_CPF *cpf,                        /* I: CPF structure pointer for things
                                           that are needed but not stored in
                                           the model */
    IAS_LOS_MODEL_L0R_DATA *l0r_data,   /* I: L0R data to create a model */
    IAS_LOS_MODEL *model                /* O: Model structure to populate */
)
{
    int status;                         /* General routine return status */
    double mjd;                         /* Modified Julian date */
    int index;
    int band_index = -1;

    /* Set the WRS path/row into the top-level of the model structure */
    model->wrs_path = l0r_data->wrs_path;
    model->wrs_row = l0r_data->wrs_row;

    /* Set the L0R image information that is sensor-specific into model */
    status = ias_sensor_set_l0r(cpf, &l0r_data->l0r_sensor_data,
                                &model->sensor);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Setting sensor L0R data into model");
        return ERROR;
    }

    /* Find a band that is present */
    for (index = 0; index < model->sensor.band_count; index++)
    {
        if (model->sensor.bands[index].band_present)
        {
            band_index = index;
            break;
        }
    }
    if (band_index == -1)
    {
        IAS_LOG_ERROR("No bands found to be present in the model");
        return ERROR;
    }

    /* Get the pole wander X/Y and UT1/UTC conversion */
    status = ias_geo_compute_getmjdcoords(
            model->sensor.bands[band_index].utc_epoch_time,
            cpf, &mjd, &model->earth.pole_wander_x, &model->earth.pole_wander_y,
            &model->earth.ut1_utc_correction);
    if (status == ERROR)
    {
        IAS_LOG_ERROR("Retrieving pole wander X/Y and UT1-UTC conversion");
        return ERROR;
    }

    /* Only does something if log level set to IAS_LOG_LEVEL_DEBUG */
    if (IAS_LOG_DEBUG_ENABLED())
        log_l0r_values(model);

    return SUCCESS;
}

/*************************************************************************
Name: log_l0r_values

Purpose: Prints debugging log data for the L0R source values.

Returns:
    nothing

Notes:
**************************************************************************/
static void log_l0r_values
(
    IAS_LOS_MODEL *model           /* I: Model structure */
)
{
    /*== DEBUG LOGGING ======================================================*/
    IAS_LOG_DEBUG("====> SET L0R TO MODEL REPORT <====");

    IAS_LOG_DEBUG("WRS path: %d", model->wrs_path);
    IAS_LOG_DEBUG("WRS row: %d", model->wrs_row);

    /*== END DEBUG ==========================================================*/
}
