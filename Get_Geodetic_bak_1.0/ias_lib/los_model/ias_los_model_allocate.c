/*************************************************************************
Name: ias_los_model_allocate

Purpose: Allocates the memory needed for a LOS model based on the information
    for the currently configured satellite in the satellite attributes library.

Returns: a pointer to the allocated model, or NULL if an error occurs.

Notes:
    - This routine only allocates memory for the LOS model fields that can
      be derived from the satellite configuration (number of bands, SCAs,
      detectors per sca, etc).  Memory that depends on the ephemeris data
      will wait until the model is initialized.
**************************************************************************/
#include <stdlib.h>
#include "ias_logging.h"
#include "ias_satellite_attributes.h"
#include "ias_los_model.h"

IAS_LOS_MODEL *ias_los_model_allocate()
{
    IAS_LOS_MODEL *model;
    IAS_SENSOR_MODEL *sensor;
    IAS_SATELLITE_ID satellite_id;
    int satellite_number;
    int sensor_count;
    int band_number_list[IAS_MAX_TOTAL_BANDS];
    int band_count;
    int index;

    /* get the satellite id and number */
    satellite_id = ias_sat_attr_get_satellite_id();
    if (satellite_id == ERROR)
    {
        IAS_LOG_ERROR("Retrieving the satellite ID");
        return NULL;
    }
    satellite_number = ias_sat_attr_get_satellite_number();
    if (satellite_number == ERROR)
    {
        IAS_LOG_ERROR("Retrieving the satellite number");
        return NULL;
    }

    /* get the list of normal bands for all sensors */
    if (ias_sat_attr_get_sensor_band_numbers(IAS_MAX_SENSORS, IAS_NORMAL_BAND,
            0, band_number_list, IAS_MAX_TOTAL_BANDS, &band_count) != SUCCESS)
    {
        IAS_LOG_ERROR("Getting list of normal bands");
        return NULL;
    }

    /* get the count of sensors present */
    sensor_count = ias_sat_attr_get_sensor_count();
    if (sensor_count == ERROR)
    {
        IAS_LOG_ERROR("Retrieving the sensor count");
        return NULL;
    }

    /* allocate the base structure, zeroing the contents so the pointers are
       initialized to null */
    model = calloc(1, sizeof(*model));
    if (!model)
    {
        IAS_LOG_ERROR("Allocating memory for the Line of Sight model");
        return NULL;
    }

    /* initialize some fields of the model */
    model->satellite_id = satellite_id;
    model->satellite_number = satellite_number;

    /* allocate the band array */
    sensor = &model->sensor;
    sensor->band_count = band_count;
    sensor->bands = calloc(band_count, sizeof(*sensor->bands));
    if (!sensor->bands)
    {
        IAS_LOG_ERROR("Allocating memory for the LOS band models");
        ias_los_model_free(model);
        return NULL;
    }

    /* initialize the sensor model */
    sensor->sensors[IAS_OLI].sensor_id = IAS_OLI;
    sensor->sensors[IAS_TIRS].sensor_id = IAS_TIRS;

    /* allocate the memory needed for each of the bands, skipping the items
       where the size is not yet known */
    for (index = 0; index < band_count; index++)
    {
        const IAS_BAND_ATTRIBUTES *band_info;
        IAS_SENSOR_BAND_MODEL *band_model;
        int sca_index;
        int band_number = band_number_list[index];

        /* get the info about this band */
        band_info = ias_sat_attr_get_band_attributes(band_number);
        if (!band_info)
        {
            IAS_LOG_ERROR("Retrieving the band attributes for band "
                    "number %d", band_number);
            ias_los_model_free(model);
            return NULL;
        }

        band_model = &sensor->bands[band_info->band_index];

        /* initialize the fields of the band model that can be */
        band_model->sensor = &sensor->sensors[band_info->sensor_id];

        /* allocate an array of SCAs for the band */
        band_model->sca_count = band_info->scas;
        band_model->scas = calloc(band_model->sca_count,
                sizeof(*band_model->scas));
        if (!band_model->scas)
        {
            IAS_LOG_ERROR("Allocating memory for the LOS SCA models for"
                    " band number %d", band_number);
            ias_los_model_free(model);
            return NULL;
        }

        /* allocate memory for each of the SCAs */
        for (sca_index = 0; sca_index < band_model->sca_count; sca_index++)
        {
            IAS_SENSOR_SCA_MODEL *sca = &band_model->scas[sca_index];

            sca->detectors = band_info->detectors_per_sca;

            /* allocate the separate members for the sca */
            sca->l0r_detector_offsets = calloc(sca->detectors,
                sizeof(*sca->l0r_detector_offsets));
            sca->detector_offsets_along_track = calloc(sca->detectors,
                sizeof(*sca->detector_offsets_along_track));
            sca->detector_offsets_across_track = calloc(sca->detectors,
                sizeof(*sca->detector_offsets_across_track));
            if (!sca->l0r_detector_offsets
                    || !sca->detector_offsets_along_track
                    || !sca->detector_offsets_across_track)
            {
                IAS_LOG_ERROR("Allocating memory for the LOS SCA model "
                        " for band number %d, sca index %d", band_number,
                        sca_index);
                ias_los_model_free(model);
                return NULL;
            }
        }
    }

    return model;
}

/*************************************************************************
Name: ias_los_model_free

Purpose: Frees all the memory allocated to a line of sight model

**************************************************************************/
void ias_los_model_free
(
    IAS_LOS_MODEL *model        /* I: model to free */
)
{
    int band_index;
    int sca_index;
    int sensor_index;
    IAS_SENSOR_MODEL *sensor;

    /* allow calling free with a NULL pointer */
    if (!model)
        return;

    sensor = &model->sensor;

    /* free the memory allocated for the band information */
    for (band_index = 0; band_index < sensor->band_count; band_index++)
    {
        IAS_SENSOR_BAND_MODEL *band = &sensor->bands[band_index];

        for (sca_index = 0; sca_index < band->sca_count; sca_index++)
        {
            IAS_SENSOR_SCA_MODEL *sca = &band->scas[sca_index];

            free(sca->l0r_detector_offsets);
            free(sca->detector_offsets_across_track);
            free(sca->detector_offsets_along_track);
        }

        free(band->scas);
    }
    free(sensor->bands);

    /* free the sensor related items */
    for (sensor_index = 0; sensor_index < IAS_MAX_SENSORS; sensor_index++)
    {
        free(sensor->frame_seconds_from_epoch[sensor_index]);
        if (sensor->sensors[sensor_index].jitter_table)
            free(sensor->sensors[sensor_index].jitter_table);
        if (sensor->sensors[sensor_index].ssm_model)
        {
            free(sensor->sensors[sensor_index].ssm_model->records);
            free(sensor->sensors[sensor_index].ssm_model);
        }
    }

    /* free the rest of the memory */
    free(model->spacecraft.attitude.sample_records);
    free(model->spacecraft.ephemeris.sample_records);

    free(model);
}
