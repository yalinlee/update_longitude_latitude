/*************************************************************************
PURPOSE: Retrieve the tirs detector status blind group from the odl tree 
         and insert into internal cpf structure
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include <stdlib.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

#define NUMBER_ATTRIBUTES 2

int ias_cpf_parse_tirs_det_status_blind
(
    const IAS_CPF *cpf,          /* I: CPF structure */
    struct IAS_CPF_DETECTOR_STATUS *tirs_det_status_blind  
                                 /* O: CPF Detector Status blind band data */
)
{
    int nbands;                          /* total number bands */
    int band_index;                      /* band loop var */
    int band_number;                     /* Actual band number */
    int band_list[IAS_MAX_NBANDS];       /* list of band numbers */
    int status;                          /* Function return value */
    int nscas;                           /* total number scas */
    int sca_index;                       /* SCA loop var */
    int count = 0;                       /* number of list buckets */
    int ndet;                            /* band detector count */
    char group_name[] = "TIRS_DETECTOR_STATUS"; 
                                         /* Group to retrieve from the CPF */
    IAS_OBJ_DESC *odl_tree;              /* ODL tree */

    /* get tirs band info  tirs band list has 10 & 11 in it */
    status = ias_sat_attr_get_sensor_band_numbers(IAS_TIRS, IAS_BLIND_BAND, 
                                                  IAS_SECONDARY_BAND, 
                                                  band_list, IAS_MAX_NBANDS,
                                                  &nbands);
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

    /* set points to null */
    for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
    {
        for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
        {   
            tirs_det_status_blind->out_of_spec[band_index][sca_index] = NULL;
            tirs_det_status_blind->inoperable[band_index][sca_index] = NULL;
        }
    }

    /* Retrieve the detector status band level counts */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* get band number from band index */
        band_number = band_list[band_index];

        /* Add the SCA information to the CPF attribute name */
        status = snprintf(attribute[count], sizeof(attribute[count]), 
                 "Out_Of_Spec_Blind_Count_B%02d", band_number);
        if (status < 0 || status >= sizeof(attribute[count]))
        { 
            IAS_LOG_ERROR("Creating Out_Of_Spec_Blind_Count attribute string");
            return ERROR;
        }

        /* Populate the table of CPF list parameters */
        list[count].group_name = group_name;
        list[count].attribute = attribute[count];
        list[count].parm_ptr 
           = &tirs_det_status_blind->out_of_spec_count[band_index];
        list[count].parm_size 
           = sizeof(tirs_det_status_blind->out_of_spec_count[band_index]);
        list[count].parm_type = IAS_ODL_Int;
        list[count].parm_count = nscas;
        count++;

        /* Add the SCA information to the CPF attribute name */
        status = snprintf(attribute[count], sizeof(attribute[count]), 
                 "Inoperable_Blind_Count_B%02d", band_number);
        if (status < 0 || status >= sizeof(attribute[count]))
        { 
            IAS_LOG_ERROR("Creating Out_Of_Spec_Blind_Count attribute string");
            return ERROR;
        }

        /* Populate the table of CPF list parameters */
        list[count].group_name = group_name;
        list[count].attribute = attribute[count];
        list[count].parm_ptr 
           = &tirs_det_status_blind->inoperable_count[band_index];
        list[count].parm_size 
           = sizeof(tirs_det_status_blind->inoperable_count[band_index]);
        list[count].parm_type = IAS_ODL_Int;
        list[count].parm_count = nscas;
        count++;
    }

    /* make a sanity check of number of parameters to retrieve */
    if (NUMBER_ATTRIBUTES != count)
    {
        IAS_LOG_ERROR("Number of parameters does not match number to retrieve");
        return ERROR;
    }

    GET_GROUP_FROM_CACHE(cpf, group_name, odl_tree);

    /* Retrieve the counts for each of the detector status values,
       specified in the list parameters, from the CPF ODL Object Tree 
       these are needed to pull the band level data from the cpf */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS) 
    {
        IAS_LOG_ERROR("Retrieving TIRS Detector Status counts group: %s",
                    group_name);
        DROP_ODL_TREE(odl_tree);
        return ERROR ;
    }

    /* Reset the list parameter counter so the detector values
       can be retrieved */
    count = 0;
    
    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* get band number from band index */
        band_number = band_list[band_index];

        nscas = ias_sat_attr_get_scas_per_band(band_number);
        if (nscas == ERROR)
        {
            IAS_LOG_ERROR("Getting scas per band group: %s", group_name);
            for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
            {
                for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
                {   
                    free(tirs_det_status_blind
                        ->out_of_spec[band_index][sca_index]);
                    tirs_det_status_blind
                        ->out_of_spec[band_index][sca_index] = NULL;
                    free(tirs_det_status_blind
                        ->inoperable[band_index][sca_index]);
                    tirs_det_status_blind
                        ->inoperable[band_index][sca_index] = NULL;
                }
            }
            DROP_ODL_TREE(odl_tree);
            return ERROR;
        }

        /* Loop through the scas */
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]), 
                     "Out_Of_Spec_Blind_B%02d_SCA%02d",
                     band_number, sca_index+1);
            if (status < 0 || status >= sizeof(attribute[count]))
            { 
                IAS_LOG_ERROR("Creating Out_Of_Spec_Blind attribute string");
                for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
                {
                    for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
                    {   
                        free(tirs_det_status_blind
                            ->out_of_spec[band_index][sca_index]);
                        tirs_det_status_blind
                            ->out_of_spec[band_index][sca_index] = NULL;
                        free(tirs_det_status_blind
                            ->inoperable[band_index][sca_index]);
                        tirs_det_status_blind
                            ->inoperable[band_index][sca_index] = NULL;
                    }
                }
                DROP_ODL_TREE(odl_tree);
                return ERROR;
            }

            /* Get the number of out of spec detectors for this SCA, if
               there aren't any, indicate 1 to read the (0) from the file */
            ndet = tirs_det_status_blind
                    ->out_of_spec_count[band_index][sca_index];
            if (ndet == 0)
                ndet++;

            /* Allocate space for the out of spec Detectors in the CPF */
            tirs_det_status_blind->out_of_spec[band_index][sca_index] 
                                       = malloc(ndet * sizeof(int));
            if (tirs_det_status_blind
                    ->out_of_spec[band_index][sca_index] == NULL)
            {
                IAS_LOG_ERROR("Allocating memory for Blind Out of spec Dets "
                              "group: %s", group_name);
                for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
                {
                    for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
                    {   
                        free(tirs_det_status_blind
                            ->out_of_spec[band_index][sca_index]);
                        tirs_det_status_blind
                            ->out_of_spec[band_index][sca_index] = NULL;
                        free(tirs_det_status_blind
                            ->inoperable[band_index][sca_index]);
                        tirs_det_status_blind
                            ->inoperable[band_index][sca_index] = NULL;
                    }
                }
                DROP_ODL_TREE(odl_tree);
                return ERROR;
            }

            /* Populate the table of CPF list parameters */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr 
              = tirs_det_status_blind->out_of_spec[band_index][sca_index];
            list[count].parm_size = ndet * sizeof(int);
            list[count].parm_type = IAS_ODL_Int;
            list[count].parm_count = ndet;
            count++;

            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]), 
                    "Inoperable_Blind_B%02d_SCA%02d",
                    band_number, sca_index+1);
            if (status < 0 || status >= sizeof(attribute[count]))
            { 
                IAS_LOG_ERROR("Creating Inoperable_Blind attribute string");
                for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
                {
                    for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
                    {   
                        free(tirs_det_status_blind
                            ->out_of_spec[band_index][sca_index]);
                        tirs_det_status_blind
                            ->out_of_spec[band_index][sca_index] = NULL;
                        free(tirs_det_status_blind
                            ->inoperable[band_index][sca_index]);
                        tirs_det_status_blind
                            ->inoperable[band_index][sca_index] = NULL;
                    }
                }
                DROP_ODL_TREE(odl_tree);
                return ERROR;
            }

            /* Get the number of Inoperable detectors for this SCA
               (Added one to allow for a zero terminated list) */
            /* Get the number of Inoperable detectors for this SCA, if
               there aren't any, indicate 1 to read the (0) from the file */
            ndet = tirs_det_status_blind
                    ->inoperable_count[band_index][sca_index];
            if (ndet == 0)
                ndet++;

            /* Allocate space for the inoperable Detectors in the CPF */
            tirs_det_status_blind->inoperable[band_index][sca_index] 
                                    = malloc(ndet * sizeof(int));
            if (tirs_det_status_blind
                    ->inoperable[band_index][sca_index] == NULL)
            {
                IAS_LOG_ERROR("Allocating memory for inoperable Dets "
                              "group: %s", group_name);
                for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
                {
                    for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
                    {   
                        free(tirs_det_status_blind
                            ->out_of_spec[band_index][sca_index]);
                        tirs_det_status_blind
                            ->out_of_spec[band_index][sca_index] = NULL;
                        free(tirs_det_status_blind
                            ->inoperable[band_index][sca_index]);
                        tirs_det_status_blind
                            ->inoperable[band_index][sca_index] = NULL;
                    }
                }
                DROP_ODL_TREE(odl_tree);
                return ERROR;
            }

            /* Populate the table of CPF list parameters */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr 
              = tirs_det_status_blind->inoperable[band_index][sca_index];
            list[count].parm_size = ndet * sizeof(int);
            list[count].parm_type = IAS_ODL_Int;
            list[count].parm_count = ndet;
            count++;

        }
    }

    /* make a sanity check of number of parameters to retrieve */
    if ((nbands * nscas * NUMBER_ATTRIBUTES) != count)
    {
        IAS_LOG_ERROR("Number of parameters does not match number to retrieve");
        DROP_ODL_TREE(odl_tree);
        return ERROR;
    }

    /* Retrieve the detector attributes values, specified in the
       list parameters, from the CPF ODL Object Tree */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS) 
    {
        IAS_LOG_ERROR("Getting blind detector status group: %s from CPF",
                    group_name); 
        DROP_ODL_TREE(odl_tree);
        for (band_index = 0; band_index < IAS_MAX_NBANDS; band_index++)
        {
            for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
            {   
                free(tirs_det_status_blind
                    ->out_of_spec[band_index][sca_index]);
                tirs_det_status_blind
                    ->out_of_spec[band_index][sca_index] = NULL;
                free(tirs_det_status_blind
                    ->inoperable[band_index][sca_index]);
                tirs_det_status_blind
                    ->inoperable[band_index][sca_index] = NULL;
            }
        }
        return ERROR;
    }

    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
}
