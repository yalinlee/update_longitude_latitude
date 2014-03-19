/*************************************************************************
Name: ias_los_model_set_cpf

Purpose: Accepts a CPF structure and a pointer to a model structure to
    copy the CPF values into.

Returns:
    SUCCESS or ERROR

Notes:
    Meant to be used in conjunction with ias_los_model_set_l0r()
    Dependencies:
    ias_los_model_set_cpf calls ias_sensor_set_cpf(). ias_sensor_set_cpf()
    sets CPF info into the model needed by ias_los_model_set_l0r(). Therefore, 
    ias_los_model_set_cpf() must be called before ias_los_model_set_l0r().
**************************************************************************/
#include "logging_channel.h" /* define the debug logging channel */
#include "ias_logging.h"
#include "ias_los_model.h"

static void log_los_cpf_values(IAS_LOS_MODEL *model);

int ias_los_model_set_cpf
(
    IAS_CPF *cpf,                  /* I: CPF structure pointer with values to
                                     set into the model structure */
    IAS_SENSOR_L0R_DATA *l0r_sensor_data,
                                  /* O: Sensor L0R data needed by other funcs */
    IAS_LOS_MODEL *model          /* O: Model structure to populate */
)
{
    int status;                       /* routine return statuses */
    const struct IAS_CPF_EARTH_CONSTANTS *earth_constants; 
                                     /* CPF earth const group */

    /* Set the basic earth constants */
    earth_constants = ias_cpf_get_earth_const(cpf);
    if (!earth_constants)
    {
        IAS_LOG_ERROR("Retrieving CPF earth constants parameters");
        return ERROR;
    }
    model->earth.semi_major_axis = earth_constants->semi_major_axis;
    model->earth.semi_minor_axis = earth_constants->semi_minor_axis;
    model->earth.eccentricity = earth_constants->eccentricity;
    model->earth.earth_angular_velocity 
        = earth_constants->earth_angular_velocity;
    model->earth.speed_of_light = earth_constants->speed_of_light;

    /* Only does something if log level set to IAS_LOG_LEVEL_DEBUG */
    if (IAS_LOG_DEBUG_ENABLED())
        log_los_cpf_values(model);

    /* Set the CPF information that goes into the sensor model. Also grab
       some info that will be needed from the CPF not stored directly in
       the model that will be needed by the set_l0r routine later. */
    status = ias_sensor_set_cpf(cpf, l0r_sensor_data, &model->sensor);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Setting CPF values into model structure");
        return ERROR;
    }

    return SUCCESS;
}




/////////////////////////////////////////////////////////////////////////////////
//add by LYL at 2014/3/6
int ias_los_model_set_cpf_for_MWD
(
    IAS_CPF *cpf,                  /* I: CPF structure pointer with values to
                                     set into the model structure */
    IAS_LOS_MODEL *model          /* O: Model structure to populate */
)
{
    int status;                       /* routine return statuses */
    const struct IAS_CPF_EARTH_CONSTANTS *earth_constants;
                                     /* CPF earth const group */

    /* Set the basic earth constants */
    earth_constants = ias_cpf_get_earth_const(cpf);
    if (!earth_constants)
    {
        IAS_LOG_ERROR("Retrieving CPF earth constants parameters");
        return ERROR;
    }
    model->earth.semi_major_axis = earth_constants->semi_major_axis;
    model->earth.semi_minor_axis = earth_constants->semi_minor_axis;
    model->earth.eccentricity = earth_constants->eccentricity;
    model->earth.earth_angular_velocity
        = earth_constants->earth_angular_velocity;
    model->earth.speed_of_light = earth_constants->speed_of_light;

    /* Only does something if log level set to IAS_LOG_LEVEL_DEBUG */
    if (IAS_LOG_DEBUG_ENABLED())
        log_los_cpf_values(model);

    /* Set the CPF information that goes into the sensor model. Also grab
       some info that will be needed from the CPF not stored directly in
       the model that will be needed by the set_l0r routine later. */
    status = ias_sensor_set_cpf_for_MWD(cpf, &model->sensor);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Setting CPF values into model structure");
        return ERROR;
    }

    return SUCCESS;
}

/*************************************************************************
Name: log_los_cpf_values

Purpose: Prints debugging log data for the CPF source values.

Returns:
    nothing

Notes:
**************************************************************************/
static void log_los_cpf_values
(
    IAS_LOS_MODEL *model        /* I: Model structure */
)
{
    /*== DEBUG LOGGING ======================================================*/
    IAS_LOG_DEBUG("====> SET CPF TO MODEL REPORT <====");
    /*== END DEBUG ==========================================================*/

    /*== DEBUG LOGGING ======================================================*/
    IAS_LOG_DEBUG("UT1UTC correction: %lf", model->earth.ut1_utc_correction);
    IAS_LOG_DEBUG("Pole wander X: %lf", model->earth.pole_wander_x);
    IAS_LOG_DEBUG("Pole wander Y: %lf", model->earth.pole_wander_y);
    IAS_LOG_DEBUG("Semi major axis: %lf", model->earth.semi_major_axis);
    IAS_LOG_DEBUG("Semi minor axis: %lf", model->earth.semi_minor_axis);
    IAS_LOG_DEBUG("Eccentricity: %lf", model->earth.eccentricity);
    IAS_LOG_DEBUG("Angular velocity: %lf", model->earth.earth_angular_velocity);
    IAS_LOG_DEBUG("Speed of light: %lf", model->earth.speed_of_light);
    /*== END DEBUG ==========================================================*/
}
