/*************************************************************************
NAME: ias_cpf_parse_diffuser_rad

PURPOSE: Retrieve the diffuser radiance data group from the odl tree and insert
         into the internal cpf structure.
        
RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include <stdlib.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"

#define NUMBER_ATTRIBUTES 4

static void free_rad_memory
(
    int nbands,                         /* number bands */
    int nscas,                          /* number scas */
    IAS_CPF_DIFFUSER_RADIANCE *diffuser_rad
);

int ias_cpf_parse_diffuser_rad
(
    const IAS_CPF *cpf,                    /* I: CPF structure */
    struct IAS_CPF_DIFFUSER_RADIANCE *diffuser_rad 
                                           /* O: cpf diffuser radiance struct*/
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
    char group_name[] = "DIFFUSER_RADIANCE"; /* group to retrieve */
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
        IAS_LOG_ERROR("Getting sensor sca count");
        return ERROR;
    }

    /* Full attribute name with band and sca names */
    char attribute[nbands * nscas * NUMBER_ATTRIBUTES][ATTRIB_STRLEN];

    ODL_LIST_TYPE list[nbands * nscas * NUMBER_ATTRIBUTES];

    /* set the pointers to null */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        for (sca_index = 0; sca_index <  nscas; sca_index++)
        {
            diffuser_rad->diff_bidir_refl_prim[band_index][sca_index] = NULL;
            diffuser_rad->diff_bidir_refl_pris[band_index][sca_index] = NULL;
            diffuser_rad->diff_rad_prim[band_index][sca_index] = NULL;
            diffuser_rad->diff_rad_pris[band_index][sca_index] = NULL;
        }
    }
    
    /* Loop thru the bands */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* get band number from band index */
        band_number = band_list[band_index];
        
        /* get number of scas this band */
        nscas = ias_sat_attr_get_scas_per_band(band_number);
        if (nscas == ERROR)
        {
            IAS_LOG_ERROR("Getting sensor sca count for band:%d", band_number);
            free_rad_memory(nbands, nscas, diffuser_rad);
            return ERROR;
        }

        /* get detector count of current band */
        ndet = ias_sat_attr_get_detectors_per_sca(band_number);
        if (ndet == ERROR)
        {
            IAS_LOG_ERROR("Getting band detector count for band:%d", 
                            band_number);
            free_rad_memory(nbands, nscas, diffuser_rad);
            return ERROR;
        }

        /* handle the detector level attributes */
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            /* Add band & sca information to the cpf attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]),
                "%s_B%02d_SCA%02d", "Diff_Bidir_Refl_Prim", band_number, 
                sca_index + 1);
            if (status < 0 || status >= sizeof(attribute[count]))
            { 
                IAS_LOG_ERROR("Creating Diff_Bidir_Refl_Prim attribute string");
                free_rad_memory(nbands, nscas, diffuser_rad);
                return ERROR;
            }
         
            /* allocate space for attribute */
            diffuser_rad->diff_bidir_refl_prim[band_index][sca_index] 
                                             = malloc(ndet * sizeof(double));
            if (diffuser_rad->diff_bidir_refl_prim[band_index][sca_index] 
                                             == NULL)
            {
                IAS_LOG_ERROR("Allocating memory diffuser bidirection "
                              "reflectance primary group: %s", group_name);
                free_rad_memory(nbands, nscas, diffuser_rad);
                return ERROR;
            }
    
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr 
                  = diffuser_rad->diff_bidir_refl_prim[band_index][sca_index];
            list[count].parm_size = ndet * sizeof(double);
            list[count].parm_type = IAS_ODL_Double;
            list[count].parm_count = ndet;
            count++;

            /* Add sca information to the cpf attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]),
                "%s_B%02d_SCA%02d", "Diff_Bidir_Refl_Pris", band_number, 
                sca_index + 1);
            if (status < 0 || status >= sizeof(attribute[count]))
            { 
                IAS_LOG_ERROR("Creating Diff_Bidir_Refl_Pris attribute string");
                free_rad_memory(nbands, nscas, diffuser_rad);
                return ERROR;
            }
        
            /* allocate space for attribute */
            diffuser_rad->diff_bidir_refl_pris[band_index][sca_index] 
                                      = malloc(ndet * sizeof(double));
            if (diffuser_rad->diff_bidir_refl_pris[band_index][sca_index] 
                                      == NULL)
            {
                IAS_LOG_ERROR("Allocating memory diffuser bidirection "
                             "reflectance pristine group: %s", group_name);
                free_rad_memory(nbands, nscas, diffuser_rad);
                return ERROR;
            }

            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr 
                  = diffuser_rad->diff_bidir_refl_pris[band_index][sca_index];
            list[count].parm_size = ndet * sizeof(double);
            list[count].parm_type = IAS_ODL_Double;
            list[count].parm_count = ndet;
            count++;

            /* Add band sca info to cpf attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]), 
                "%s_B%02d_SCA%02d", "Diff_Rad_Primary", band_number, 
                sca_index + 1);
            if (status < 0 || status >= sizeof(attribute[count]))
            { 
                IAS_LOG_ERROR("Creating Diff_Rad_Primary attribute string");
                free_rad_memory(nbands, nscas, diffuser_rad);
                return ERROR;
            }

            /* allocate space for diffuser rad primay */
            diffuser_rad->diff_rad_prim[band_index][sca_index] 
                                          = malloc(ndet * sizeof(double));
            if (diffuser_rad->diff_rad_prim[band_index][sca_index] == NULL)
            {
                IAS_LOG_ERROR("Allocating memory diffuser rad primary "
                              "group: %s", group_name);
                free_rad_memory(nbands, nscas, diffuser_rad);
                return ERROR;
            }
            
            /* populate list with diffuser radiance primary data */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr 
                = diffuser_rad->diff_rad_prim[band_index][sca_index];
            list[count].parm_size = ndet * sizeof(double);
            list[count].parm_type = IAS_ODL_Double;
            list[count].parm_count = ndet;
            count++;

            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]), 
                "%s_B%02d_SCA%02d", "Diff_Rad_Pristine", band_number, 
                sca_index + 1);
            if (status < 0 || status >= sizeof(attribute[count]))
            { 
                IAS_LOG_ERROR("Creating Diff_Rad_Pristine attribute string");
                free_rad_memory(nbands, nscas, diffuser_rad);
                return ERROR;
            }

            /* allocate space for diffuser radiance pristine */
            diffuser_rad->diff_rad_pris[band_index][sca_index] 
                                         = malloc(ndet * sizeof(double));
            if (diffuser_rad->diff_rad_pris[band_index][sca_index] == NULL)
            {
                IAS_LOG_ERROR("Allocating memory diffuser rad pristine "
                              "group: %s", group_name);
                free_rad_memory(nbands, nscas, diffuser_rad);
                return ERROR;
            }

            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr
                    = diffuser_rad->diff_rad_pris[band_index][sca_index];
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
        IAS_LOG_ERROR("Getting group: %s from CPF", group_name);
        DROP_ODL_TREE(odl_tree);
        free_rad_memory(nbands, nscas, diffuser_rad);
        return ERROR;
    }

    DROP_ODL_TREE(odl_tree);

    return SUCCESS;
}


static void free_rad_memory
(
    int nbands,                                 /* band loop control */
    int nscas,                                  /* sca loop control */
    IAS_CPF_DIFFUSER_RADIANCE *diffuser_rad
)
{
    int band_index;                 /* band loop counter */
    int sca_index;                  /* sca loop counter */

    for (band_index = 0; band_index < nbands; band_index++)
    {
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            free(diffuser_rad->diff_bidir_refl_prim[band_index][sca_index]);
            diffuser_rad->diff_bidir_refl_prim[band_index][sca_index] = NULL;
            free(diffuser_rad->diff_bidir_refl_pris[band_index][sca_index]);
            diffuser_rad->diff_bidir_refl_pris[band_index][sca_index] = NULL;
            free(diffuser_rad->diff_rad_prim[band_index][sca_index]);
            diffuser_rad->diff_rad_prim[band_index][sca_index] = NULL;
            free(diffuser_rad->diff_rad_pris[band_index][sca_index]);
            diffuser_rad->diff_rad_pris[band_index][sca_index] = NULL;
        }
    }
}
