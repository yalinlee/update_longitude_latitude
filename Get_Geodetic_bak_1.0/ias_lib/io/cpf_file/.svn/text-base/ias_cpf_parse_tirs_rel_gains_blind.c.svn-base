/*************************************************************************

NAME: ias_cpf_parse_tirs_rel_gains_blind

PURPOSE: Retrieve the tirs relative gains blind band data group from the 
         odl tree and insert into the internal cpf structure.

         NOTE: There is only one blind band, to promote reuse of previously 
         declared relative gains structure a band loop is utilized even
         though there is only one band.
        
RETURN VALUE: SUCCSSS or ERROR

******************************************************************************/

#include <stdlib.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"
#include "ias_satellite_attributes.h"

#define NUMBER_ATTRIBUTES 1

int ias_cpf_parse_tirs_rel_gains_blind
(
    const IAS_CPF *cpf,                         /* I: CPF structure */
    const char *group_name,                     /* I: group to retrieve */
    const char *attribute_prefix,               /* I: Rel_Gains_Blind,
                                                    Pre_Rel_Gains_Blind,
                                                    Post_Rel_Gains_Blind */
    struct IAS_CPF_RELATIVE_GAINS *tirs_rel_gains_blind 
                                         /* O: CPF tirs rel gains blind band */
)
{
    int nscas;                            /* number of scas */
    int nbands;                           /* number of bands */
    int sca_index;                        /* sca loop var */
    int band_index;                       /* band loop var */
    int band_list[IAS_MAX_NBANDS];        /* list of band numbers */
    int status;                           /* Function return value */
    int ndet;                             /* band detector count */
    int count = 0;                        /* number of list buckets */

    IAS_OBJ_DESC *odl_tree;               /* ODL tree */

    /* get tirs band info */
    status = ias_sat_attr_get_sensor_band_numbers(IAS_TIRS, IAS_BLIND_BAND, 
                                                  IAS_SECONDARY_BAND, band_list,
                                                  IAS_MAX_NBANDS, &nbands);
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
    /* This is for the blind band there is only one */
    char attribute[nbands * nscas * NUMBER_ATTRIBUTES][ATTRIB_STRLEN];

    ODL_LIST_TYPE list[nbands * nscas * NUMBER_ATTRIBUTES];

    /* set the pointers to null */
    /* there is only one bland band involved but to reuse structure 
       declarations we will use a band loop */
    for (band_index = 0; band_index < nbands; band_index++)
    {
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            tirs_rel_gains_blind->per_detector[band_index][sca_index] = NULL;
        }
    }
    
    for (band_index = 0; band_index < nbands; band_index++)
    {
        /* get detector count of current band */
        ndet = ias_sat_attr_get_detectors_per_sca(band_list[band_index]);
        if (ndet == ERROR)
        {
            IAS_LOG_ERROR("Getting detector count for TIRS Blind Band");
            return ERROR;
        }
        for (sca_index = 0; sca_index < nscas; sca_index++)
        {
            /* Add the band and sca information to the CPF attribute name */
            status = snprintf(attribute[count], sizeof(attribute[count]), 
                    "%s_B15_SCA%02d", attribute_prefix, sca_index + 1);
            if (status < 0 || status >= sizeof(attribute[count]))
            { 
                IAS_LOG_ERROR("Creating attribute name string");
                for (sca_index = 0; sca_index < nscas; sca_index++)
                {
                    free(tirs_rel_gains_blind
                            ->per_detector[band_index][sca_index]);
                    tirs_rel_gains_blind
                            ->per_detector[band_index][sca_index] = NULL;
                }
                return ERROR;
            }
    
            /* allocate space for Relative Gains in CPF */
            tirs_rel_gains_blind->per_detector[band_index][sca_index] 
                                = malloc(ndet * sizeof(double));
    
            if (tirs_rel_gains_blind
                    ->per_detector[band_index][sca_index] == NULL)
            {
                IAS_LOG_ERROR("Allocating memory tirs relative gains blind "
                              "band group: %s", group_name);
                for (sca_index = 0; sca_index < nscas; sca_index++)
                {
                    free(tirs_rel_gains_blind
                            ->per_detector[band_index][sca_index]);
                    tirs_rel_gains_blind
                            ->per_detector[band_index][sca_index] = NULL;
                }
                return ERROR;
            }
    
            /* populate list with relative gains info */
            list[count].group_name = group_name;
            list[count].attribute = attribute[count];
            list[count].parm_ptr 
                = tirs_rel_gains_blind->per_detector[band_index][sca_index];
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
        for (band_index = 0; band_index < nscas; band_index++)
        {
            for (sca_index = 0; sca_index < nscas; sca_index++)
            {
                free(tirs_rel_gains_blind->per_detector[band_index][sca_index]);
                tirs_rel_gains_blind
                        ->per_detector[band_index][sca_index] = NULL;
            }
        }
        return ERROR;
    }

    DROP_ODL_TREE(odl_tree);
    
    return SUCCESS;
}
