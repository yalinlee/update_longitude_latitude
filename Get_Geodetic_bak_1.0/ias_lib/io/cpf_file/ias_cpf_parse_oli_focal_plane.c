/*************************************************************************

NAME: ias_cpf_parse_oli_focal_plane

PURPOSE: Retrieve the oli focal plane data group from the odl tree and insert
         into the internal cpf structure.  Will include TIRS and OLI data.
         OLI focal plane data will be loaded into the cpf structure first,
         then TIRS data will be added to where appropriate.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "ias_cpf.h"
#include "ias_const.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

#define NUMBER_ATTRIBUTES 23
#define NUMBER_BAND_ATTRIBUTES 2

int ias_cpf_parse_oli_focal_plane
(
    const IAS_CPF *cpf,                     /* I: CPF structure */
    struct IAS_CPF_FOCAL_PLANE *focal_plane /* O: CPF focal plane data */
)
{
    int status;                            /* Function return value */
    int band_index;                        /* band loop var */
    int sca_index;                         /* SCA loop var */
    int band_number;                       /* Actual band number */
    int band_list[IAS_MAX_NBANDS];         /* list of band numbers */
    int nbands;                            /* total number bands */
    int nscas;                             /* total number scas */
    int i;                                 /* Array counters */
    int count = 0;                         /* number of list buckets */
    char *band_str[IAS_MAX_NBANDS];        /* band name temp array */
    char group_name[] = "OLI_FOCAL_PLANE"; /* Name of group to retrieve */

    IAS_OBJ_DESC *odl_tree;                /* ODL tree */

    /* make sure the band_str pointers are initialized in case an error
       occurs and all of it needs to be freed */
    memset(band_str, 0, sizeof(band_str));

    /* get oli band info */
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
    char attribute[nbands * nscas * NUMBER_BAND_ATTRIBUTES 
                   + NUMBER_ATTRIBUTES][ATTRIB_STRLEN];

    ODL_LIST_TYPE list[nbands * nscas * NUMBER_BAND_ATTRIBUTES 
                       + NUMBER_ATTRIBUTES];

    GET_GROUP_FROM_CACHE(cpf, group_name, odl_tree);

    /* Loop thru the oli bands */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* get band number from band list */
        band_number = band_list[band_index];
        
        /* Add the SCA info to the cpf attribute name */
        status = snprintf(attribute[count], sizeof(attribute[count]), 
                    "SCA_Offset_B%02d", band_number);
        if (status < 0 || status >= sizeof(attribute[count]))
        { 
            IAS_LOG_ERROR("Creating SCA_Offset attribute string");
            return ERROR;
        }

        /* get number of scas this band */
        nscas = ias_sat_attr_get_scas_per_band(band_number);
        if (nscas == ERROR)
        {
            IAS_LOG_ERROR("Getting sca count for band: %d", band_number);
            return ERROR;
        }
            
        /* Retrieve the sca offset values */
        list[count].group_name = group_name;
        list[count].attribute = attribute[count];
        list[count].parm_ptr = focal_plane->sca_offset[band_index];
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

        /* retrieve the nominal fill offsets */
        list[count].group_name = group_name;
        list[count].attribute = attribute[count];
        list[count].parm_ptr = focal_plane->nominal_fill_offset[band_index];
        list[count].parm_size = nscas * sizeof(int);
        list[count].parm_type = IAS_ODL_Int;
        list[count].parm_count = nscas;
        count++;

        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            status = snprintf(attribute[count], sizeof(attribute[count]), 
                    "Along_LOS_Legendre_B%02d_SCA%02d", band_number, 
                    sca_index + 1);
            if (status < 0 || status >= sizeof(attribute[count]))
            { 
                IAS_LOG_ERROR("Creating Along_LOS_Legendre attribute string");
                return ERROR;
            }

            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr 
                   = &focal_plane->along_los_legendre[band_index][sca_index];
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
                IAS_LOG_ERROR("Creating Across_LOS_Legendre attribute string");
                return ERROR;
            }

            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr 
                    = &focal_plane->across_los_legendre[band_index][sca_index];
            list[count].parm_size = IAS_LOS_LEGENDRE_TERMS * sizeof(double);
            list[count].parm_type = IAS_ODL_Double;
            list[count].parm_count = IAS_LOS_LEGENDRE_TERMS;
            count++;
        }
    }

    /* Retrieve the oli detectors per band count */
    list[count].group_name = group_name;
    list[count].attribute = "Detectors_Per_Band";
    list[count].parm_ptr = &focal_plane->detectors_per_band;
    list[count].parm_size = sizeof(focal_plane->detectors_per_band);
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = nbands;
    count++;            /* increment the list counter */

    /* Retrieve the oli band name values */
    list[count].group_name = group_name;
    list[count].attribute = "Band_Names";
    list[count].parm_ptr = (void *) band_str;
    list[count].parm_size = sizeof(band_str);
    list[count].parm_type = IAS_ODL_ArrayOfString;
    list[count].parm_count = nbands;
    count++;            /* increment the list counter */

    /* Retrieve the sca overlap values */
    list[count].group_name = group_name;
    list[count].attribute = "SCA_Overlap";
    list[count].parm_ptr = &focal_plane->sca_overlap;
    list[count].parm_size = sizeof(focal_plane->sca_overlap);
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = nbands;
    count++;            /* increment the list counter */

    /* Retrieve the band offset values */
    list[count].group_name = group_name;
    list[count].attribute = "Band_Offset";
    list[count].parm_ptr = &focal_plane->band_offset;
    list[count].parm_size = sizeof(focal_plane->band_offset);
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = nbands;
    count++;            /* increment the list counter */

    /* Retrieve the band order values */
    list[count].group_name = group_name;
    list[count].attribute = "Band_Order";
    list[count].parm_ptr = &focal_plane->band_order;
    list[count].parm_size = sizeof(focal_plane->band_order);
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = nbands;
    count++;            /* increment the list counter */

    /* make a sanity check of number of parameters to retrieve */
    if ((nbands * nscas * NUMBER_BAND_ATTRIBUTES + NUMBER_ATTRIBUTES) != count)
    {
        IAS_LOG_ERROR("Number of parameters does not match number to retrieve");
        return ERROR;
    }

    /* Populate the list from the odl tree */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting group: %s from CPF", group_name);
        for (i = 0; i < nbands; i++)
            free(band_str[i]); 

        DROP_ODL_TREE(odl_tree);
        return ERROR;
    }

    /* copy the band order strings into the cpf structure */
    for (i = 0; i < nbands; i++)
    {
        strncpy(focal_plane->band_names[i], band_str[i], 
               IAS_BAND_NAME_SIZE);
        free(band_str[i]); 
    }

    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
}   /* end of parse oli focal plane */
