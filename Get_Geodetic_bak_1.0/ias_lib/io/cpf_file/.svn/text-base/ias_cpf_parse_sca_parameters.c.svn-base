/*************************************************************************

NAME: ias_cpf_parse_sca_parameters

PURPOSE: Retrieve the sca parameters data group from the odl tree and 
         insert into the internal cpf structure.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

#define NUMBER_ATTRIBUTES 4
#define NUMBER_BAND_ATTRIBUTES 1

int ias_cpf_parse_sca_parameters
(
    const IAS_CPF *cpf,                     /* I: CPF structure */
    int sensor,                             /* I: sensor, oli or tirs */
    const char *group_name,                 /* I: CPF group to get */
    struct IAS_CPF_SCA_PARAMETERS *sca_parm /* O: CPF sca params */
)
{
    int status;                     /* Function return value */
    int nbands;                     /* total number bands */
    int band_index;                 /* band loop var */
    int nscas;                      /* total number scas */
    int band_list[IAS_MAX_NBANDS];  /* list of band numbers */
    int count = 0;                  /* number of list buckets */

    IAS_OBJ_DESC *odl_tree;         /* ODL tree */

    /* get band info */
    status = ias_sat_attr_get_sensor_band_numbers(sensor, IAS_NORMAL_BAND, 
                                                  0, band_list,
                                                  IAS_MAX_NBANDS, &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting band numbers for sensor id: %d", sensor);
        return ERROR;
    }

    /* get sca count */
    nscas = ias_sat_attr_get_sensor_sca_count(sensor);
    if (nscas == ERROR)
    {
        IAS_LOG_ERROR("Getting sca count for sensor id: %d", sensor);
        return ERROR;
    }

    /* Full attribute name with band and sca names */
    char attribute[nbands * NUMBER_BAND_ATTRIBUTES 
                          + NUMBER_ATTRIBUTES][ATTRIB_STRLEN];

    ODL_LIST_TYPE list[nbands * NUMBER_BAND_ATTRIBUTES + NUMBER_ATTRIBUTES];

    count = 0;
    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* Add the band info to the CPF attribute name */
        snprintf(attribute[count], ATTRIB_STRLEN, "Discontinuity_Ratio_B%02d",
                band_list[band_index]);

        list[count].group_name = group_name;
        list[count].attribute = attribute[count];
        list[count].parm_ptr = sca_parm->discontinuity_ratio[band_index];
        list[count].parm_size = sizeof(double) * (nscas -1);
        list[count].parm_type = IAS_ODL_Double; 
        list[count].parm_count = nscas - 1;
        count++;
    }
    list[count].group_name = group_name;
    list[count].attribute = "Stripe_Cutoff";
    list[count].parm_ptr = sca_parm->stripe_cutoff;
    list[count].parm_size = sizeof(double) * nbands;
    list[count].parm_type = IAS_ODL_Double;
    list[count].parm_count = nbands;
    count++;
     
    list[count].group_name = group_name;
    list[count].attribute = "Max_Valid_Correlation_Shift"; 
    list[count].parm_ptr = sca_parm->max_valid_correlation_shift;
    list[count].parm_size = sizeof(int) * nbands;
    list[count].parm_type = IAS_ODL_Int; 
    list[count].parm_count = nbands;
    count++;

    list[count].group_name = group_name;
    list[count].attribute = "Min_Valid_Neighbor_Segments"; 
    list[count].parm_ptr = sca_parm->min_valid_neighbor_segments;
    list[count].parm_size = sizeof(int) * nbands;
    list[count].parm_type = IAS_ODL_Int; 
    list[count].parm_count = nbands;
    count++;

    list[count].group_name = group_name;
    list[count].attribute = "SCA_Overlap_Threshold";
    list[count].parm_ptr = sca_parm->sca_overlap_threshold;
    list[count].parm_size = sizeof(double) * nbands;
    list[count].parm_type = IAS_ODL_Double;
    list[count].parm_count = nbands;
    count++;

    /* make a sanity check of number of parameters to retrieve */
    if ((nbands * NUMBER_BAND_ATTRIBUTES + NUMBER_ATTRIBUTES) != count)
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
        return ERROR;
    }

    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
}
