/*************************************************************************

NAME: ias_cpf_parse_cloud_cover_assessment

PURPOSE: Retrieve the cloud cover assessment data group from the
         odl tree and insert into the internal cpf strucure.

RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "ias_cpf.h"
#include "ias_logging.h"
#include "local_defines.h"

static void free_cc_assessment_memory
(
    struct IAS_CPF_CLOUD_COVER_ASSESSMENT *cc_assessment,
    int nbr_classes,
    int nbr_algorithms,
    int nbr_weights,
    char *weight_names[]
);


int ias_cpf_parse_cloud_cover_assessment
(
    const IAS_CPF *cpf,             /* I: CPF structure */
    struct IAS_CPF_CLOUD_COVER_ASSESSMENT *cc_assessment
                                    /* O: CPF cloud cover params */
)
{
    int status;                     /* Status of return from function */
    int len;                        /* Length of return from snprintf */
    int str_len;                    /* Length of weight string */
    int list_size;                  /* Size of the list */
    int count = 0;                  /* Number of items in CPF_LIST_TYPE */
    int i = 0;
    char group_name[] = "CLOUD_COVER_ASSESSMENT";
                                    /* Group to retrieve from the CPF */
    char **weight_names = NULL;     /* Weight Name Buffer */
    double *weight_buff;            /* Weight Buffer */
    IAS_OBJ_DESC *odl_tree;         /* ODL tree */
    ODL_LIST_TYPE *list;            /* Struct to contain group attributes */

    /* allocate for the list */
    list_size = 3;
    list = malloc(list_size * sizeof(ODL_LIST_TYPE));
    if ( list == NULL)
    {
        IAS_LOG_ERROR("Allocating first list group: %s", group_name);
        return ERROR;
    }

    /* start count */
    count = 0;
    /* populate the list for Number_Of_Classes */
    list[count].group_name = group_name;
    list[count].attribute = "Number_Of_Classes";
    list[count].parm_ptr = &cc_assessment->number_of_classes;
    list[count].parm_size = sizeof(cc_assessment->number_of_classes);
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = 1;
    count++;
    /* populate the list for Number_Of_Algorithms */
    list[count].group_name = group_name;
    list[count].attribute = "Number_Of_Algorithms";
    list[count].parm_ptr = &cc_assessment->number_of_algorithms;
    list[count].parm_size = sizeof(cc_assessment->number_of_algorithms);
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = 1;
    count++;
    /* populate the list for Cirrus_Threshold */
    list[count].group_name = group_name;
    list[count].attribute = "Cirrus_Threshold";
    list[count].parm_ptr = &cc_assessment->cirrus_threshold;
    list[count].parm_size = sizeof(cc_assessment->cirrus_threshold);
    list[count].parm_type = IAS_ODL_Double;
    list[count].parm_count = 1;
    count++;

    /* get the odl tree */
    GET_GROUP_FROM_CACHE(cpf, group_name, odl_tree);

    /* populate the cc_assessment from the odl tree */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting group: %s from CPF", group_name);
        DROP_ODL_TREE(odl_tree);
        free(list);
        return ERROR;
    }
    free(list);

    /* new allocate for the list */
    list_size = 2;
    list = malloc(list_size * sizeof(ODL_LIST_TYPE));
    if ( list == NULL)
    {
        IAS_LOG_ERROR("Allocating second list group: %s", group_name);
        DROP_ODL_TREE(odl_tree);
        return ERROR;
    }

    /* start count */
    count = 0;
    /* allocate for the algorithm_names */
    cc_assessment->algorithm_names = calloc(
                     cc_assessment->number_of_algorithms, sizeof(char *));
    if (cc_assessment->algorithm_names == NULL)
    {
        IAS_LOG_ERROR("Allocating algorithm_names array group: %s", group_name);
        DROP_ODL_TREE(odl_tree);
        free(list);
        return ERROR;
    }
    /* populate the list for Algorithm_Names */
    list[count].group_name = group_name;
    list[count].attribute = "Algorithm_Names";
    list[count].parm_ptr = (void *) cc_assessment->algorithm_names;
    list[count].parm_size = cc_assessment->number_of_algorithms * 
        sizeof(char *);
    list[count].parm_type = IAS_ODL_ArrayOfString;
    list[count].parm_count = cc_assessment->number_of_algorithms;
    count++;

    /* allocate for the run_if_thermal */
    cc_assessment->run_if_thermal = malloc(
                   cc_assessment->number_of_algorithms * sizeof(int));
    if (cc_assessment->run_if_thermal == NULL)
    {
        IAS_LOG_ERROR("Allocating run_if_thermal array group: %s", group_name);
        DROP_ODL_TREE(odl_tree);
        free_cc_assessment_memory(cc_assessment, 0, 0, 0, weight_names);
        free(list);
        return ERROR;
    }
    /* populate the list for Run_If_Thermal */
    list[count].group_name = group_name;
    list[count].attribute = "Run_If_Thermal";
    list[count].parm_ptr = cc_assessment->run_if_thermal;
    list[count].parm_size = cc_assessment->number_of_algorithms * sizeof(int);
    list[count].parm_type = IAS_ODL_Int;
    list[count].parm_count = cc_assessment->number_of_algorithms;
    count++;

    /* Populate the cc_assessment from the odl tree */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting class data - group: %s from CPF", group_name);
        DROP_ODL_TREE(odl_tree);
        free_cc_assessment_memory(cc_assessment, 0, 
             cc_assessment->number_of_algorithms, 0, weight_names);
        free(list);
        return ERROR;
    }
    free(list);

    /* new allocate for the list */
    list_size = cc_assessment->number_of_algorithms + 1;
    list = malloc(list_size * sizeof(ODL_LIST_TYPE));
    if ( list == NULL)
    {
        IAS_LOG_ERROR("Allocating third list group: %s", group_name);
        DROP_ODL_TREE(odl_tree);
        free_cc_assessment_memory(cc_assessment, 0,
             cc_assessment->number_of_algorithms, 0, weight_names);
        return ERROR;
    }

    /* start count */
    count = 0;

    /* allocate for the class type */
    cc_assessment->cca_class_type = calloc(
            cc_assessment->number_of_classes, sizeof(char *));
    if (cc_assessment->cca_class_type == NULL)
    {
        IAS_LOG_ERROR("Allocating class string array group: %s", group_name);
        DROP_ODL_TREE(odl_tree);
        free_cc_assessment_memory(cc_assessment, 0,
             cc_assessment->number_of_algorithms, 0, weight_names);
        free(list);
        return ERROR;
    }
    /* populate the list for CCA_Class_Type */
    list[count].group_name = group_name;
    list[count].attribute = "CCA_Class_Type";
    list[count].parm_ptr = (void *) cc_assessment->cca_class_type;
    list[count].parm_size = cc_assessment->number_of_classes * sizeof(char *);
    list[count].parm_type = IAS_ODL_ArrayOfString;
    list[count].parm_count = cc_assessment->number_of_classes;
    count++;

    /* allocate for the weights */
    cc_assessment->weights = calloc(cc_assessment->number_of_algorithms, 
         sizeof(double *));
    if (cc_assessment->weights == NULL)
    {
        IAS_LOG_ERROR("Allocating weights array group: %s", group_name);
        DROP_ODL_TREE(odl_tree);
        free_cc_assessment_memory(cc_assessment, 0,
             cc_assessment->number_of_algorithms, 0, weight_names);
        free(list);
        return ERROR;
    }

    /* allocate for the weight names */
    weight_names = calloc(cc_assessment->number_of_algorithms, sizeof(char *));
    if (weight_names == NULL)
    {
        IAS_LOG_ERROR("Allocating weight names array group: %s", group_name);
        DROP_ODL_TREE(odl_tree);
        free_cc_assessment_memory(cc_assessment, 0,
             cc_assessment->number_of_algorithms, 0, weight_names);
        free(list);
        return ERROR;
    }

    for (i = 0; i < cc_assessment->number_of_algorithms; i++)
    {
        if (cc_assessment->algorithm_names[i] == NULL)
        {
            IAS_LOG_ERROR("Number of algorithms and number of weight names "
                "do not match");
            DROP_ODL_TREE(odl_tree);
            free_cc_assessment_memory(cc_assessment, 0,
                 cc_assessment->number_of_algorithms, i, weight_names);
            free(list);
            return ERROR;
        }
        /* String length with space for _Weights names */
        str_len = strlen(cc_assessment->algorithm_names[i]) + 9;
        /* allocate for the weight_name */
        weight_names[i] = malloc(str_len * sizeof(char));
        if (weight_names[i] == NULL)
        {
            IAS_LOG_ERROR("Allocating memory for name for group: %s",
                group_name);
            DROP_ODL_TREE(odl_tree);
            free_cc_assessment_memory(cc_assessment, 0,
                 cc_assessment->number_of_algorithms, i, weight_names);
            free(list);
            return ERROR;
        }
        /* Create attribute name */
        len = snprintf (weight_names[i], str_len, "%s_Weights",
            cc_assessment->algorithm_names[i]);
        if ((len < 0) || len >= str_len)
        {
            IAS_LOG_ERROR("Creating %s_Weights name for group: %s",
                cc_assessment->algorithm_names[i], group_name);
            DROP_ODL_TREE(odl_tree);
            free_cc_assessment_memory(cc_assessment, 0,
                 cc_assessment->number_of_algorithms, i, weight_names);
            free(list);
            return ERROR;
        }

        /* allocate for the acca_weights */
        weight_buff = malloc(cc_assessment->number_of_classes * 
             sizeof(double));
        if (weight_buff == NULL)
        {
            IAS_LOG_ERROR("Allocating %s double array group: %s",
                weight_names[i], group_name);
            DROP_ODL_TREE(odl_tree);
            free_cc_assessment_memory(cc_assessment, 0,
                 cc_assessment->number_of_algorithms, i, weight_names);
            free(list);
            return ERROR;
        }
        /* set weights pointer */
        cc_assessment->weights[i] = weight_buff;
        /* populate the list for algorithm weight */
        list[count].group_name = group_name;
        list[count].attribute = weight_names[i];
        list[count].parm_ptr = weight_buff;
        list[count].parm_size = cc_assessment->number_of_classes *
            sizeof(double);
        list[count].parm_type = IAS_ODL_Double;
        list[count].parm_count = cc_assessment->number_of_classes;
        count++;
    }

    /* Populate the cc_assessment from the odl tree */
    status = ias_odl_get_field_list(odl_tree, list, count);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting weight data - group: %s from CPF", group_name);
        DROP_ODL_TREE(odl_tree);
        free_cc_assessment_memory(cc_assessment,
             cc_assessment->number_of_classes,
             cc_assessment->number_of_algorithms,
             cc_assessment->number_of_algorithms, weight_names);
        free(list);
        return ERROR;
    }
    /* free the weight_names stored in the list attribute */
    /* note that attribute 0 has the CCA_Class_Type so we start at 1 */
    for (i = 0; i < cc_assessment->number_of_algorithms; i++)
    {
        free(weight_names[i]);
    }
    free (weight_names);
    weight_names = NULL;
    DROP_ODL_TREE(odl_tree);
    free(list);

    return SUCCESS;
}

/*****************************************************************************

NAME: free_cc_assessment_memory

PURPOSE: Free all allocated memory when an error is encountered

RETURNS: None

******************************************************************************/
static void free_cc_assessment_memory
(
    struct IAS_CPF_CLOUD_COVER_ASSESSMENT *cc_assessment,
    int nbr_classes,
    int nbr_algorithms,
    int nbr_weights,
    char *weight_names[]
)
{
    int index;                 /* band loop control */

    free(cc_assessment->run_if_thermal);
    cc_assessment->run_if_thermal = NULL;
    for (index = 0; index < nbr_classes; index++)
    {
        free(cc_assessment->cca_class_type[index]);
        cc_assessment->cca_class_type[index] = NULL;
    }
    free(cc_assessment->cca_class_type);
    cc_assessment->cca_class_type = NULL;
    for (index = 0; index < nbr_algorithms; index++)
    {
        free(cc_assessment->algorithm_names[index]);
        cc_assessment->algorithm_names[index] = NULL;
    }
    free(cc_assessment->algorithm_names);
    cc_assessment->algorithm_names = NULL;
    for (index = 0; index < nbr_weights; index++)
    {
        if (cc_assessment->weights)
        {
            free(cc_assessment->weights[index]);
            cc_assessment->weights[index] = NULL;
        }
        if (weight_names)
        {
            free(weight_names[index]);
            weight_names[index] = NULL;
        }
    }
    free(weight_names);
    weight_names = NULL;
    free(cc_assessment->weights);
    cc_assessment->weights = NULL;
}
