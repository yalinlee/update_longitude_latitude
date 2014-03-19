/*************************************************************************
 NAME:                       ias_bpf_mem

 PURPOSE: Memory management routines for the BPF structure

**************************************************************************/
#include <stdlib.h>

#include "ias_bpf.h"
#include "ias_logging.h"
#include "bpf_local_defines.h"


/*************************************************************************
 NAME:                       ias_bpf_allocate

 PURPOSE: Allocates space for a BPF structure, assuming it will be populated
          by the caller instead of being read from a file

 RETURNS  pointer to the new IAS_BPF structure

**************************************************************************/
IAS_BPF *ias_bpf_allocate()
{
    IAS_BPF *bpf_ptr = NULL;            /* Pointer to return */

    /* Try allocating a memory block for the structure */
    bpf_ptr = calloc(1, sizeof(IAS_BPF));

    if (bpf_ptr == NULL)
    {
        IAS_LOG_ERROR("Cannot allocate BPF structure memory block");
        return NULL;
    }

    /* When allocating a BPF structure manually, assume the parameters
     * are all loaded so the library will not attempt to read an ODL file */
    bpf_ptr->orbit_parameters_loaded =  1;
    bpf_ptr->bias_model_loaded = 1;
    bpf_ptr->file_attributes_loaded =  1;

    /* Done */
    return bpf_ptr;
}




/*************************************************************************
 NAME:                       ias_bpf_free

 PURPOSE: Free all allocated BPF memory.

 RETURNS  nothing

**************************************************************************/
void ias_bpf_free
(
    IAS_BPF *bpf                        /* I: BPF data structure */
)
{
    int band_index;                     /* Band loop counter */

    if (bpf == NULL)
    {
        return;
    }

    /* Free any allocated bias model data blocks in each band */
    for (band_index = 0; band_index < IAS_BPF_NBANDS; band_index++)
    {
        if (bpf->bias_model[band_index].vnir != NULL)
            free(bpf->bias_model[band_index].vnir);
        if (bpf->bias_model[band_index].swir != NULL)
            free(bpf->bias_model[band_index].swir);
        if (bpf->bias_model[band_index].pan != NULL)
            free(bpf->bias_model[band_index].pan);
        if (bpf->bias_model[band_index].thermal != NULL)
            free(bpf->bias_model[band_index].thermal);
    }

    /* Free the buffer with the file contents */
    free(bpf->raw_file_buffer);

    /* Free the BPF structure */
    free(bpf);

    /* Done */
} 
