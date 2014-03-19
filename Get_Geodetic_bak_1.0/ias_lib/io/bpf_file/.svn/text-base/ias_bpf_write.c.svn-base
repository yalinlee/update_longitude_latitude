/*==============================================================================
NAME: ias_bpf_write

PURPOSE:
    This file contains a collections of functions that are used to write a Bias
    Parameter File (BPF) to disk as an Object Description Language (ODL) file.
==============================================================================*/

/* System Includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* IAS Includes */
#include "ias_odl.h"
#include "ias_bpf.h"
#include "ias_logging.h"
#include "bpf_local_defines.h"

/* Local Includes */


/*------------------------------------------------------------------------------
NAME: add_bias_model_pan

PURPOSE:
    Add the bias model information to an ODL object for one of the PAN band's
    SCA's.

RETURNS:
    This function returns a status of SUCCESS if the detector information is
    successfully added to the ODL structure, ERROR if an error is encountered.
------------------------------------------------------------------------------*/
static int add_bias_model_pan (
    IAS_OBJ_DESC * pOdl, /* I: ODL object */
    int evenFlag, /* I: Are these detector values for an EVEN line? */
    int band, /* I: Band number 1..n */
    int sca, /* I: SCA number 1..n */
    double detectorParms[IAS_BPF_PAN_MAX_NDET][IAS_BPF_PAN_VALUES],/*I: values*/
    double a0_coefficient /* I: A0 coefficient */
    )
{
    IAS_OBJ_DESC * pGroup;
    char           groupName[1024];
    char           parmName[1024];
    int            detector;
    int            status;

    if (evenFlag)
        sprintf(groupName, "BIAS_MODEL_EVEN_B%02d_SCA%02d", band, sca);
    else
        sprintf(groupName, "BIAS_MODEL_ODD_B%02d_SCA%02d", band, sca);

    pGroup = ias_odl_add_group(pOdl, groupName);
    if (pGroup == NULL)
    {
        IAS_LOG_ERROR("Adding group %s to BPF file", groupName);
        ias_odl_free_tree(pOdl);
        return ERROR;
    }

    for (detector = 0; detector < IAS_BPF_PAN_MAX_NDET; detector++)
    {
        sprintf(parmName, "D%03d", detector+1);
        status = ias_odl_add_field(pGroup, parmName, IAS_ODL_Double,
            (sizeof(double) * IAS_BPF_PAN_VALUES), detectorParms[detector],
            IAS_BPF_PAN_VALUES);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Adding %s to %s", parmName, groupName);
            return ERROR;
        }
    }

    /* Add the A0 coefficient for this SCA */
    status = ias_odl_add_field(pGroup, "A0_Coefficient", IAS_ODL_Double,
        sizeof(double), &a0_coefficient, 1);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Adding A0_Coefficient to %s", groupName);
        return ERROR;
    }
    return SUCCESS;
}


/*------------------------------------------------------------------------------
NAME: add_bias_model_swir

PURPOSE:
    Add the bias model information to an ODL object for one of the SWIR band's
    SCA's.

RETURNS:
    This function returns a status of SUCCESS if the detector information is
    successfully added to the ODL structure, ERROR if an error is encountered.
------------------------------------------------------------------------------*/
static int add_bias_model_swir (
    IAS_OBJ_DESC * pOdl, /* I: ODL object */
    int band, /* I: Band number 1..n */
    int sca, /* I: SCA number 1..n */
    double detectorParms[IAS_BPF_MS_MAX_NDET][IAS_BPF_SWIR_VALUES],/*I: values*/
    double a0_coefficient /* I: A0 coefficient */
    )
{
    IAS_OBJ_DESC * pGroup;
    char           groupName[1024];
    char           parmName[1024];
    int            detector;
    int            status;

    sprintf(groupName, "BIAS_MODEL_B%02d_SCA%02d", band, sca);
    pGroup = ias_odl_add_group(pOdl, groupName);
    if (pGroup == NULL)
    {
        IAS_LOG_ERROR("Adding group %s to BPF file", groupName);
        ias_odl_free_tree(pOdl);
        return ERROR;
    }

    for (detector = 0; detector < IAS_BPF_MS_MAX_NDET; detector++)
    {
        sprintf(parmName, "D%03d", detector+1);
        status = ias_odl_add_field(pGroup, parmName, IAS_ODL_Double,
            (sizeof(double) * IAS_BPF_SWIR_VALUES), detectorParms[detector],
            IAS_BPF_SWIR_VALUES);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Adding %s to %s", parmName, groupName);
            return ERROR;
        }
    }

    /* Add the A0 coefficient for this SCA */
    status = ias_odl_add_field(pGroup, "A0_Coefficient", IAS_ODL_Double,
        sizeof(double), &a0_coefficient, 1);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Adding A0_Coefficient to %s", groupName);
        return ERROR;
    }
    return SUCCESS;
}


/*------------------------------------------------------------------------------
NAME: add_bias_model_vnir

PURPOSE:
    Add the bias model information to an ODL object for one of the VNIR band's
    SCA's.

RETURNS:
    This function returns a status of SUCCESS if the detector information is
    successfully added to the ODL structure, ERROR if an error is encountered.
------------------------------------------------------------------------------*/
static int add_bias_model_vnir (
    IAS_OBJ_DESC * pOdl, /* I: ODL object */
    int band, /* I: Band number 1..n */
    int sca, /* I: SCA number 1..n */
    double detectorParms[IAS_BPF_MS_MAX_NDET][IAS_BPF_VNIR_VALUES],/*I: values*/
    double a0_coefficient /* I: A0 coefficient */
    )
{
    IAS_OBJ_DESC * pGroup;
    char           groupName[1024];
    char           parmName[1024];
    int            detector;
    int            status;

    sprintf(groupName, "BIAS_MODEL_B%02d_SCA%02d", band, sca);
    pGroup = ias_odl_add_group(pOdl, groupName);
    if (pGroup == NULL)
    {
        IAS_LOG_ERROR("Adding group %s to BPF file", groupName);
        ias_odl_free_tree(pOdl);
        return ERROR;
    }

    for (detector = 0; detector < IAS_BPF_MS_MAX_NDET; detector++)
    {
        sprintf(parmName, "D%03d", detector+1);
        status = ias_odl_add_field(pGroup, parmName, IAS_ODL_Double,
            (sizeof(double) * IAS_BPF_VNIR_VALUES), detectorParms[detector],
            IAS_BPF_VNIR_VALUES);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Adding %s to %s", parmName, groupName);
            return ERROR;
        }
    }

    /* Add the A0 coefficient for this SCA */
    status = ias_odl_add_field(pGroup, "A0_Coefficient", IAS_ODL_Double,
        sizeof(double), &a0_coefficient, 1);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Adding A0_Coefficient to %s", groupName);
        return ERROR;
    }
    return SUCCESS;
}


/*------------------------------------------------------------------------------
NAME: add_bias_model_thermal

PURPOSE:
    Add the bias model information to an ODL object for one of the thermal
    band's SCA's.

RETURNS:
    This function returns a status of SUCCESS if the detector information is
    successfully added to the ODL structure, ERROR if an error is encountered.
------------------------------------------------------------------------------*/
static int add_bias_model_thermal
(
    IAS_OBJ_DESC *pOdl,               /* I: ODL object */
    int band,                         /* I: Band number 1..n */
    int sca,                          /* I: SCA number 1..n */
    double detectorParms[IAS_BPF_THERMAL_MAX_NDET][IAS_BPF_THERMAL_VALUES]
                                      /* I: values*/
    )
{
    IAS_OBJ_DESC   *pGroup;
    char           groupName[1024];
    char           parmName[1024];
    int            detector;
    int            status;

    sprintf(groupName, "BIAS_MODEL_B%02d_SCA%02d", band, sca);
    pGroup = ias_odl_add_group(pOdl, groupName);
    if (pGroup == NULL)
    {
        IAS_LOG_ERROR("Adding group %s to BPF file", groupName);
        ias_odl_free_tree(pOdl);
        return ERROR;
    }

    for (detector = 0; detector < IAS_BPF_THERMAL_MAX_NDET; detector++)
    {
        sprintf(parmName, "D%03d", (detector + 1));
        status = ias_odl_add_field(pGroup, parmName, IAS_ODL_Double,
            (sizeof(double) * IAS_BPF_THERMAL_VALUES), detectorParms[detector],
            IAS_BPF_THERMAL_VALUES);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Adding %s to %s", parmName, groupName);
            return ERROR;
        }
    }

    return SUCCESS;
}


/*------------------------------------------------------------------------------
NAME: add_file_attributes

PURPOSE:
    This is a static function that adds a BPF's file attributes to the ODL tree
    structure.

RETURNS:
    This function returns SUCCESS if the FILE_ATTRIBUTES group is successfully
    added to the ODL object, ERROR if an error is encountered.
------------------------------------------------------------------------------*/
static int add_file_attributes (
    IAS_OBJ_DESC * pGroup, /* I: Ptr to FILE_ATTRIBUTES group in the ODL */
    struct IAS_BPF_FILE_ATTRIBUTES * pFileAttr /* I: FILE_ATTRIBUTES in BPF */
    )
{
    int len;
    int status;

    /*--------------------------------------------------------------------------
     * When adding a string parameter using the ODL library an empty string must
     * have a reported length of 1 for the library to correctly handle it.
     * Therefore, all of the string fields are checked for a length of 0 and it
     * is reset to 1 to account for this strange feature/behavior.
     *------------------------------------------------------------------------*/

    if ((len = strlen(pFileAttr->effective_date_begin)) == 0)
        len = 1;
    status = ias_odl_add_field(pGroup, "Effective_Date_Begin",
        IAS_ODL_String, len, pFileAttr->effective_date_begin, 1);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Adding Effective_Date_Begin '%s' to FILE_ATTRIBUTES",
            pFileAttr->effective_date_begin);
        return ERROR;
    }

    if ((len = strlen(pFileAttr->effective_date_end)) == 0)
        len = 1;
    status = ias_odl_add_field(pGroup, "Effective_Date_End",
        IAS_ODL_String, len, pFileAttr->effective_date_end, 1);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Adding Effective_Date_End '%s' to FILE_ATTRIBUTES",
            pFileAttr->effective_date_end);
        return ERROR;
    }

    if ((len = strlen(pFileAttr->baseline_date)) == 0)
        len = 1;
    status = ias_odl_add_field(pGroup, "Baseline_Date", IAS_ODL_String,
        len, pFileAttr->baseline_date, 1);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Adding Baseline_Date '%s' to FILE_ATTRIBUTES",
            pFileAttr->baseline_date);
        return ERROR;
    }

    if ((len = strlen(pFileAttr->file_name)) == 0)
        len = 1;
    status = ias_odl_add_field(pGroup, "File_Name", IAS_ODL_String,
        len, pFileAttr->file_name, 1);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Adding File_Name '%s' to FILE_ATTRIBUTES",
            pFileAttr->file_name);
        return ERROR;
    }

    if ((len = strlen(pFileAttr->file_source)) == 0)
        len = 1;
    status = ias_odl_add_field(pGroup, "File_Source", IAS_ODL_String,
        len, pFileAttr->file_source, 1);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Adding File_Source '%s' to FILE_ATTRIBUTES",
            pFileAttr->file_source);
        return ERROR;
    }

    if ((len = strlen(pFileAttr->spacecraft_name)) == 0)
        len = 1;
    status = ias_odl_add_field(pGroup, "Spacecraft_Name", IAS_ODL_String,
        len, pFileAttr->spacecraft_name, 1);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Adding Spacecraft_Name '%s' to FILE_ATTRIBUTES",
            pFileAttr->spacecraft_name);
        return ERROR;
    }

    if ((len = strlen(pFileAttr->sensor_name)) == 0)
        len = 1;
    status = ias_odl_add_field(pGroup, "Sensor_Name", IAS_ODL_String,
        len, pFileAttr->sensor_name, 1);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Adding Sensor_Name '%s' to FILE_ATTRIBUTES",
            pFileAttr->sensor_name);
        return ERROR;
    }

    if ((len = strlen(pFileAttr->description)) == 0)
        len = 1;
    status = ias_odl_add_field(pGroup, "Description", IAS_ODL_String,
        len, pFileAttr->description, 1);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Adding Description '%s' to FILE_ATTRIBUTES",
            pFileAttr->description);
        return ERROR;
    }

    status = ias_odl_add_field(pGroup, "Version", IAS_ODL_Int, sizeof(int),
        &pFileAttr->version, 1);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Adding Version %d to FILE_ATTRIBUTES",
            pFileAttr->version);
        return ERROR;
    }

    return SUCCESS;
}


/*------------------------------------------------------------------------------
NAME: add_orbit_parameters

PURPOSE:
    This is a static function that adds a BPF's orbit parameters to the ODL tree
    structure.

RETURNS:
    This function returns SUCCESS if the ORBIT_PARAMETERS group is successfully
    added to the ODL object, ERROR if an error is encountered.
------------------------------------------------------------------------------*/
static int add_orbit_parameters (
    IAS_OBJ_DESC * pGroup, /* I: Ptr to ORBIT_PARAMETERS group in the ODL */
    struct IAS_BPF_ORBIT_PARAMETERS * pOrbit /* I: ORBIT_PARAMETERS in BPF */
    )
{
    int status;

    status = ias_odl_add_field(pGroup, "Orbit_Number", IAS_ODL_Int, sizeof(int),
        &pOrbit->begin_orbit_number, 1);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Adding Orbit_Number %d to ORBIT_PARAMETERS",
            pOrbit->begin_orbit_number);
        return ERROR;
    }

    return SUCCESS;
}


/*------------------------------------------------------------------------------
NAME: ias_bpf_write

PURPOSE:
    Given a Bias Parameter File (BPF) structure, write that structure to a file.

RETURNS:
    SUCCESS if the function successfully writes the BPF to the specified file,
    ERROR if an error is encountered.
------------------------------------------------------------------------------*/
int ias_bpf_write
(
    IAS_BPF * bpf,           /* I: BPF structure to write to a file */
    const char *filePath     /* I: Path to an ODL file to be written */
)
{
    IAS_OBJ_DESC *pGroup;
    IAS_OBJ_DESC *pOdl;
    int           i;

    /* The file passed in cannot exist or the new ODL objects will be appended
       to it.  If the file does exist, remove it so that a new BPF file (in ODL
       format) will be created. */
    if ((access(filePath, F_OK) == 0) && unlink(filePath))
    {
        IAS_LOG_ERROR("Existing BPF cannot be overwritten: %s", filePath);
        return ERROR;
    }

    /* Create a new ODL tree for the BPF data.  This object will have all of
       the data contained in the supplied 'bpf' argument added to it. */
    pOdl = ias_odl_create_tree();
    if (pOdl == NULL)
    {
        IAS_LOG_ERROR("Creating ODL object used to write BPF");
        return ERROR;
    }

    /* Add the BPF's file attributes to the new ODL object */
    pGroup = ias_odl_add_group(pOdl, "FILE_ATTRIBUTES");
    if (pGroup == NULL)
    {
        IAS_LOG_ERROR("Adding group FILE_ATTRIBUTES to BPF file");
        ias_odl_free_tree(pOdl);
        return ERROR;
    }
    add_file_attributes(pGroup, &bpf->file_attributes);

    /* Add the BPF's orbit parameters to the new ODL object */
    pGroup = ias_odl_add_group(pOdl, "ORBIT_PARAMETERS");
    if (pGroup == NULL)
    {
        IAS_LOG_ERROR("Adding group ORBIT_PARAMETERS to BPF file");
        ias_odl_free_tree(pOdl);
        return ERROR;
    }
    add_orbit_parameters(pGroup, &bpf->orbit_parameters);

    /* Add the bias model information for each SCA, of each band to the ODL
       object. */
    for (i = 0; i < IAS_BPF_NBANDS; i++)
    {
        int band;
        int sca;
        int status = SUCCESS;

        band = bpf->bias_model[i].band_number;

        if ((band < 1) || (band > IAS_BPF_NBANDS))
        {
            IAS_LOG_WARNING("Bias model not set for band index %d: skipped", i);
            continue;
        }

        if ((bpf->bias_model[i].spectral_type == IAS_SPECTRAL_PAN) &&
            (bpf->bias_model[i].pan != NULL))
        {
            for (sca = 0; (sca < IAS_MAX_NSCAS) && (status == SUCCESS); sca++)
            {
                status = add_bias_model_pan(pOdl, TRUE, band, sca+1,
                    bpf->bias_model[i].pan->det_params_even[sca],
                    bpf->bias_model[i].pan->a0_coefficient_even[sca]);
            }
            for (sca = 0; (sca < IAS_MAX_NSCAS) && (status == SUCCESS); sca++)
            {
                status = add_bias_model_pan(pOdl, FALSE, band, sca+1,
                    bpf->bias_model[i].pan->det_params_odd[sca],
                    bpf->bias_model[i].pan->a0_coefficient_odd[sca]);
            }
        }
        else if ((bpf->bias_model[i].spectral_type == IAS_SPECTRAL_SWIR) &&
            (bpf->bias_model[i].swir != NULL))
        {
            for (sca = 0; (sca < IAS_MAX_NSCAS) && (status == SUCCESS); sca++)
            {
                status = add_bias_model_swir(pOdl, band, sca+1,
                    bpf->bias_model[i].swir->det_params[sca],
                    bpf->bias_model[i].swir->a0_coefficient[sca]);
            }
        }
        else if ((bpf->bias_model[i].spectral_type == IAS_SPECTRAL_VNIR) &&
            (bpf->bias_model[i].vnir != NULL))
        {
            for (sca = 0; (sca < IAS_MAX_NSCAS) && (status == SUCCESS); sca++)
            {
                status = add_bias_model_vnir(pOdl, band, sca+1,
                    bpf->bias_model[i].vnir->det_params[sca],
                    bpf->bias_model[i].vnir->a0_coefficient[sca]);
            }
        }
        else if ((bpf->bias_model[i].spectral_type == IAS_SPECTRAL_THERMAL)
            && (bpf->bias_model[i].thermal != NULL))
        {
            for (sca = 0;
                 (sca < IAS_BPF_THERMAL_MAX_NSCAS && (status == SUCCESS));
                 sca++)
            {
                status = add_bias_model_thermal(pOdl, band, (sca + 1),
                    bpf->bias_model[i].thermal->det_params[sca]);
            }
        }
        else
        {
            IAS_LOG_WARNING("Bias model not set for band %d: skipped", band);
            continue;
        }

        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Adding SCA detector bias information");
            ias_odl_free_tree(pOdl);
            return ERROR;
        }
    }

    /* Finally, write the ODL representation of the BPF information to the file
       */
    ias_odl_write_tree(pOdl, (char *)filePath);
    ias_odl_free_tree(pOdl);
    return SUCCESS;
}
