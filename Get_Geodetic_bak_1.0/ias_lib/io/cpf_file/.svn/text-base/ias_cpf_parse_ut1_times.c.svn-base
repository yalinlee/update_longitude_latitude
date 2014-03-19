/*************************************************************************

NAME: ias_cpf_parse_ut1_times

PURPOSE: Retrieve the ut1 times data group from the odl tree and insert 
         into internal cpf structure
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"

int ias_cpf_parse_ut1_times
(
    const IAS_CPF *cpf,                           /* I: CPF structure */
    struct IAS_CPF_UT1_TIME_PARAMETERS *ut1_times /* O: cpf ut1/utc time data */
)
{
    int status;                      /* Function return value */
    char group_name[] = "UT1_TIME_PARAMETERS"; /* Name of group to retrieve */
    char *month_str[IAS_UT1_PERIOD];
    int count;                       /* Number of group attributes */
    int i;                           /* loop counter */
    IAS_OBJ_DESC *odl_tree;          /* ODL tree */

    memset(month_str, 0, sizeof(month_str));

    ODL_LIST_TYPE list[] =           /* Struct to contain group attributes */
    {
        {group_name, "UT1_Year", &ut1_times->ut1_year,
         sizeof(ut1_times->ut1_year), IAS_ODL_Int, IAS_UT1_PERIOD},

        {group_name, "UT1_Month", (void *) month_str,
         sizeof(month_str), IAS_ODL_ArrayOfString, IAS_UT1_PERIOD},

        {group_name, "UT1_Day", &ut1_times->ut1_day,
         sizeof(ut1_times->ut1_day), IAS_ODL_Int, IAS_UT1_PERIOD},

        {group_name, "UT1_Modified_Julian", &ut1_times->ut1_modified_julian,
         sizeof(ut1_times->ut1_modified_julian), IAS_ODL_Int, IAS_UT1_PERIOD},

        {group_name, "UT1_X", &ut1_times->ut1_x,
         sizeof(ut1_times->ut1_x), IAS_ODL_Double, IAS_UT1_PERIOD},

        {group_name, "UT1_Y", &ut1_times->ut1_y,
         sizeof(ut1_times->ut1_y), IAS_ODL_Double, IAS_UT1_PERIOD},

        {group_name, "UT1_UTC", &ut1_times->ut1_utc,
         sizeof(ut1_times->ut1_utc), IAS_ODL_Double, IAS_UT1_PERIOD},
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
        for (i = 0; i < IAS_UT1_PERIOD; i++)
        {
            free(month_str[i]);
        }
        return ERROR;
    }

    DROP_ODL_TREE(odl_tree);

    /* get the month group out of the month_str bucket */
    for (i = 0; i < IAS_UT1_PERIOD; i++)
    {
        strncpy(ut1_times->ut1_month[i], month_str[i], 
                IAS_UT1_MONTH_STRLEN);
        free(month_str[i]);
    }

    return SUCCESS;
}
