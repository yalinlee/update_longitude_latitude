/**************************************************************************

NAME: ias_cpf_parse_tirs_detector_response_blind

PURPOSE: Retrieve the tirs detector response blind data group from the 
         odl tree and insert into the internal cpf strucure.
        
RETURNS: SUCCESS or ERROR

******************************************************************************/

#include <stdlib.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

#define NUMBER_ATTRIBUTES 3

int ias_cpf_parse_tirs_detector_response_blind
(
    const IAS_CPF *cpf,                /* I: CPF structure */
    struct IAS_CPF_TIRS_DETECTOR_RESPONSE *tirs_det_response_blind 
                                       /* O: CPF tirs detector response data */
)
{
    int nbands;           /* total tirs bands */
    int band_index;       /* band loop counter */
    int band_number;      /* actual band number */
    int band_list[IAS_TIRS_MAX_NBANDS];  /* list of band numbers */
    int nscas;            /* total number scas */
    int sca_index;        /* sca loop counter */
    int ndet;             /* band detector count */
    int status;           /* Status of return from function */
    int count = 0;        /* Number of items in CPF_LIST_TYPE */
    char group_name[] = "TIRS_DETECTOR_RESPONSE_BLIND"; 
                          /* Group to retrieve from the CPF */

    IAS_OBJ_DESC *odl_tree;    /* ODL tree */

    /* get tirs band info */
    status = ias_sat_attr_get_sensor_band_numbers(IAS_TIRS, IAS_BLIND_BAND, 
                                                  IAS_SECONDARY_BAND, band_list,
                                                  IAS_TIRS_MAX_NBANDS, &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting tirs blind band attributes");
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
    char attribute[nbands * nscas * NUMBER_ATTRIBUTES][ATTRIB_STRLEN]; 

    ODL_LIST_TYPE list[nbands * nscas * NUMBER_ATTRIBUTES]; 

    /* set the pointers to null */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
           tirs_det_response_blind
                ->baseline_dark_response[band_index][sca_index] = NULL;
           tirs_det_response_blind
                ->background_response[band_index][sca_index] = NULL;
           tirs_det_response_blind
                ->gain_offsets[band_index][sca_index] = NULL;
        } 
    }
    
    /* Loop through the bands */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* get band number from band index */
        band_number = band_list[band_index];
        
        /* get detector count of current band */
        ndet = ias_sat_attr_get_detectors_per_sca(band_number);
        if (ndet == ERROR)
        {
            IAS_LOG_ERROR("Getting detector count per sca from sat attributes");
            for (band_index = 0; band_index < nbands; band_index++)
            {
                for (sca_index = 0; sca_index < nscas; sca_index++)
                {
                    free(tirs_det_response_blind
                       ->baseline_dark_response[band_index][sca_index]);
                    tirs_det_response_blind
                       ->baseline_dark_response[band_index][sca_index] = NULL;
                    free(tirs_det_response_blind
                       ->background_response[band_index][sca_index]);
                    tirs_det_response_blind
                       ->background_response[band_index][sca_index] = NULL;
                    free(tirs_det_response_blind
                       ->gain_offsets[band_index][sca_index]);
                    tirs_det_response_blind
                       ->gain_offsets[band_index][sca_index] = NULL;
                }
            }
            return ERROR;
        }

        /* loop through the scas */
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]), 
                     "Baseline_Dark_Response_B%02d_SCA%02d",
                     band_number, sca_index + 1);
            if (status < 0 || status >= sizeof(attribute[count]))
            { 
                IAS_LOG_ERROR("Creating Baseline_Dark_Response attribute "
                              "string");
                for (band_index = 0; band_index < nbands; band_index++)
                {
                    for (sca_index = 0; sca_index < nscas; sca_index++)
                    {
                        free(tirs_det_response_blind
                           ->baseline_dark_response[band_index][sca_index]);
                        tirs_det_response_blind
                           ->baseline_dark_response[band_index][sca_index]
                           = NULL;
                        free(tirs_det_response_blind
                           ->background_response[band_index][sca_index]);
                        tirs_det_response_blind
                           ->background_response[band_index][sca_index] = NULL;
                        free(tirs_det_response_blind
                           ->gain_offsets[band_index][sca_index]);
                        tirs_det_response_blind
                           ->gain_offsets[band_index][sca_index] = NULL;
                    }
                }
                return ERROR;
            }        

            /* allocate space for dark responce parameters */
            tirs_det_response_blind
                    ->baseline_dark_response[band_index][sca_index] 
                            = malloc(ndet * sizeof(double)); 
            if (tirs_det_response_blind
                    ->baseline_dark_response[band_index][sca_index] == NULL) 
            {
                IAS_LOG_ERROR("Allocating memory for tirs dark response blind "
                              "group: %s", group_name);
                for (band_index = 0; band_index < nbands; band_index++)
                {
                    for (sca_index = 0; sca_index < nscas; sca_index++)
                    {
                        free(tirs_det_response_blind
                           ->baseline_dark_response[band_index][sca_index]);
                        tirs_det_response_blind
                           ->baseline_dark_response[band_index][sca_index]
                           = NULL;
                        free(tirs_det_response_blind
                           ->background_response[band_index][sca_index]);
                        tirs_det_response_blind
                           ->background_response[band_index][sca_index] = NULL;
                        free(tirs_det_response_blind
                           ->gain_offsets[band_index][sca_index]);
                        tirs_det_response_blind
                           ->gain_offsets[band_index][sca_index] = NULL;
                    }
                }
                return ERROR;
            }
            
            /* populate list with dark response info */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr = tirs_det_response_blind
                            ->baseline_dark_response[band_index][sca_index];
            list[count].parm_size = ndet * sizeof(double);
            list[count].parm_type = IAS_ODL_Double;
            list[count].parm_count = ndet;
            count++;

            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]),
                     "Background_Response_B%02d_SCA%02d",
                     band_number, sca_index + 1);
            if (status < 0 || status >= sizeof(attribute[count]))
            { 
                IAS_LOG_ERROR("Creating Background_Response attribute string");
                for (band_index = 0; band_index < nbands; band_index++)
                {
                    for (sca_index = 0; sca_index < nscas; sca_index++)
                    {
                        free(tirs_det_response_blind
                           ->baseline_dark_response[band_index][sca_index]);
                        tirs_det_response_blind
                           ->baseline_dark_response[band_index][sca_index]
                           = NULL;
                        free(tirs_det_response_blind
                           ->background_response[band_index][sca_index]);
                        tirs_det_response_blind
                           ->background_response[band_index][sca_index] = NULL;
                        free(tirs_det_response_blind
                           ->gain_offsets[band_index][sca_index]);
                        tirs_det_response_blind
                           ->gain_offsets[band_index][sca_index] = NULL;                    }
                }
                return ERROR;
            }
        
            /* allocate space for Background responce parameters */
            tirs_det_response_blind->background_response[band_index][sca_index] 
                        = malloc(ndet * sizeof(double)); 
            if (tirs_det_response_blind
                        ->background_response[band_index][sca_index] == NULL) 
            {
                IAS_LOG_ERROR("Allocating memory for tirs background response "
                              "group: %s", group_name);
                for (band_index = 0; band_index < nbands; band_index++)
                {
                    for (sca_index = 0; sca_index < nscas; sca_index++)
                    {
                        free(tirs_det_response_blind
                           ->baseline_dark_response[band_index][sca_index]);
                        tirs_det_response_blind
                           ->baseline_dark_response[band_index][sca_index]
                           = NULL;
                        free(tirs_det_response_blind
                           ->background_response[band_index][sca_index]);
                        tirs_det_response_blind
                           ->background_response[band_index][sca_index] = NULL;
                        free(tirs_det_response_blind
                           ->gain_offsets[band_index][sca_index]);
                        tirs_det_response_blind
                           ->gain_offsets[band_index][sca_index] = NULL;                    }
                }
                return ERROR;
            }
            
            /* populate list with background response info */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr = tirs_det_response_blind
                                ->background_response[band_index][sca_index];
            list[count].parm_size = ndet * sizeof(double);
            list[count].parm_type = IAS_ODL_Double;
            list[count].parm_count = ndet;
            count++;

            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]),
                     "Gain_Offsets_B%02d_SCA%02d",
                     band_number, sca_index + 1);
            if (status < 0 || status >= sizeof(attribute[count]))
            { 
                IAS_LOG_ERROR("Creating Gain_Offsets attribute string");
                for (band_index = 0; band_index < nbands; band_index++)
                {
                    for (sca_index = 0; sca_index < nscas; sca_index++)
                    {
                        free(tirs_det_response_blind
                           ->baseline_dark_response[band_index][sca_index]);
                        tirs_det_response_blind
                           ->baseline_dark_response[band_index][sca_index]
                           = NULL;
                        free(tirs_det_response_blind
                           ->background_response[band_index][sca_index]);
                        tirs_det_response_blind
                           ->background_response[band_index][sca_index] = NULL;
                        free(tirs_det_response_blind
                           ->gain_offsets[band_index][sca_index]);
                        tirs_det_response_blind
                           ->gain_offsets[band_index][sca_index] = NULL;                    }
                }
                return ERROR;
            }
        
            /* allocate space for Background responce parameters */
            tirs_det_response_blind->gain_offsets[band_index][sca_index] 
                        = malloc(ndet * sizeof(double)); 
            if (tirs_det_response_blind
                        ->gain_offsets[band_index][sca_index] == NULL) 
            {
                IAS_LOG_ERROR("Allocating memory for tirs gain function offset "
                              "group: %s", group_name);
                for (band_index = 0; band_index < nbands; band_index++)
                {
                    for (sca_index = 0; sca_index < nscas; sca_index++)
                    {
                        free(tirs_det_response_blind
                           ->baseline_dark_response[band_index][sca_index]);
                        tirs_det_response_blind
                           ->baseline_dark_response[band_index][sca_index]
                           = NULL;
                        free(tirs_det_response_blind
                           ->background_response[band_index][sca_index]);
                        tirs_det_response_blind
                           ->background_response[band_index][sca_index] = NULL;
                        free(tirs_det_response_blind
                           ->gain_offsets[band_index][sca_index]);
                        tirs_det_response_blind
                           ->gain_offsets[band_index][sca_index] = NULL;
                    }
                }
                return ERROR;
            }
            
            /* populate list with gain function offset info */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr = tirs_det_response_blind
                                ->gain_offsets[band_index][sca_index];
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
        IAS_LOG_ERROR("Getting tirs det response group: %s from CPF",
                        group_name);

        /* free up allocated memory */
        for (band_index = 0; band_index < nbands; band_index++)
        {
            for (sca_index = 0; sca_index < nscas; sca_index++)
            {
                free(tirs_det_response_blind
                        ->baseline_dark_response[band_index][sca_index]);
                tirs_det_response_blind
                    ->baseline_dark_response[band_index][sca_index] = NULL;
                free(tirs_det_response_blind
                        ->background_response[band_index][sca_index]);
                tirs_det_response_blind
                    ->background_response[band_index][sca_index] = NULL;
                free(tirs_det_response_blind
                        ->gain_offsets[band_index][sca_index]);
                tirs_det_response_blind
                    ->gain_offsets[band_index][sca_index] = NULL;
            }
        }
        DROP_ODL_TREE(odl_tree);
        return ERROR;
    }

    /* free allocated memory */
    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
}
