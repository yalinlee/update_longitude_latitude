/*************************************************************************

NAME: ias_cpf_parse_earth_constants

PURPOSE: Retrieve the earth constants data group from the 
         odl tree and insert into internal cpf structure

RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include <stdlib.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"

typedef struct TEMP_SORT_RECORD
{
    int leap_years;
    int leap_months;
    int leap_days;
    int num_leap_seconds;
} TEMP_SORT_RECORD;

/******************************************************************************
NAME: compare_sortdates

PURPOSE:
Comparsion function for qsort (to sort by the leap second date).

RETURN VALUE:   Type = int
Value    Description
-----    -----------
  1      The first leap second date is greater than the second.
 -1      The first leap second date is less than the second.
  0      The first leap second date is equal to the second.
******************************************************************************/
static int compare_sortdates
(
    const void *r1,
    const void *r2
)
{
    const TEMP_SORT_RECORD * const a = (TEMP_SORT_RECORD *)r1;
    const TEMP_SORT_RECORD * const b = (TEMP_SORT_RECORD *)r2;
    int sortdate1, sortdate2;
    sortdate1 = a->leap_years * 10000 + a->leap_months * 100 + a->leap_days;
    sortdate2 = b->leap_years * 10000 + b->leap_months * 100 + b->leap_days;
    if (sortdate1 > sortdate2)
        return 1;
    else if (sortdate1 < sortdate2)
        return -1;
    else
        return 0;
}
int ias_cpf_parse_earth_const
(
    const IAS_CPF *cpf,                   /* I: CPF structure */
    struct IAS_CPF_EARTH_CONSTANTS *earth /* O: CPF earth constants data */
)
{
    int status;                      /* Function return value */
    char group_name[] = "EARTH_CONSTANTS"; /* Name of group to retrieve */
    char **month_str;                /* Temp months */
    TEMP_SORT_RECORD *sortarea;      /* Used to sort the leap second data */
    int count;                       /* Number of group attributes */
    int nsecs;                       /* number leap seconds in cpf */
    int index;                       /* loop variable */
    IAS_OBJ_DESC *odl_tree;          /* ODL tree */

    ODL_LIST_TYPE list[] =           /* Struct to contain group attributes */
    {
        {group_name, "Ellipsoid_Name", &earth->ellipsoid_name,
         sizeof(earth->ellipsoid_name), IAS_ODL_String, 1},

        {group_name, "Semi_Major_Axis", &earth->semi_major_axis,
         sizeof(earth->semi_major_axis), IAS_ODL_Double, 1},

        {group_name, "Semi_Minor_Axis", &earth->semi_minor_axis,
         sizeof(earth->semi_minor_axis), IAS_ODL_Double, 1},

        {group_name, "Ellipticity", &earth->ellipticity,
         sizeof(earth->ellipticity), IAS_ODL_Double, 1},

        {group_name, "Eccentricity", &earth->eccentricity,
         sizeof(earth->eccentricity), IAS_ODL_Double, 1},

        {group_name, "Gravity_Constant", &earth->gravity_constant,
         sizeof(earth->gravity_constant), IAS_ODL_Double, 1},

        {group_name, "J2_Earth_Model", &earth->j2_earth_model,
         sizeof(earth->j2_earth_model), IAS_ODL_Double, 1},

        {group_name, "Earth_Angular_Velocity", &earth->earth_angular_velocity,
         sizeof(earth->earth_angular_velocity), IAS_ODL_Double, 1},

        {group_name, "Datum", &earth->datum,
         sizeof(earth->datum), IAS_ODL_String, 1},

        {group_name, "Speed_of_Light", &earth->speed_of_light,
         sizeof(earth->speed_of_light), IAS_ODL_Double, 1},

        {group_name, "Spheroid_Code", &earth->spheroid_code,
         sizeof(earth->spheroid_code), IAS_ODL_Int, 1},

        {group_name, "Number_of_Leap_Seconds", 
         &earth->leap_seconds_data.leap_seconds_count,
         sizeof(earth->leap_seconds_data.leap_seconds_count), IAS_ODL_Int, 1}
    };

    /* Calculate the number of attributes to retrieve */
    count = sizeof(list) / sizeof(ODL_LIST_TYPE);

    GET_GROUP_FROM_CACHE(cpf, group_name, odl_tree);

    /* Populate the list from the odl tree, this will get the number of
       leap seconds that have occured since 2000 so we can get the 
       year, day of year and seconds of day of each leap second. */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting CPF group: %s", group_name);
        DROP_ODL_TREE(odl_tree);
        return ERROR;
    }

    /* Retrieve the leap seconds */
    nsecs = earth->leap_seconds_data.leap_seconds_count;

    count = 0;
    /* allocate for the year bucket */
    earth->leap_seconds_data.leap_years = malloc(nsecs * sizeof(int));

    if (earth->leap_seconds_data.leap_years == NULL)
    {
        IAS_LOG_ERROR ("Allocating leap years memory group: %s", group_name);
        DROP_ODL_TREE(odl_tree);
        return ERROR;
    }

    /* populate the table of cpf list parameters */
    list[count].group_name = group_name;
    list[count].attribute = "Leap_Years";
    list[count].parm_ptr = earth->leap_seconds_data.leap_years;
    list[count].parm_size = nsecs * sizeof(int);
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = nsecs;
    count++;

    /* allocate for the month bucket */
    earth->leap_seconds_data.leap_months = malloc(nsecs * sizeof(int));
    if (earth->leap_seconds_data.leap_months == NULL)
    {
        IAS_LOG_ERROR("Allocating leap months memory group: %s", group_name);
        free(earth->leap_seconds_data.leap_years);
        earth->leap_seconds_data.leap_years = NULL;
        DROP_ODL_TREE(odl_tree);
        return ERROR;
    }

    /* allocate for the temporary month bucket */
    month_str = malloc(nsecs * sizeof(char *));
    if (month_str == NULL)
    {
        IAS_LOG_ERROR("Allocating month string array group: %s", group_name);
        free(earth->leap_seconds_data.leap_years);
        free(earth->leap_seconds_data.leap_months);
        earth->leap_seconds_data.leap_years = NULL;
        DROP_ODL_TREE(odl_tree);
        return ERROR;
    }


    /* populate the table of cpf list parameters */
    list[count].group_name = group_name;
    list[count].attribute = "Leap_Months";
    list[count].parm_ptr = (void *) month_str;
    list[count].parm_size = nsecs * sizeof(char *);
    list[count].parm_type = IAS_ODL_ArrayOfString;
    list[count].parm_count = nsecs;
    count++;

    /* allocate for the day bucket */
    earth->leap_seconds_data.leap_days = malloc(nsecs * sizeof(int));
    if (earth->leap_seconds_data.leap_days == NULL)
    {
        IAS_LOG_ERROR("Allocating leap days memory group: %s", group_name);
        DROP_ODL_TREE(odl_tree);
        free(earth->leap_seconds_data.leap_years);
        earth->leap_seconds_data.leap_years = NULL;
        free(earth->leap_seconds_data.leap_months);
        earth->leap_seconds_data.leap_months = NULL;
        free(month_str);
        return ERROR;
    }
    /* populate the table of cpf list parameters */
    list[count].group_name = group_name;
    list[count].attribute = "Leap_Days";
    list[count].parm_ptr = earth->leap_seconds_data.leap_days;
    list[count].parm_size = nsecs * sizeof(int);
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = nsecs;
    count++;

    /* allocate for the second bucket */
    earth->leap_seconds_data.num_leap_seconds = malloc(nsecs * sizeof(int));
    if (earth->leap_seconds_data.num_leap_seconds == NULL)
    {
        IAS_LOG_ERROR("Allocating leap seconds memory group: %s", group_name);
        DROP_ODL_TREE(odl_tree);
        free(earth->leap_seconds_data.leap_years);
        earth->leap_seconds_data.leap_years = NULL;
        free(earth->leap_seconds_data.leap_months);
        earth->leap_seconds_data.leap_months = NULL;
        free(earth->leap_seconds_data.leap_days);
        earth->leap_seconds_data.leap_days = NULL;
        free(month_str);
        return ERROR;
    }
    /* populate the table of cpf list parameters */
    list[count].group_name = group_name;
    list[count].attribute = "Leap_Seconds";
    list[count].parm_ptr = earth->leap_seconds_data.num_leap_seconds;
    list[count].parm_size = nsecs * sizeof(int);
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = nsecs;
    count++;

    /* get the list from the odltree */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting leap seconds group: %s from CPF", group_name);
        DROP_ODL_TREE(odl_tree);
        free(earth->leap_seconds_data.leap_years);
        earth->leap_seconds_data.leap_years = NULL;
        free(earth->leap_seconds_data.leap_months);
        earth->leap_seconds_data.leap_months = NULL;
        free(earth->leap_seconds_data.leap_days);
        earth->leap_seconds_data.leap_days = NULL;
        free(earth->leap_seconds_data.num_leap_seconds);
        earth->leap_seconds_data.num_leap_seconds = NULL;
        for (index = 0; index < nsecs; index++)
        {
            free(month_str[index]);
        }
        free(month_str);
        return ERROR;
    }

    /* Copy the month values and convert them here. */
    for (index = 0; index < nsecs; index++)
    {
        if (ias_cpf_convert_3digit_month_to_number(month_str[index],
                &earth->leap_seconds_data.leap_months[index]) != SUCCESS)
        {
            IAS_LOG_ERROR("Converting leap month [%s]", month_str[index]);
            DROP_ODL_TREE(odl_tree);
            free(earth->leap_seconds_data.leap_years);
            earth->leap_seconds_data.leap_years = NULL;
            free(earth->leap_seconds_data.leap_months);
            earth->leap_seconds_data.leap_months = NULL;
            free(earth->leap_seconds_data.leap_days);
            earth->leap_seconds_data.leap_days = NULL;
            free(earth->leap_seconds_data.num_leap_seconds);
            earth->leap_seconds_data.num_leap_seconds = NULL;
            for (index = 0; index < nsecs; index++)
            {
                free(month_str[index]);
            }
            free(month_str);
            return ERROR;
        }
    }

    for (index = 0; index < nsecs; index++)
    {
        free(month_str[index]);
    }
    free(month_str);

    DROP_ODL_TREE(odl_tree);

    if (nsecs == 0)
        return SUCCESS;

    /* Sort the leap seconds results in chronological order from oldest to
       newest. */
    sortarea = malloc(nsecs * sizeof(TEMP_SORT_RECORD));
    if (sortarea == NULL)
    {
        /* Check for allocation error. */
        IAS_LOG_ERROR("Unable to allocate memory for sorting leap seconds in "
            "group: %s from CPF", group_name);
        return ERROR;
    }
    for (index = 0; index < nsecs; index++)
    {
        /* Prepare the data for sorting. */
        sortarea[index].leap_years =
            earth->leap_seconds_data.leap_years[index];
        sortarea[index].leap_months =
            earth->leap_seconds_data.leap_months[index];
        sortarea[index].leap_days =
            earth->leap_seconds_data.leap_days[index];
        sortarea[index].num_leap_seconds =
            earth->leap_seconds_data.num_leap_seconds[index];
    }
    qsort(sortarea, nsecs, sizeof(TEMP_SORT_RECORD), compare_sortdates);
    /* Copy the sorted results back into the CPF tree. */
    for (index = 0; index < nsecs; index++)
    {
        earth->leap_seconds_data.leap_years[index] =
            sortarea[index].leap_years;
        earth->leap_seconds_data.leap_months[index] =
            sortarea[index].leap_months;
        earth->leap_seconds_data.leap_days[index] =
            sortarea[index].leap_days;
        earth->leap_seconds_data.num_leap_seconds[index] =
            sortarea[index].num_leap_seconds;
    }
    free(sortarea);
    return SUCCESS;
}
