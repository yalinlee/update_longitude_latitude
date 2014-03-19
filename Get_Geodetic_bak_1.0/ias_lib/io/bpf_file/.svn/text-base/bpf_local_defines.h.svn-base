/*************************************************************************
 NAME:                  bpf_local_defines.h

 PURPOSE: Header file defining "private" BPF data and routines
**************************************************************************/
#ifndef BPF_LOCAL_DEFINES_H
#define BPF_LOCAL_DEFINES_H

#include "ias_odl.h"
#include "ias_const.h"
#include "ias_bpf.h"

/*-----------------------------------------------------------------------
                     BPF DATA STRUCTURE
-------------------------------------------------------------------------*/


/*                The BPF data structure itself                    */
struct IAS_BPF
{
    /* Core substructures */
    struct IAS_BPF_FILE_ATTRIBUTES file_attributes;
    struct IAS_BPF_ORBIT_PARAMETERS orbit_parameters;
    struct IAS_BPF_BIAS_MODEL bias_model[IAS_BPF_NBANDS];

    /* Flags indicating which substructures have been loaded */
    int file_attributes_loaded;
    int orbit_parameters_loaded;
    int bias_model_loaded;

    /* Buffer holding entire contents of the BPF file */
    char *raw_file_buffer;
};


/* Define macros to make it easier to use the group cache */
#define GET_BPF_GROUP_FROM_CACHE(bpf, group, tree) \
    tree = ias_bpf_get_odl_tree_from_cache(bpf, group); \
    if (tree == NULL) \
    { \
        IAS_LOG_ERROR("Cannot read BPF group %s from cache", group); \
        return ERROR; \
    }

#define DROP_BPF_ODL_TREE(tree) \
    ias_odl_free_tree(tree); \
    tree = NULL




/*--------------------------------------------------------------------------
                     BPF "Parse" function prototypes
----------------------------------------------------------------------------*/
int ias_bpf_parse_file_attributes
(
    const IAS_BPF *bpf,                    /* I: BPF data structure */
    struct IAS_BPF_FILE_ATTRIBUTES *file_attributes 
                                           /* O: Populated file attributes
                                              data structure */
);


int ias_bpf_parse_orbit_parameters
(
    const IAS_BPF *bpf,                    /* I: BPF data structure */
    struct IAS_BPF_ORBIT_PARAMETERS *orbit_parameters 
                                           /* O: Populated orbit
                                              parameters data
                                              structure */
);


int ias_bpf_parse_bias_model
(
    IAS_BPF *bpf,                          /* I: BPF data structure */
    struct IAS_BPF_BIAS_MODEL *bias_model  /* O: Populated bias model
                                              parameters data structure */
);



/*-------------------------------------------------------------------------
                    Other BPF function prototypes
---------------------------------------------------------------------------*/
int ias_bpf_cache_file
(
    const char *filename,                      /* I: BPF file name */
    IAS_BPF *bpf                               /* O: Fully populated
                                                  BPF data structure */
);


IAS_OBJ_DESC *ias_bpf_get_odl_tree_from_cache
(
    const IAS_BPF *bpf,                        /* I: Fully populated
                                                  BPF data structure */
    const char *group_name                     /* I: Name of BPF group
                                                  to retrieve */
);


#endif
