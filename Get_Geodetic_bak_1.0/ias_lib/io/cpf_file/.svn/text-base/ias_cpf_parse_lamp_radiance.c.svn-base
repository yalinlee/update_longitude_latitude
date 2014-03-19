/*************************************************************************

NAME: ias_cpf_parse_lamp_radiance

PURPOSE: Retrieve the lamp radiance data group from the odl tree and insert
         into the internal cpf structure.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include <stdlib.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

#define NUMBER_ATTRIBUTES 3

static void free_lamp_rad_memory
(
    int nbands,                             /* band loop control */
    int nscas,                              /* sca loop control */
    struct IAS_CPF_LAMP_RADIANCE *lamp_rad  /* struct to free */
);

int ias_cpf_parse_lamp_radiance
(
    const IAS_CPF *cpf,                    /* I: CPF structure */
    struct IAS_CPF_LAMP_RADIANCE *lamp_rad /* O: CPF lamp radiance data */
)
{
    int nbands;                           /* total number bands */
    int band_index;                       /* band loop var */
    int band_number;                      /* Actual band number */
    int band_list[IAS_MAX_NBANDS];        /* list of band numbers */
    int status;                           /* Function return value */
    int nscas;                            /* total number scas */
    int sca_index;                        /* sca loop counter */
    int ndet;                             /* band detector count */
    int count = 0;                        /* number of list buckets */
    char group_name[] = "LAMP_RADIANCE";  /* Name of group to retrieve */

    IAS_OBJ_DESC *odl_tree;               /* ODL tree */

    /* get oli band info */
    status = ias_sat_attr_get_sensor_band_numbers(IAS_OLI, IAS_NORMAL_BAND, 
                                                  0, band_list,
                                                  IAS_MAX_NBANDS, &nbands);
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
    char attribute[nbands * nscas * NUMBER_ATTRIBUTES][ATTRIB_STRLEN];

    ODL_LIST_TYPE list[nbands * nscas * NUMBER_ATTRIBUTES];

    /* set the pointers to null */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            lamp_rad->effective_rad_backup[band_index][sca_index] = NULL;
            lamp_rad->effective_rad_pristine[band_index][sca_index] = NULL;
            lamp_rad->effective_rad_working[band_index][sca_index] = NULL;
        }
    }
    
    /* Loop through the bands */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* get band number from band index */
        band_number = band_list[band_index];

        /* get number of scas this band */
        nscas = ias_sat_attr_get_scas_per_band(band_number);
        if (nscas == ERROR)
        {
            IAS_LOG_ERROR("Getting sca count for band number: %d", 
                 band_number);
            free_lamp_rad_memory(nbands, nscas, lamp_rad);
            return ERROR;
        }

        /* loop through the scas */
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {

            /* get detector count of current band */
            ndet = ias_sat_attr_get_detectors_per_sca(band_number);
            if (ndet == ERROR)
            {
                IAS_LOG_ERROR("Getting detector count for  band number: %d", 
                               band_number);
                free_lamp_rad_memory(nbands, nscas, lamp_rad);
                return ERROR;
            }
           
            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]), 
                "%s_B%02d_SCA%02d", "Effective_Rad_Backup", band_number, 
                sca_index +1);
            if (status < 0 || status >= sizeof(attribute[count]))
            { 
                IAS_LOG_ERROR("Creating Effective_Rad_Backup attribute string");
                free_lamp_rad_memory(nbands, nscas, lamp_rad);
                return ERROR;
            }

            /* allocate space for effective radiance backup lamps in CPF */
            lamp_rad->effective_rad_backup[band_index][sca_index] 
                                              = malloc(ndet * sizeof(double));

            if (lamp_rad->effective_rad_backup[band_index][sca_index] == NULL)
            {
                IAS_LOG_ERROR("Allocating memory effective radiance "
                              "backup lamps group: %s", group_name);
                free_lamp_rad_memory(nbands, nscas, lamp_rad);
                return ERROR;
            }

            /* populate list with effective radiance lamp backup info */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr 
                = lamp_rad->effective_rad_backup[band_index][sca_index];
            list[count].parm_size = ndet * sizeof(double);
            list[count].parm_type = IAS_ODL_Double;
            list[count].parm_count = ndet;
            count++;

            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]), 
                "%s_B%02d_SCA%02d", "Effective_Rad_Working", band_number, 
                sca_index +1);
            if (status < 0 || status >= sizeof(attribute[count]))
            {
                IAS_LOG_ERROR("Creating Effective_Rad_Working attribute "
                              "string");
                free_lamp_rad_memory(nbands, nscas, lamp_rad);
                return ERROR;
            }

            /* allocate space for effective radiance working lamps in CPF */
            lamp_rad->effective_rad_working[band_index][sca_index] 
                                              = malloc(ndet *  sizeof(double));

            if (lamp_rad->effective_rad_working[band_index][sca_index] == NULL)
            {
                IAS_LOG_ERROR("Allocating memory effective radiance "
                              "working lamps group: %s", group_name);
                free_lamp_rad_memory(nbands, nscas, lamp_rad);
                return ERROR;
            }

            /* populate list with effective radiance lamp backup info */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr 
                = lamp_rad->effective_rad_working[band_index][sca_index];
            list[count].parm_size = ndet * sizeof(double);
            list[count].parm_type = IAS_ODL_Double;
            list[count].parm_count = ndet;
            count++;

            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]), 
                "%s_B%02d_SCA%02d", "Effective_Rad_Pristine", band_number, 
                sca_index + 1);
            if (status < 0 || status >= sizeof(attribute[count]))
            { 
                IAS_LOG_ERROR("Creating Effective_Rad_Pristine attribute "
                              "string");
                free_lamp_rad_memory(nbands, nscas, lamp_rad);
                return ERROR;
            }

            /* allocate space for effective radiance lamps pristine */
            lamp_rad->effective_rad_pristine[band_index][sca_index] 
                                                = malloc(ndet * sizeof(double));

            if (lamp_rad->effective_rad_pristine[band_index][sca_index] == NULL)
            {
                IAS_LOG_ERROR("Allocating memory effective radiance "
                              "lamp pristine group: %s", group_name);
                free_lamp_rad_memory(nbands, nscas, lamp_rad);
                return ERROR;
            }

            /* populate list with effective lamp radiance pristine */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr 
                    = lamp_rad->effective_rad_pristine[band_index][sca_index];
            list[count].parm_size = ndet * sizeof(double);
            list[count].parm_type = IAS_ODL_Double;
            list[count].parm_count = ndet;
            count++;
         }
    }

    GET_GROUP_FROM_CACHE(cpf, group_name, odl_tree);

    /* Populate the list from the odl tree */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting group: %s from CPF", group_name);
        DROP_ODL_TREE(odl_tree);
        free_lamp_rad_memory(nbands, nscas, lamp_rad);
        return ERROR;
    }
    
    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
}
static void free_lamp_rad_memory
(
    int nbands,                             /* band loop control */
    int nscas,                              /* sca loop control */
    struct IAS_CPF_LAMP_RADIANCE *lamp_rad
)
{
    int band_index;                 /* band loop control */
    int sca_index;                  /* sca loop control */

    for (band_index = 0; band_index < nbands; band_index++)
    {
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            free(lamp_rad->effective_rad_backup[band_index][sca_index]);
            lamp_rad->effective_rad_backup[band_index][sca_index] = NULL;
            free(lamp_rad->effective_rad_working[band_index][sca_index]);
            lamp_rad->effective_rad_working[band_index][sca_index] = NULL;
            free(lamp_rad->effective_rad_pristine[band_index][sca_index]); 
            lamp_rad->effective_rad_pristine[band_index][sca_index] = NULL;
        }
    }
}
