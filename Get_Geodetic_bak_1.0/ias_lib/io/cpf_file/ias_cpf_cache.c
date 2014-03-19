/*************************************************************************

NAME: ias_cpf_cache

PURPOSE: The ODL routines are extremely slow when parsing a huge ODL file like
    the CPF.  To avoid that, the entire CPF is read into a buffer and kept in
    memory.  When specific fields are needed from the CPF, the respective
    "get" routine will use these routines to find the individual group
    needed and parse only that group with the ODL routines.  That speeds
    up the CPF access by at least two orders of magnitude.

**************************************************************************/

/* Directs the libc header files to provide the standard GNU system 
   interfaces including all GNU extentions of which fmemopen is part of. */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include "ias_odl.h"
#include "ias_cpf.h"
#include "local_defines.h"
#include "ias_logging.h"
#include "config.h"
#include "ias_satellite_attributes.h"

/*****************************************************************************
NAME:  ias_cpf_cache_file

PURPOSE: Reads the entire CPF file into a buffer to allow easily searching it
    later when groups are needed from the CPF.  The raw_file_buffer in the 
    CPF structure is used to store the file contents.

RETURNS: SUCCESS or ERROR

******************************************************************************/
int ias_cpf_cache_file
(
    const char *filename,   /* I: filename to read */
    IAS_CPF *cpf            /* I/O: pointer to the CPF structure */
)
{
    int nbands;                     /* total number bands per sensor */
    int band_list[IAS_MAX_NBANDS];  /* list of band numbers */
    int nscas;                      /* total number scas per senser */
    int status;                     /* function return */
    long size;                      /* size of file to read into cache */
    struct stat st;                 /* file stats struct to get file size */

    /* Check the local defines to ensure nothing has changed and our arrays
       have been properly sized.
       get oli band info */
    status = ias_sat_attr_get_sensor_band_numbers(IAS_OLI, IAS_NORMAL_BAND, 
                                                  0, band_list,
                                                  IAS_MAX_NBANDS, &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting oli band attributes");
        return ERROR;
    }

    /* check the oli defines */
    if (IAS_OLI_MAX_NBANDS != nbands)
    {
        IAS_LOG_ERROR("OLI band DEFINE: %d does not match satellite "
                      "attributes: %d", IAS_OLI_MAX_NBANDS, nbands);
        return ERROR;
    }

    /* get tirs band info */
    status = ias_sat_attr_get_sensor_band_numbers(IAS_TIRS, IAS_NORMAL_BAND, 
                                                  0, band_list,
                                                  IAS_MAX_NBANDS, &nbands);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Getting tirs band attributes");
        return ERROR;
    }

    /* get sca count */
    nscas = ias_sat_attr_get_sensor_sca_count(IAS_TIRS);
    if (nscas == ERROR)
    {
        IAS_LOG_ERROR("Getting sat sca count");
        return ERROR;
    }

    /* check the tirs defines */
    if (IAS_TIRS_MAX_NBANDS != nbands)
    {
        IAS_LOG_ERROR("TIRS band DEFINE: %d does not match satellite "
                      "attributes: %d", IAS_TIRS_MAX_NBANDS, nbands);
        return ERROR;
    }

    if (IAS_TIRS_MAX_NSCAS != nscas)
    {
        IAS_LOG_ERROR("TIRS sca DEFINE: %d does not match satellite "
                      "attributes: %d", IAS_TIRS_MAX_NSCAS, nscas);
        return ERROR;
    }

    /* open the file */
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        IAS_LOG_ERROR("Opening CPF file %s", filename);
        return ERROR;
    }
    
    /* get file size */
    if (stat(filename, &st) == 0)
        size = st.st_size;
    else
    {
        IAS_LOG_ERROR("Determining the size of the CPF file");
        fclose(file);
        return ERROR;
    }

    /* allocate space for the data (+ 1 for terminating zero)*/
    cpf->raw_file_buffer = malloc((size + 1) * sizeof(char));
    if (!cpf->raw_file_buffer)
    {
        IAS_LOG_ERROR("Allocating memory for the CPF raw file buffer");
        fclose(file);
        return ERROR;
    }

    /* read the file */
    if (fread(cpf->raw_file_buffer, sizeof(char), size, file) != size)
    {
        IAS_LOG_ERROR("Reading CPF file %s", filename);
        free(cpf->raw_file_buffer);
        cpf->raw_file_buffer = NULL;
        fclose(file);
        return ERROR;
    }

    /* terminate the buffer with a null terminator */
    cpf->raw_file_buffer[size] = '\0';

    /* close the file */
    fclose(file);

    return SUCCESS;
}

/*****************************************************************************
NAME:  ias_cpf_get_odl_tree_from_cache

PURPOSE: Finds the requested group in the CPF raw_file_buffer and parses it
    into an ODL tree for parsing.

RETURNS:
    Pointer to the ODL tree or NULL if an error occurs.

******************************************************************************/
IAS_OBJ_DESC *ias_cpf_get_odl_tree_from_cache
(
    const IAS_CPF *cpf,         /* I: CPF to use */
    const char *group_name      /* I: group name to look for */
)
{
    char start_string[1000];
    char end_string[1000];
    char *start;
    char *end;
    int group_size;
    int str_len;                /* start or end string length */
    int done;                   /* while loop control */
    IAS_OBJ_DESC *tree;
    int return_value;

    /* Create the strings that define the start and end of the group wanted.
       Note that this requires spaces around the '=' sign.  If that turns out
       to be a problem when people hand edit the files, this will need to be
       improved.  But for speed purposes, making the search simple helps. */
    return_value = snprintf(start_string, sizeof(start_string), "GROUP = %s", 
                   group_name);
    if (return_value < 0 || return_value >= sizeof(start_string))
    {
        IAS_LOG_ERROR("Creating the start_string");
        return NULL;
    }

    return_value = snprintf(end_string, sizeof(end_string), "END_GROUP = %s", 
                   group_name);
    if (return_value < 0 || return_value >= sizeof(end_string))
    {
        IAS_LOG_ERROR("Creating the end_string");
        return NULL;
    }

    /* search the raw buffer for the start of the wanted group */
    str_len = strlen(start_string);
    start = cpf->raw_file_buffer;
    done = 0;

    /* this loop will prevent us from selecting a group where the group_name is
       a subset of the group selected */
    while (!done)
    {
        start = strstr(start, start_string);
        if (!start)
        {
            IAS_LOG_ERROR("%s Group not found in CPF", group_name);
            return NULL;
        }
        
        /* check for new line, carriage return or space */
        if (start[str_len] == '\n' 
           || start[str_len] == '\r' || start[str_len] == ' ') 
           done = 1;        
        else
            start += str_len; /* found subset skip past it */
    }
    /* search for the end of the group */
    end = strstr(start, end_string);
    if (!end)
    {
        IAS_LOG_ERROR("%s Group not terminated in CPF", group_name);
        return NULL;
    }

    /* calculate the size of the buffer */
    group_size = end - start + strlen(end_string);

/* This code will need to be compiled on Solaris and fmemopen is not available
   on that platform so will need to handle the reading of cpf groups the 
   hard way */
#ifdef HAVE_FMEMOPEN
    /* turn the buffer for the group into a FILE so the odl library can be
       used */
    FILE *mem_file = fmemopen(start, group_size, "r");
    if (!mem_file)
    {
        IAS_LOG_ERROR("Opening CPF memory file");
        return NULL;
    }

    /* parse the tree with the ODL library */
    tree = ias_odl_parse_file(NULL, mem_file);
    if (!tree)
    {
        IAS_LOG_ERROR("Parsing group %s from CPF file", group_name);
        fclose(mem_file);
        return NULL;
    }

    /* XQian 2013.06.27 for debug */
    IAS_LOG_INFO("HAVE_FMEMOPEN is defined! \n");

    /* close the FILE */
    fclose(mem_file);
#else
    /* no fmemopen routine so do it the hard way of calling OdlParseLabelString
       which ends up writing a temporary file */
    
    /* allocate a temporary buffer for the group */
    char *buffer = malloc(group_size + 1);
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
        IAS_LOG_ERROR("Parsing group %s from CPF file", group_name);
        free(buffer);
        return NULL;
    }

    /* XQian 2013.06.27 for debug */
    IAS_LOG_INFO("HAVE_FMEMOPEN is not defined! \n");

    free(buffer);
#endif
    return tree;
}
