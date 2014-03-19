/*************************************************************************

NAME: ias_cpf_parse_tirs_focal_plane

PURPOSE: Retrieve the tirs focal plane data group from the odl tree and insert
         into the internal cpf tirs structure.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "ias_cpf.h"
#include "ias_const.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

#define NUMBER_BAND_ATTRIBUTES 2
#define NUMBER_ATTRIBUTES 12

int ias_cpf_parse_tirs_focal_plane
(
    const IAS_CPF *cpf,                     /* I: CPF structure */
    struct IAS_CPF_FOCAL_PLANE *focal_plane /* O: cpf focal plane data */
)
{
    int status;                     /* Function return value */
    int band_index;                 /* band loop var */
    int sca_index;                  /* SCA loop var */
    int band_number;                /* Actual band number */
    int normal_band_index;          /* normal band number converted to index */
    int band_list[IAS_MAX_NBANDS];  /* list of band numbers */
    int nbands;                     /* total number bands */
    int nscas;                      /* total number scas */
    int i;                          /* Array counters */
    int count = 0;                  /* number of list buckets */
    char group_name[] = "TIRS_FOCAL_PLANE"; /* Name of group to retrieve */

    int local_dets_per_band[IAS_MAX_NBANDS]; /* local array */
    char *band_str[IAS_MAX_NBANDS]; /* local band name array */
    int local_band_offset[IAS_MAX_NBANDS]; /* local array */
    int local_sca_overlap[IAS_MAX_NBANDS]; /* local array */
    

    IAS_OBJ_DESC *odl_tree;              /* ODL tree */

    memset(band_str, 0, sizeof(band_str));

    /* get tirs band info */
    status = ias_sat_attr_get_sensor_band_numbers(IAS_TIRS, IAS_NORMAL_BAND,
                                                  0, band_list, IAS_MAX_NBANDS,
                                                  &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting sat band attributes");
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
    char attribute[nbands * nscas * NUMBER_BAND_ATTRIBUTES 
                   + NUMBER_ATTRIBUTES][ATTRIB_STRLEN];

    ODL_LIST_TYPE list[nbands * nscas * NUMBER_BAND_ATTRIBUTES 
                       + NUMBER_ATTRIBUTES];

    /* Loop thru the tirs bands */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* get band number from band index */
        band_number = band_list[band_index];
        
        /* get the index of the normal band number */
        normal_band_index 
            = ias_sat_attr_convert_band_number_to_index(band_number);
        if (normal_band_index == ERROR)
        {
            IAS_LOG_ERROR("Converting the band number to an index");
            return ERROR;
        }

        /* get number of scas this band */
        nscas = ias_sat_attr_get_scas_per_band(band_number);
        if (nscas == ERROR)
        {
            IAS_LOG_ERROR("Getting sca count for band number: %d", 
                 band_number);
            return ERROR;
        }

        /* get the index equivalent of the normal band number */
        normal_band_index 
            = ias_sat_attr_convert_band_number_to_index(band_list[band_index]);
        if (normal_band_index == ERROR)
        {
            IAS_LOG_ERROR("Converting the band number to an index");
            return ERROR;
        }

        /* Add the SCA info to the cpf attribute name */
        status = snprintf(attribute[count], sizeof(attribute[count]), 
                    "SCA_Offset_B%02d", band_number);
        if (status < 0 || status >= sizeof(attribute[count]))
        { 
            IAS_LOG_ERROR("Creating SCA_Offset attribute string");
            return ERROR;
        }

        /* Retrieve the sca offset values */
        list[count].group_name = group_name;
        list[count].attribute = attribute[count];
        list[count].parm_ptr = focal_plane->sca_offset[normal_band_index];
        list[count].parm_size = nscas * sizeof(int);
        list[count].parm_type = IAS_ODL_Int;
        list[count].parm_count = nscas;
        count++;            /* increment the list counter */

        
        /* Add the SCA info to the cpf attribute name */
        status = snprintf(attribute[count], sizeof(attribute[count]), 
                    "Nominal_Fill_Offset_B%02d", band_number);
        if (status < 0 || status >= sizeof(attribute[count]))
        { 
            IAS_LOG_ERROR("Creating Nominal_Fill_Offset attribute string");
            return ERROR;
        }

        /* Populate the table of CPF list parameters */
        list[count].group_name = group_name;
        list[count].attribute = attribute[count];
        list[count].parm_ptr 
                   = focal_plane->nominal_fill_offset[normal_band_index];
        list[count].parm_size = nscas * sizeof(int);
        list[count].parm_type = IAS_ODL_Int;
        list[count].parm_count = nscas;
        count++;

        /* Add the SCA info to the cpf attribute name */
        status = snprintf(attribute[count], sizeof(attribute[count]), 
                    "Primary_Row_Offset_B%02d", band_number);
        if (status < 0 || status >= sizeof(attribute[count]))
        { 
            IAS_LOG_ERROR("Creating Primary_Row_Offset attribute string");
            return ERROR;
        }

        /* retrieve the primary row offsets */
        list[count].group_name = group_name;
        list[count].attribute = attribute[count];
        list[count].parm_ptr 
                = focal_plane->primary_row_offsets[normal_band_index];
        list[count].parm_size = nscas * sizeof(int);
        list[count].parm_type = IAS_ODL_Int;
        list[count].parm_count = nscas;
        count++;

        /* Add the SCA info to the cpf attribute name */
        status = snprintf(attribute[count], sizeof(attribute[count]), 
                    "Alternate_Row_Offset_B%02d", band_number);
        if (status < 0 || status >= sizeof(attribute[count]))
        { 
            IAS_LOG_ERROR("Creating Alternate_Row_Offset attribute string");
            return ERROR;
        }

        /* retrieve the alternate row offsets */
        list[count].group_name = group_name;
        list[count].attribute = attribute[count];
        list[count].parm_ptr 
                = focal_plane->alternate_row_offsets[normal_band_index];
        list[count].parm_size = nscas * sizeof(int);
        list[count].parm_type = IAS_ODL_Int;
        list[count].parm_count = nscas;
        count++;

        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            status = snprintf(attribute[count], sizeof(attribute[count]), 
                    "Along_LOS_Legendre_B%02d_SCA%02d",
                    band_number, sca_index + 1);
                if (status < 0 || status >= sizeof(attribute[count]))
                { 
                    IAS_LOG_ERROR("Creating Along_LOS_Legendre attribute "
                                  "string");
                    return ERROR;
                }

            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr 
               = focal_plane->along_los_legendre[normal_band_index][sca_index];
            list[count].parm_size = IAS_LOS_LEGENDRE_TERMS * sizeof(double);
            list[count].parm_type = IAS_ODL_Double;
            list[count].parm_count = IAS_LOS_LEGENDRE_TERMS;
            count++;

            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]), 
                    "Across_LOS_Legendre_B%02d_SCA%02d",
                    band_number, sca_index + 1);
                if (status < 0 || status >= sizeof(attribute[count]))
                { 
                    IAS_LOG_ERROR("Creating Across_LOS_Legendre attribute "
                                  "string");
                    return ERROR;
                }

            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr 
              = focal_plane->across_los_legendre[normal_band_index][sca_index];
            list[count].parm_size = IAS_LOS_LEGENDRE_TERMS * sizeof(double);
            list[count].parm_type = IAS_ODL_Double;
            list[count].parm_count = IAS_LOS_LEGENDRE_TERMS;
            count++;
        }  /* end of sca loop */
    } /* end of for band loop */

    /* Retrieve the tirs band name values */
    list[count].group_name = group_name;
    list[count].attribute = "Band_Names";
    list[count].parm_ptr = (void *) band_str;
    list[count].parm_size = sizeof(band_str);
    list[count].parm_type = IAS_ODL_ArrayOfString;
    list[count].parm_count = nbands;
    count++;            /* increment the list counter */

    /* Retrieve the band offset values */
    list[count].group_name = group_name;
    list[count].attribute = "Band_Offset";
    list[count].parm_ptr = (void *) local_band_offset;
    list[count].parm_size = sizeof(local_band_offset);
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = nbands;
    count++;            /* increment the list counter */

    /* Retrieve the detectors per band values */
    list[count].group_name = group_name;
    list[count].attribute = "Detectors_Per_Band";
    list[count].parm_ptr = (void *) local_dets_per_band;
    list[count].parm_size = sizeof(local_dets_per_band);
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = nbands;
    count++;            /* increment the list counter */

    /* Retrieve the sca offset values */
    list[count].group_name = group_name;
    list[count].attribute = "SCA_Overlap";
    list[count].parm_ptr = (void *) local_sca_overlap;
    list[count].parm_size = sizeof(local_sca_overlap);
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = nbands;
    count++;            /* increment the list counter */

    /* make a sanity check of number of parameters to retrieve */
    if (nbands * nscas * NUMBER_BAND_ATTRIBUTES + NUMBER_ATTRIBUTES != count)
    {
        IAS_LOG_ERROR("Number of parameters does not match number to retrieve");
        return ERROR;
    }


    GET_GROUP_FROM_CACHE(cpf, group_name, odl_tree);

    /* Populate the list from the odl tree */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting tirs focal plane group: %s from CPF",
                        group_name);
        /* free allocated memory */
        for (i = 0; i < nbands; i++)
            free(band_str[i]); 
            
        DROP_ODL_TREE(odl_tree);
        return ERROR;
    }

    DROP_ODL_TREE(odl_tree);

    /* copy the band level parameters to correct buckets of cpf structure */
    for (i = 0; i < nbands; i++)
    {
        normal_band_index 
            = ias_sat_attr_convert_band_number_to_index(band_list[i]);
        if (normal_band_index == ERROR)
        {
            IAS_LOG_ERROR("Converting the band number to an index");
            return ERROR;
        }
        focal_plane->sca_overlap[normal_band_index] = local_sca_overlap[i];
        focal_plane->band_offset[normal_band_index] = local_band_offset[i];
        focal_plane
            ->detectors_per_band[normal_band_index] = local_dets_per_band[i];
        strncpy(focal_plane->band_names[normal_band_index], 
                                        band_str[i], IAS_BAND_NAME_SIZE);

        /* free memory */
        free(band_str[i]); 
    }

    return SUCCESS;
}   /* end of parse tirs focal plane */
