/*************************************************************************

NAME: ias_cpf_parse_tirs_det_noise

PURPOSE: Retrieve the TIRS detector noise data group from the odl tree and 
         insert into the internal cpf structure.  Includes all the TIRS bands
         band(s)

NOTE:    The structure elements pointers are initialized in ias_cpf_read.

RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include <stdlib.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

#define NUMBER_ATTRIBUTES 1

static void free_det_noise_memory
(
    struct IAS_CPF_DETECTOR_NOISE *detector_noise /* O: CPF det noise data */
);

int ias_cpf_parse_tirs_det_noise
(
    const IAS_CPF *cpf,            /* I: CPF structure */
    struct IAS_CPF_DETECTOR_NOISE *detector_noise /* O: CPF det noise data */
)
{
    int band_count;                /* total number bands */
    int band_index;                /* band loop var */
    int normal_band_index;         /* real band number converted to index */ 
    int band_number;               /* Actual band number */
    int band_list[IAS_MAX_TOTAL_BANDS]; /* list of normal band numbers */
    int status;                    /* Function return value */
    int nscas;                     /* total number scas */
    int sca_index;                 /* sca loop counter */
    int ndets;                     /* band detector count */
    int count = 0;                 /* number of list buckets */
    char group_name[] = "TIRS_DETECTOR_NOISE"; /* group to retrieve */

    IAS_OBJ_DESC *odl_tree;        /* ODL tree */

    /* get all the TIRS bands */
    status = ias_sat_attr_get_any_sensor_band_numbers(IAS_TIRS, 
                      IAS_NORMAL_BAND | IAS_BLIND_BAND | IAS_SECONDARY_BAND,
                      0, band_list, IAS_MAX_TOTAL_BANDS, &band_count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting normal and blind band attributes");
        return ERROR;
    }

    /* get sca count */
    nscas = ias_sat_attr_get_sensor_sca_count(IAS_TIRS);
    if (nscas == ERROR)
    {
        IAS_LOG_ERROR("Getting sat sca count");
        return ERROR;
    }

    /* Full attribute name with band and sca names */
    char attribute[band_count * nscas * NUMBER_ATTRIBUTES][ATTRIB_STRLEN];

    ODL_LIST_TYPE list[band_count * nscas * NUMBER_ATTRIBUTES];

    /* Loop through the bands */
    count = 0;
    for (band_index = 0; band_index < band_count; band_index++)
    {
        /* get band number from band index */
        band_number = band_list[band_index];

        /* get the index of the actual band number */
        normal_band_index 
                = ias_sat_attr_convert_band_number_to_index(band_number);
        if (normal_band_index == ERROR)
        {
            IAS_LOG_ERROR("Converting the band number to an index");
            free_det_noise_memory(detector_noise);
            return ERROR;
        }

        /* get detector count of current band */
        ndets = ias_sat_attr_get_detectors_per_sca(band_number);
        if (ndets == ERROR)
        {
            IAS_LOG_ERROR("Getting detector count for  band number: %d", 
                            band_number);
            free_det_noise_memory(detector_noise);
            return ERROR;
        }

        /* loop through the scas */
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]), 
                    "Detector_Noise_B%02d_SCA%02d",
                    band_number, sca_index + 1);
            if (status < 0 || status >= sizeof(attribute[count]))
            { 
                IAS_LOG_ERROR("Creating Detector_Noise attribute string");
                free_det_noise_memory(detector_noise);
                return ERROR;
            }

            /* allocate space for Detector Noise in CPF */
            detector_noise->per_detector[normal_band_index][sca_index] 
                                        = malloc(ndets * sizeof(double));

            if (detector_noise->per_detector[normal_band_index][sca_index]
                == NULL)
            {
                IAS_LOG_ERROR("Allocating memory detector noise group: %s",
                                group_name);
                free_det_noise_memory(detector_noise);
                return ERROR;
            }

            /* populate list with detector noise info */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr 
                = detector_noise->per_detector[normal_band_index][sca_index];
            list[count].parm_size = ndets * sizeof(double);
            list[count].parm_type = IAS_ODL_Double;
            list[count].parm_count = ndets;
            count++;
         }
    }

    /* make a sanity check of number of parameters to retrieve */
    if ((band_count * nscas * NUMBER_ATTRIBUTES) != count)
    {
        IAS_LOG_ERROR("Number of parameters does not match number to retrieve");
        return ERROR;
    }

    GET_GROUP_FROM_CACHE(cpf, group_name, odl_tree);

    /* Populate the list from the odl tree */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting group: %s from CPF", group_name);
        DROP_ODL_TREE(odl_tree);
        free_det_noise_memory(detector_noise);
        return ERROR;
    }
    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
}


static void free_det_noise_memory
(
    struct IAS_CPF_DETECTOR_NOISE *detector_noise /* O: CPF det noise data */
)
{
    int band_index;             /* loop counter */
    int sca_index;              /* loop counter */

    for (band_index = 0; band_index < IAS_MAX_TOTAL_BANDS; band_index++)
    {
        for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
        {
            free(detector_noise->per_detector[band_index][sca_index]);
            detector_noise->per_detector[band_index][sca_index] = NULL;
        }
    }
}
