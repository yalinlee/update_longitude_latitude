/***************************************************************************
 NAME:                          ias_bpf_cache

 PURPOSE:    The ODL routines are extremely slow when parsing large ODL files
             such as the BPF.  As an optimization, the entire BPF is read into
             an internal memory buffer.  When specific BPF group data are
             needed, the respective "get" routine calls the routines
             implemented in this module to find and parse the specified group.
             This speeds up BPF access significantly.

*****************************************************************************/
#define _GNU_SOURCE             /* Allows use of fmemopen() */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include "ias_odl.h"
#include "ias_bpf.h"
#include "ias_logging.h"
#include "bpf_local_defines.h"
#include "config.h"


#define IAS_BPF_GROUP_ENDGROUP_STRLEN 120   /* Length of a "GROUP = " or
                                               "END_GROUP = " string */


/*--------------------------------------------------------------------------
 NAME:                  ias_bpf_cache_file

 PURPOSE:  Reads the entire BPF file contents into a local memory buffer

 RETURNS:  Integer status code of SUCCESS or ERROR
----------------------------------------------------------------------------*/
int ias_bpf_cache_file
(
    const char *filename,            /* I: BPF file name */
    IAS_BPF *bpf                      /* I/O: Pointer to a populated BPF
                                        data structure */
)
{

    long bpf_file_size;              /* Size of BPF file (in bytes) */

    /* Open the BPF file */
    FILE *bpf_fptr = fopen(filename, "r");
    if (bpf_fptr == NULL)
    {
        IAS_LOG_ERROR("Cannot open BPF file %s", filename);
        return ERROR;
    }

    /* Get the size of the BPF file */
    if (fseek(bpf_fptr, 0, SEEK_END) != 0)
    {
        IAS_LOG_ERROR("Cannot determine BPF file size");
        fclose(bpf_fptr);
        return ERROR;
    }

    bpf_file_size = ftell(bpf_fptr);
    if (bpf_file_size < 0)
    {
        IAS_LOG_ERROR("Cannot determine BPF file size");
        fclose(bpf_fptr);
        return ERROR;
    }

    /* Get back to the start of the file */
    if (fseek(bpf_fptr, 0, SEEK_SET) != 0)
    {
        IAS_LOG_ERROR("Cannot return to beginning of BPF file");
        fclose(bpf_fptr);
        return ERROR;
    }

    /* Allocate BPF data structure buffer */
    bpf->raw_file_buffer = malloc((bpf_file_size + 1) * sizeof(char));
    if (bpf->raw_file_buffer == NULL)
    {
        IAS_LOG_ERROR("Cannot allocate BPF memory buffer");
        fclose(bpf_fptr);
        return ERROR;
    }

    /* Read the BPF file */
    if (fread(bpf->raw_file_buffer, sizeof(char), bpf_file_size, bpf_fptr)
               != bpf_file_size)
    {
        IAS_LOG_ERROR("Cannot read BPF file contents");
        free(bpf->raw_file_buffer);
        bpf->raw_file_buffer = NULL;
        fclose(bpf_fptr);
        return ERROR;
    }

    /* Set the last character in the buffer to the NULL terminator */
    bpf->raw_file_buffer[bpf_file_size] = '\0';

    /* Close the BPF file */
    fclose(bpf_fptr);

    /* Done */
    return SUCCESS;
}




/*-----------------------------------------------------------------------
 NAME:              ias_bpf_get_odl_tree_from_cache

 PURPOSE:  Finds the requested BPF group data within the raw file buffer
           and parses it into an ODL tree for parsing

 RETURNS:  Pointer to the ODL tree if successful
           NULL pointer if an error occurs
-------------------------------------------------------------------------*/
IAS_OBJ_DESC *ias_bpf_get_odl_tree_from_cache
(
    const IAS_BPF *bpf,                 /* I: BPF data to use */
    const char *group_name              /* I: Name of BPF group to get */
)
{
    char start_string[IAS_BPF_GROUP_ENDGROUP_STRLEN];
    char end_string[IAS_BPF_GROUP_ENDGROUP_STRLEN];
    char *start = NULL;
    char *end = NULL;

    int group_size;

    IAS_OBJ_DESC *tree;


    /* Create the defining "GROUP = ..." and "END_GROUP = ..."
       strings */
    sprintf(start_string, "GROUP = %s", group_name);
    sprintf(end_string, "END_GROUP = %s", group_name);

    /* Search the buffer data for the start of the requested group */
    start = strstr(bpf->raw_file_buffer, start_string);
    if (start == NULL)
    {
       IAS_LOG_ERROR("Group %s not found in BPF", group_name);
       return NULL;
    }

    /* Search the buffer data for the end of the requested group */
    end = strstr(start, end_string);
    if (end == NULL)
    {
        IAS_LOG_ERROR("BPF group %s not properly terminated", group_name);
        return NULL;
    }

    /* Calculate the size of the buffer required for the group */
    group_size = end - start + strlen(end_string);

#ifdef HAVE_FMEMOPEN
    /* Convert the buffer for the group into a FILE so the ODL library
       can be used */
    FILE *mem_fptr = fmemopen(start, group_size, "r");
    if (mem_fptr == NULL)
    {
        IAS_LOG_ERROR("Cannot open BPF memory file");
        return NULL;
    }

    tree = ias_odl_parse_file(NULL, mem_fptr);
    if (tree == NULL)
    {
        IAS_LOG_ERROR("Cannot parse group %s from BPF file", group_name);
        fclose(mem_fptr);
        return NULL;
    }

    /* Close the memory file */
    fclose(mem_fptr);
#else
    /* no fmemopen routine so do it the hard way of calling OdlParseLabelString
       which ends up writing a temporary file */
    
    /* allocate a temporary buffer for the group */
    char *buffer = calloc((group_size + 1), sizeof(char));
    if (!buffer)
    {
        IAS_LOG_ERROR("Failed to allocate memory");
        return NULL;
    }

    /* copy the string to the buffer and terminate it */
    memcpy(buffer, start, group_size);
    buffer[group_size] = '\0';

    tree = ias_odl_parse_label_string(buffer);
    if (!tree)
    {
        IAS_LOG_ERROR("Error parsing group %s from CPF file", group_name);
        free(buffer);
        return NULL;
    }

    free(buffer);
#endif

    /* Done */
    return tree;
}
