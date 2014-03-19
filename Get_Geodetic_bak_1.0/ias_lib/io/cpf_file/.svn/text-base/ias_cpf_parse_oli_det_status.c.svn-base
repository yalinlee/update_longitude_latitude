/*************************************************************************
NAME:   ias_cpf_parse_oli_det_status.c

PURPOSE: Retrieve the oli detector status group from the odl tree and insert 
         into internal cpf structure
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

#define NUMBER_BAND_ATTRIBUTES 2
#define NUMBER_ATTRIBUTES 2

static void free_oli_det_status_memory
(
    int nbands,                         /* band loop control */
    int nscas,                          /* sca loop control */
    struct IAS_CPF_DETECTOR_STATUS *detector_status,  /* struct to free */
    int band_list[]                     /* list of band numbers */
);

int ias_cpf_parse_oli_det_status
(
    const IAS_CPF *cpf,                 /* I: CPF structure */
    struct IAS_CPF_DETECTOR_STATUS *detector_status  
                                        /* O: CPF Detector Status data */
)
{
    int nbands;                          /* total number bands */
    int band_index;                      /* band loop var */
    int band_number;                     /* Actual band number */
    int parameter_band_number;           /* parameter attribute band number */
    int struct_band_index;               /* structure band index */
    int band_list[IAS_MAX_TOTAL_BANDS];  /* list of band numbers */
    int status;                          /* Function return value */
    int nscas;                           /* total number scas */
    int sca_index;                       /* SCA loop var */
    int count = 0;                       /* number of list buckets */
    int ndet;                            /* band detector count */
    int *int_ptr;                         /* int pointer for malloc */
    char group_name[] = "OLI_DETECTOR_STATUS";
                                         /* Group to retrieve from the CPF */
    char band_prefix[5];                 /* band parameter name prefix */
    IAS_OBJ_DESC *odl_tree;              /* ODL tree */
    const IAS_BAND_ATTRIBUTES *band_attr; /* band attributes structure */

    /* get oli band info */
    status = ias_sat_attr_get_any_sensor_band_numbers(IAS_OLI,
                IAS_NORMAL_BAND | IAS_BLIND_BAND | IAS_VRP_BAND, 0, band_list,
                IAS_MAX_TOTAL_BANDS, &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting sat band attributes");
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
    char attribute[nbands * nscas * NUMBER_BAND_ATTRIBUTES 
                   + NUMBER_ATTRIBUTES][ATTRIB_STRLEN];

    ODL_LIST_TYPE list[nbands * nscas * NUMBER_BAND_ATTRIBUTES 
                       + NUMBER_ATTRIBUTES];

    /* set structure points to null */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* get band number from band index */
        band_number = band_list[band_index];

        /* get the index equivalent of the band number */
        struct_band_index = ias_sat_attr_convert_band_number_to_index
            (band_number);
        if (struct_band_index == ERROR)
        {
            IAS_LOG_ERROR("Converting band number: %d to an index",
                            band_number);
            return ERROR;
        }
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            detector_status->out_of_spec[struct_band_index]
                    [sca_index] = NULL;
            detector_status->inoperable[struct_band_index]
                    [sca_index] = NULL;
        }
    }


    /* Retrieve the detector status band level counts */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* get band number from band index */
        band_number = band_list[band_index];

        band_attr = ias_sat_attr_get_band_attributes(band_number);
        if (band_attr == NULL)
        {
            IAS_LOG_ERROR("Getting band attributes vrp band number: %d",
                            band_number);
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

        /* get the index equivalent of the band number */
        struct_band_index = ias_sat_attr_convert_band_number_to_index
            (band_number);
        if (struct_band_index == ERROR)
        {
            IAS_LOG_ERROR("Converting band number: %d to an index",
                            band_number);
            return ERROR;
        }

        /* Add the band information to the CPF attribute name */
        status = snprintf(attribute[count], sizeof(attribute[count]),
               "%s%s%02d", band_prefix, "Out_Of_Spec_Count_B",
               parameter_band_number);
        if ((status < 0) || (status >= sizeof(attribute[count])))
        {
            IAS_LOG_ERROR("Constructing CPF parameter name, group: %s",
                           group_name);
            return ERROR;
        }
        /* Populate the table of CPF list parameters */
        list[count].group_name = group_name;
        list[count].attribute = attribute[count];
        list[count].parm_ptr = 
                &detector_status->out_of_spec_count[struct_band_index];
        list[count].parm_size = sizeof(
                detector_status->out_of_spec_count[struct_band_index]);
        list[count].parm_type = IAS_ODL_Int;
        list[count].parm_count = nscas;
        count++;

        /* Add the band information to the CPF attribute name */
        status = snprintf(attribute[count], sizeof(attribute[count]),
               "%s%s%02d", band_prefix, "Inoperable_Count_B",
               parameter_band_number);
        if ((status < 0) || (status >= sizeof(attribute[count])))
        {
            IAS_LOG_ERROR("Constructing CPF parameter name, group: %s",
                           group_name);
            return ERROR;
        }
        /* Populate the table of CPF list parameters */
        list[count].group_name = group_name;
        list[count].attribute = attribute[count];
        list[count].parm_ptr =
                &detector_status->inoperable_count[struct_band_index];
        list[count].parm_size = sizeof(
                detector_status->inoperable_count[struct_band_index]);
        list[count].parm_type = IAS_ODL_Int;
        list[count].parm_count = nscas;
        count++;
    }

    /* make a sanity check of number of parameters to retrieve */
    if ((nbands * NUMBER_BAND_ATTRIBUTES) != count)
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
        IAS_LOG_ERROR("Retrieving group: %s from CPF ",
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

        band_attr = ias_sat_attr_get_band_attributes(band_number);
        if (band_attr == NULL)
        {
            IAS_LOG_ERROR("Getting band attributes vrp band number: %d",
                            band_number);
            free_oli_det_status_memory(nbands, nscas, detector_status,
                band_list);
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

        /* get the index equivalent of the band number */
        struct_band_index = ias_sat_attr_convert_band_number_to_index
            (band_number);
        if (struct_band_index == ERROR)
        {
            IAS_LOG_ERROR("Converting band number: %d to an index",
                            band_number);
            free_oli_det_status_memory(nbands, nscas, detector_status,
                band_list);
            return ERROR;
        }

        /* Loop through the scas */
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]),
                "%s%s%02d_SCA%02d", band_prefix, "Out_Of_Spec_B",
                parameter_band_number, sca_index +1);
            if ((status < 0) || (status >= sizeof(attribute[count])))
            {
                IAS_LOG_ERROR("Constructing CPF parameter name, group: %s",
                           group_name);
                free_oli_det_status_memory(nbands, nscas, detector_status,
                    band_list);
                return ERROR;
            }
            /* Get the number of out of spec detectors for this SCA, if
               there aren't any, indicate 1 to read the (0) from the file */
            ndet = detector_status->out_of_spec_count
                   [struct_band_index][sca_index];
            if (ndet == 0)
                ndet++;
            /* Allocate space for the out of spec Detectors in the CPF */
            int_ptr = malloc(ndet * sizeof(int));
            if (int_ptr == NULL)
            {
                IAS_LOG_ERROR("Allocating memory Out of spec dets group: %s",
                                group_name);
                free_oli_det_status_memory(nbands, nscas, detector_status,
                    band_list);
                return ERROR;
            }

            /* Populate the table of CPF list parameters */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr = int_ptr;
            detector_status->out_of_spec
                [struct_band_index][sca_index] = int_ptr;
            list[count].parm_size = ndet * sizeof(int);
            list[count].parm_type = IAS_ODL_Int;
            list[count].parm_count = ndet;
            count++;

            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]),
                "%s%s%02d_SCA%02d", band_prefix, "Inoperable_B",
                parameter_band_number, sca_index +1);
            if ((status < 0) || (status >= sizeof(attribute[count])))
            {
                IAS_LOG_ERROR("Constructing CPF parameter name, group: %s",
                           group_name);
                free_oli_det_status_memory(nbands, nscas, detector_status,
                    band_list);
                return ERROR;
            }
            /* Get the number of Inoperable detectors for this SCA, if there
               aren't any, indicate 1 to read the (0) from the file */
            ndet = detector_status->inoperable_count
                   [struct_band_index][sca_index];
            if (ndet == 0)
                ndet++;
            /* Allocate space for the inoperable Detectors in the CPF */
            int_ptr = malloc(ndet * sizeof(int));
            if (int_ptr == NULL)
            {
                IAS_LOG_ERROR("Allocating memory inoperable Dets group: %s",
                                group_name);
                free_oli_det_status_memory(nbands, nscas, detector_status,
                    band_list);
                return ERROR;
            }

            /* Populate the table of CPF list parameters */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr = int_ptr;
            detector_status->inoperable
                [struct_band_index][sca_index] = int_ptr;
            list[count].parm_size = ndet * sizeof(int);
            list[count].parm_type = IAS_ODL_Int;
            list[count].parm_count = ndet;
            count++;
        }
    }

    /* Populate the table with detector select table element */
    list[count].group_name = group_name;
    list[count].attribute = "Detector_Select_Table";
    list[count].parm_ptr = &detector_status->oli_detector_select_table;
    list[count].parm_size = sizeof(int);
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = 1;
    count++;

    /* Populate the table with detector select table ID element */
    list[count].group_name = group_name;
    list[count].attribute = "Detector_Select_Table_Id";
    list[count].parm_ptr = &detector_status->oli_detector_select_table_id;
    list[count].parm_size = sizeof(int);
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = 1;
    count++;
    /* make a sanity check of number of parameters to retrieve */ 
    if ((nbands * nscas * NUMBER_BAND_ATTRIBUTES + NUMBER_ATTRIBUTES) != count)
    {
        IAS_LOG_ERROR("Number of parameters does not match number to retrieve");
        free_oli_det_status_memory(nbands, nscas, detector_status, band_list);
        return ERROR;
    }
    /* Retrieve the detector attributes values, specified in the
       list parameters, from the CPF ODL Object Tree */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS) 
    {
        IAS_LOG_ERROR("Getting group: %s from CPF", group_name); 
        DROP_ODL_TREE(odl_tree);
        free_oli_det_status_memory(nbands, nscas, detector_status, band_list);
        return ERROR;
    }
    DROP_ODL_TREE(odl_tree);
    return SUCCESS;
}


static void free_oli_det_status_memory
(
    int nbands,                         /* band loop control */
    int nscas,                          /* sca loop control */
    struct IAS_CPF_DETECTOR_STATUS *detector_status,  /* struct to free */
    int band_list[]                     /* list of band numbers */
)
{
    int band_index;                     /* band loop control */
    int sca_index;                      /* sca loop control */
    int normal_band_index;          /* normal band number converted to index */

    for (band_index = 0; band_index < nbands; band_index++)
    {
        normal_band_index
            = ias_sat_attr_convert_band_number_to_index(band_list[band_index]);
        if (normal_band_index == ERROR)
        {
            IAS_LOG_ERROR("Converting the band number to an index");
            return;
        }
        for (sca_index = 0; sca_index < IAS_MAX_NSCAS; sca_index++)
        {   
            free(detector_status->out_of_spec[normal_band_index]
                    [sca_index]);
            detector_status->out_of_spec[normal_band_index]
                    [sca_index] = NULL;
            free(detector_status->inoperable[normal_band_index]
                    [sca_index]);
            detector_status->inoperable[normal_band_index]
                    [sca_index] = NULL;
        }
    }

}
