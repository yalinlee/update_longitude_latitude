/*************************************************************************

NAME: ias_cpf_parse_lunar_irradiance

PURPOSE: Retrieve the lunar irradiance data group from the odl tree and 
         insert into the internal cpf structure.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

int ias_cpf_parse_lunar_irradiance
(
    const IAS_CPF *cpf,                     /* I: CPF structure */
    struct IAS_CPF_LUNAR_IRRADIANCE *lunar_irrad 
                                            /* O: CPF lunar irradiance data */
)
{
    int status;                             /* Function return value */
    int count = 0;                          /* number of list buckets */
    int nbands;                             /* number of oli bands */
    int band_list[IAS_MAX_NBANDS];          /* list of band numbers */
    char group_name[] = "LUNAR_IRRADIANCE"; /* Name of group to retrieve */

    IAS_OBJ_DESC *odl_tree;                 /* ODL tree */

    /* get oli band info */
    status = ias_sat_attr_get_sensor_band_numbers(IAS_OLI, IAS_NORMAL_BAND, 
                                                  0, band_list,
                                                  IAS_MAX_NBANDS, &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting sat band attributes");
        return ERROR;
    }

    /* struct to contain group attributes */
    ODL_LIST_TYPE list[] =
    { 
        {group_name, "Median_Filter_Size", &lunar_irrad->median_filter_size,
         sizeof(lunar_irrad->median_filter_size), IAS_ODL_Int, 1},

        {group_name,"Irradiance_Conversion", 
         &lunar_irrad->irradiance_conversion,
         sizeof(lunar_irrad->irradiance_conversion), 
         IAS_ODL_Double, nbands},

        {group_name,"Integration_Threshold_Factor", 
         &lunar_irrad->integration_threshold_factor,
         sizeof(lunar_irrad->integration_threshold_factor), 
         IAS_ODL_Double, nbands},
    };

    /* Calculate the number of attributes to retrieve */
    count = sizeof(list) / sizeof(ODL_LIST_TYPE);

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
