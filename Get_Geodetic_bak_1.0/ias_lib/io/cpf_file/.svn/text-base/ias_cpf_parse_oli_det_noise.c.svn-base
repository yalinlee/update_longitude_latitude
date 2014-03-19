/*************************************************************************

NAME: ias_cpf_parse_oli_det_noise

PURPOSE: Retrieve the OLI detector noise data group from the odl tree and 
         insert into the internal cpf structure. 

         The real vrp band numbers are 19 - 30 but are listed in the 
         CPF file with the prefix of VRP_ and use the associated normal band
         number.

NOTE:    The structure element pointers are initialized in ias_cpf_read.

RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

#define NUMBER_ATTRIBUTES 1

static void free_oli_det_noise_memory
(
    struct IAS_CPF_DETECTOR_NOISE *detector_noise
);

int ias_cpf_parse_oli_det_noise
(
    const IAS_CPF *cpf,                           /* I: CPF structure */
    struct IAS_CPF_DETECTOR_NOISE *detector_noise /* O: CPF det noise data */
)
{
    int band_count;                       /* total number bands */
    int band_index;                       /* band loop var */
    int normal_band_index;                /* Actual band index */
    int band_number;                      /* Actual band number */
    int parameter_band_number;            /* Band number of cpf parameter */
    int band_list[IAS_MAX_TOTAL_BANDS];   /* list of band numbers */
    int status;                           /* Function return value */
    int nscas;                            /* total number scas */
    int sca_index;                        /* sca loop counter */
    int ndets;                            /* band detector count */
    int count = 0;                        /* number of list buckets */
    char band_prefix[5];                  /* band parameter name prefix */
    char group_name[] = "OLI_DETECTOR_NOISE"; /* group to retrieve */

    IAS_OBJ_DESC *odl_tree;               /* ODL tree */

    const IAS_BAND_ATTRIBUTES *band_attr; /* band attributes structure */

    /* get oli band info */
    status = ias_sat_attr_get_any_sensor_band_numbers(IAS_OLI,
                IAS_NORMAL_BAND | IAS_BLIND_BAND | IAS_VRP_BAND, 0, band_list,
                IAS_MAX_TOTAL_BANDS, &band_count);

    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting sat normal and blind band attributes");
        return ERROR;
    }

    /* get sca count */
    nscas = ias_sat_attr_get_sensor_sca_count(IAS_OLI);
    if (nscas == ERROR)
    {
        IAS_LOG_ERROR("Getting sat OLI sca count");
        return ERROR;
    }

    /* Full attribute name with band and sca names */
    char attribute[band_count * nscas * NUMBER_ATTRIBUTES][ATTRIB_STRLEN];

    ODL_LIST_TYPE list[band_count * nscas * NUMBER_ATTRIBUTES];

    /* Loop through the bands */
    for (band_index = 0; band_index < band_count; band_index++)
    {
        /* get band number from band index */
        band_number = band_list[band_index];

        band_attr = ias_sat_attr_get_band_attributes(band_number);
        if (band_attr == NULL)
        {
            IAS_LOG_ERROR("Getting band attributes for band number: %d",
                            band_number);
            free_oli_det_noise_memory(detector_noise);
            return ERROR;
        }

        /* see if we are processing a vrp band */
        if(ias_sat_attr_band_classification_matches(band_number, (IAS_VRP_BAND))            || (ias_sat_attr_band_classification_matches(band_number,
                        (IAS_VRP_BAND | IAS_BLIND_BAND))))
        {
            /* get the normal band number for the vrp parameters */
            parameter_band_number = band_attr->normal_band_number;

            strncpy(band_prefix, "VRP_", sizeof(band_prefix));
            band_prefix[sizeof(band_prefix) - 1] = '\0';
        }
        else
        {
            strcpy(band_prefix, "");
            parameter_band_number = band_number;
        }

        /* get the index equivalent of the normal band number */
        normal_band_index 
            = ias_sat_attr_convert_band_number_to_index(band_number);
        if (normal_band_index == ERROR)
        {
            IAS_LOG_ERROR("Converting band number: %d to an index", 
                            band_number);
            free_oli_det_noise_memory(detector_noise);
            return ERROR;
        }

        /* get detector count of current band */
        ndets = ias_sat_attr_get_detectors_per_sca(band_number);
        if (ndets == ERROR)
        {
            IAS_LOG_ERROR("Getting detector count for band: %d", band_number);
            free_oli_det_noise_memory(detector_noise); 
            return ERROR;
        }

        /* loop through the scas */
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]),
                    "%s%s_B%02d_SCA%02d", band_prefix, 
                    "Detector_Noise", parameter_band_number, sca_index + 1);
            if ((status < 0) || (status >= sizeof(attribute[count])))
            {
                IAS_LOG_ERROR("Constructing CPF parameter name, group: %s",
                               group_name);
                return ERROR;
            }

            /* allocate space for Detector Noise in CPF */
            detector_noise->per_detector[normal_band_index][sca_index] 
                                        = malloc(ndets * sizeof(double));
            if (detector_noise
                ->per_detector[normal_band_index][sca_index] == NULL)
            {
                IAS_LOG_ERROR("Allocating memory group: %s", group_name);
                free_oli_det_noise_memory(detector_noise); 
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
        free_oli_det_noise_memory(detector_noise); 
        return ERROR;
    }

    GET_GROUP_FROM_CACHE(cpf, group_name, odl_tree);

    /* Populate the list from the odl tree */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting group: %s from CPF", group_name);
        DROP_ODL_TREE(odl_tree);
        free_oli_det_noise_memory(detector_noise); 
        return ERROR;
    }

    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
}

static void free_oli_det_noise_memory
(
    struct IAS_CPF_DETECTOR_NOISE *detector_noise
)
{
    int band_index;                 /* band loop control */
    int sca_index;                  /* sca loop control */

    for (band_index = 0; band_index < IAS_MAX_TOTAL_BANDS; band_index++)
    {
        for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
        {
            free(detector_noise->per_detector[band_index][sca_index]);
            detector_noise->per_detector[band_index][sca_index] = NULL;
        }
    }
}
