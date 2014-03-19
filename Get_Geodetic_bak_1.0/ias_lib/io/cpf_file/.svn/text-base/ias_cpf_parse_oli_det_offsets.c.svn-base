/*************************************************************************

NAME: ias_cpf_parse_oli_det_offsets

PURPOSE: Retrieve the oli detector offset data group from the odl tree 
         and insert into the internal cpf structure.
        
RETURN VALUE: ERROR or SUCCESS

******************************************************************************/

#include <stdlib.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

#define NUMBER_ATTRIBUTES 2

static void free_oli_det_offset_memory
(
    int nbands,                         /* band loop control */
    int nscas,                          /* sca loop control */
    struct IAS_CPF_DETECTOR_OFFSETS *det_offsets  /* struct to free */
);

int ias_cpf_parse_oli_det_offsets
(
    const IAS_CPF *cpf,                         /* I: CPF structure */
    struct IAS_CPF_DETECTOR_OFFSETS *detector_offsets 
                                                /* O: CPF det offsets data */
)
{
    int nbands;                          /* total number bands */
    int band_index;                      /* band loop var */
    int band_number;                     /* Actual band number */
    int band_list[IAS_MAX_NBANDS];       /* list of band numbers */
    int status;                          /* Function return value */
    int nscas;                           /* total number scas */
    int sca_index;                             /* SCA loop var */
    int ndet;                            /* band detector count */
    int count = 0;                       /* number of list buckets */

    char group_name[] = "OLI_DETECTOR_OFFSETS"; /* group to retrieve */

    IAS_OBJ_DESC *odl_tree;              /* ODL tree */

    /* get band info */
    status = ias_sat_attr_get_sensor_band_numbers(IAS_OLI, IAS_NORMAL_BAND, 
                                                  0, band_list, IAS_MAX_NBANDS,
                                                  &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting band attributes");
        return ERROR;
    }

    /* get sca count */
    nscas = ias_sat_attr_get_sensor_sca_count(IAS_OLI);
    if (nscas == ERROR)
    {
        IAS_LOG_ERROR("Getting sat sca count");
        return ERROR;
    }

    /* Full attribute name with band and sca names */
    char attribute[nbands * nscas * NUMBER_ATTRIBUTES][ATTRIB_STRLEN];

    ODL_LIST_TYPE list[nbands * nscas * NUMBER_ATTRIBUTES];

    /* set the pointers to null */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            detector_offsets->along_per_detector[band_index][sca_index] = NULL;
            detector_offsets->across_per_detector[band_index][sca_index] = NULL;
        }
    }

    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* get band number from band index */
        band_number = band_list[band_index];

        /* get number of scas */
        nscas = ias_sat_attr_get_scas_per_band(band_number);
        if (nscas == ERROR)
        {
            IAS_LOG_ERROR("Getting sca count for band number: %d", 
                 band_number);
            free_oli_det_offset_memory(nbands, nscas, detector_offsets);
            return ERROR;
        }
        /* get detector count of current band */
        ndet = ias_sat_attr_get_detectors_per_sca(band_number);
        if (ndet == ERROR)
        {
            IAS_LOG_ERROR("Getting detector count for band number: %d", 
                 band_number);
            free_oli_det_offset_memory(nbands, nscas, detector_offsets);
            return ERROR;
        }

        /* Loop through the scas */
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]), 
                     "Along_Detector_Offsets_B%02d_SCA%02d",
                     band_number, sca_index + 1);
            if (status < 0 || status >= sizeof(attribute[count]))
            { 
                IAS_LOG_ERROR("Creating Along_Detector_Offsets attribute "
                              "string");
                free_oli_det_offset_memory(nbands, nscas, detector_offsets);
                return ERROR;
            }

            /* allocate space for Along Det Offsets in CPF */
            detector_offsets->along_per_detector[band_index][sca_index] 
                                                = malloc(ndet * sizeof(double));
            if (detector_offsets->along_per_detector[band_index][sca_index] 
                    == NULL)
            {
                IAS_LOG_ERROR("Allocating memory along det offsets group: %s", 
                                group_name);
                free_oli_det_offset_memory(nbands, nscas, detector_offsets);
                return ERROR;
            }

            /* populate list with oli along det delay info */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr 
                = detector_offsets->along_per_detector[band_index][sca_index];
            list[count].parm_size = ndet * sizeof(double);
            list[count].parm_type = IAS_ODL_Double;
            list[count].parm_count = ndet;
            count++;
         
            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]), 
                     "Across_Detector_Offsets_B%02d_SCA%02d",
                      band_number, sca_index + 1);
            if (status < 0 || status >= sizeof(attribute[count]))
            { 
                IAS_LOG_ERROR("Creating Across_Detector_Offsets attribute "
                              "string");
                free_oli_det_offset_memory(nbands, nscas, detector_offsets);
                return ERROR;
            }

            /* allocate space for Across Det Offsets in CPF */
            detector_offsets->across_per_detector[band_index][sca_index]
                                                = malloc(ndet * sizeof(double));
            if (detector_offsets->across_per_detector[band_index][sca_index] 
                    == NULL)
            {
                IAS_LOG_ERROR("Allocating memory across det offsets group: %s",
                                group_name);
                free_oli_det_offset_memory(nbands, nscas, detector_offsets);
                return ERROR;
            }

            /* populate list with oli along det delay info */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr 
                = detector_offsets->across_per_detector[band_index][sca_index];
            list[count].parm_size = ndet * sizeof(double);
            list[count].parm_type = IAS_ODL_Double;
            list[count].parm_count = ndet;
            count++;
         }
    }

    /* make a sanity check of number of parameters to retrieve */
    if ((nbands * nscas * NUMBER_ATTRIBUTES) != count)
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
        free_oli_det_offset_memory(nbands, nscas, detector_offsets);
        return ERROR;
    }

    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
}

static void free_oli_det_offset_memory
(
    int nbands,                         /* band loop control */
    int nscas,                          /* sca loop control */
    struct IAS_CPF_DETECTOR_OFFSETS *detector_offsets  /* struct to free */
)
{
    int band_index;                     /* band loop control */
    int sca_index;                      /* sca loop control */

    for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
    {
        for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
        {
            free(detector_offsets->along_per_detector[band_index][sca_index]);
            detector_offsets->along_per_detector[band_index][sca_index] = NULL;
            free(detector_offsets->across_per_detector[band_index][sca_index]);
            detector_offsets->across_per_detector[band_index][sca_index] = NULL;
        }
    }
}
