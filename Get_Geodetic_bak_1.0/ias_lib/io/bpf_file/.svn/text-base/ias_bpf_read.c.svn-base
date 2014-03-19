/*************************************************************************
 NAME:                     ias_bpf_read

 PURPOSE:    Opens the BPF file on the file system, allocates a memory
             block to hold the BPF data, populates the ODL "tree",
             and calls appropriate interface functions to populate the
             desired BPF group(s)

 RETURNS:    Pointer to a BPF data structure if successful
             NULL pointer if there's an error

**************************************************************************/
#include <stdlib.h>

#include "ias_bpf.h"
#include "ias_logging.h"
#include "bpf_local_defines.h"


IAS_BPF *ias_bpf_read
(
    const char *filename      /* I: name of the BPF file to read */
)
{
    IAS_BPF *bpf = NULL;      /* Pointer to BPF data structure */

    /* allocate memory for the BPF structure */
    bpf = calloc(1, sizeof(*bpf));
    if (!bpf)
    {
        IAS_LOG_ERROR("Allocating memory for the BPF structure");
        return NULL;
    }

    /* read the BPF file into memory */
    if (ias_bpf_cache_file(filename, bpf) != SUCCESS)
    {
        IAS_LOG_ERROR("Error reading BPF file from %s", filename);
        free(bpf);
        return NULL;
    }

    /* Done */
    return bpf;
}




/*-----------------------------------------------------------------------
 NAME:                ias_bpf_get_orbit_parameters

 PURPOSE:  Public interface function to retrieve the IAS_BPF_ORBIT_PARAMETERS
           information from the BPF data structure

 RETURNS:  Pointer to an IAS_BPF_ORBIT_PARAMETERS data structure if
           successful
           NULL pointer if there's an error
--------------------------------------------------------------------------*/
const struct IAS_BPF_ORBIT_PARAMETERS *ias_bpf_get_orbit_parameters
(
    IAS_BPF *bpf                       /* I/O: BPF data structure */
)
{
    if (!bpf)
    {
        IAS_LOG_ERROR("Cannot get orbit parameter data from null BPF "
            "structure");
        return NULL;
    }
    if (!bpf->orbit_parameters_loaded)
    {
        int status;

        status = ias_bpf_parse_orbit_parameters(bpf, &bpf->orbit_parameters);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading orbit parameter data from ODL tree");
            return NULL;
        }
        bpf->orbit_parameters_loaded =  1;
    }

    /* Done */
    return &bpf->orbit_parameters;
}




/*-----------------------------------------------------------------------
 NAME:               ias_bpf_get_bias_model

 PURPOSE:  Public interface function to retrieve the IAS_BPF_BIAS_MODEL
           group from the BPF data structure

 RETURNS:  Pointer to a IAS_BPF_BIAS_MODEL data structure if successful
           NULL pointer if there's an error
--------------------------------------------------------------------------*/
const struct IAS_BPF_BIAS_MODEL *ias_bpf_get_bias_model
(
    IAS_BPF *bpf                        /* I/O: BPF data structure */
)
{
    if (!bpf)
    {
        IAS_LOG_ERROR("Cannot get bias model data from null BPF structure");
        return NULL;
    }
    if (!bpf->bias_model_loaded)
    {
        int status;

        status = ias_bpf_parse_bias_model(bpf, bpf->bias_model);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading bias model data from ODL tree");
            return NULL;
        }
        bpf->bias_model_loaded = 1;
    }

    /*  Done */
    return bpf->bias_model;
}




/*-----------------------------------------------------------------------
 NAME:                ias_bpf_get_file_attributes

 PURPOSE:  Public interface function to retrieve the BPF_FILE_ATTRIBUTES
           information from the BPF data structure

 RETURNS:  Pointer to an IAS_BPF_FILE_ATTRIBUTES data structure if
           successful
           NULL pointer if there's an error
--------------------------------------------------------------------------*/
const struct IAS_BPF_FILE_ATTRIBUTES *ias_bpf_get_file_attributes
(
    IAS_BPF *bpf                       /* I/O: BPF data structure */
)
{
    if (!bpf)
    {
        IAS_LOG_ERROR("Cannot get attribute parameter data from null BPF "
            "structure");
        return NULL;
    }
    if (!bpf->file_attributes_loaded)
    {
        int status;

        status = ias_bpf_parse_file_attributes(bpf, &bpf->file_attributes);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Reading file attribute parameter data from ODL "
                "tree");
            return NULL;
        }
        bpf->file_attributes_loaded =  1;
    }

    /* Done */
    return &bpf->file_attributes;
}
