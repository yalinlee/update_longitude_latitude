/****************************************************************************
NAME:    ias_model_io library containing the following routines:
             ias_model_read
             ias_model_write

PURPOSE: Two routines to read and write the model from/to an HDF5 file

RETURNS: SUCCESS or ERROR
*****************************************************************************/
#include "ias_logging.h"
#include "ias_miscellaneous.h"
#include "common_model_io.h"

/* for params not used in these ias_model_read_table and
   ias_model_write_table calls for non-band or sca tables */
#define NOT_USED -1
#define MODEL_FILE_TYPE "LOS Model File"

/*************************************************************************

NAME: ias_model_is_model_file

PURPOSE: Determine whether the file an LOS Model file 

RETURNS: 1 (TRUE) -- is an LOS Model file
         0 (FALSE) -- is not an LOS Model file

**************************************************************************/
int ias_model_is_model_file
(
    const char *model_filename /* I: HDF input file name */
)
{
    return ias_misc_check_file_type(model_filename, MODEL_FILE_TYPE);
}

/*************************************************************************

NAME: ias_model_read

PURPOSE: External routine to access the HDF5 model file and read the
         file contents into memory.

RETURNS: Pointer to the model structure that has been allocated and populated
         or NULL pointer if there was an error
**************************************************************************/
IAS_LOS_MODEL *ias_model_read
(
    const char *model_filename     /* I: HDF input model file name */
)
{
    IAS_LOS_MODEL *model = NULL;    /* The model struct to populate from file */
    hid_t file;                     /* File handle for HDF */
    int status;                     /* Function return status */
    hsize_t number_of_fields;       /* HDF file information holder */
    hsize_t number_of_records;      /* HDF file information holder */
    int model_format_version;


    /* Make sure the file is a model file */
    if (!ias_model_is_model_file(model_filename))
    {
        IAS_LOG_ERROR("%s is not a model file", model_filename);
        return NULL;
    }

    /* Allocate the parts of the model we can at this time, based on satellite
       attributes. Those parts not allocated here will need to be allocated
       as needed and sizing information is read from the model file. These
       parts of the model to allocate and populate later include the attitude
       and ephemeris sample records and the frame times. */
    model = ias_los_model_allocate();
    if (!model)
    {
        IAS_LOG_ERROR("Allocating model structure for file: %s",
                model_filename);
        return NULL;
    }

    /* Open the input file */
    file =  H5Fopen(model_filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file < 0)
    {
        IAS_LOG_ERROR("Opening model file: %s", model_filename);
        ias_los_model_free(model);
        return NULL;
    }

    /* Get the model format attribute, if it isn't the correct one it can't
       be read */
    if (H5LTget_attribute_int(file, "/", "Model Format Version",
                &model_format_version) < 0)
    {
        IAS_LOG_ERROR("Could not get model format version for %s",
                model_filename);
        H5Fclose(file);
        ias_los_model_free(model);
        return NULL;
    }

    if (model_format_version != MODEL_FORMAT_VERSION)
    {
        IAS_LOG_ERROR("Incorrect model format version for %s", model_filename);
        H5Fclose(file);
        ias_los_model_free(model);
        return NULL;
    }

    /* Get the general table information */
    status = H5TBget_table_info(file, GENERAL_TABLE_NAME, &number_of_fields,
            &number_of_records);
    if (status < 0 || (int)number_of_records != 1)
    {
        IAS_LOG_ERROR("Could not get general model table information for %s; "
                "retrieved %d records", model_filename, (int)number_of_records);
        H5Fclose(file);
        ias_los_model_free(model);
        return NULL;
    }

    if (ias_model_read_table(file, GENERAL_TABLE, NOT_USED, NOT_USED, model)
            != SUCCESS)
    {
        IAS_LOG_ERROR("Reading general model table for %s", model_filename);
        H5Fclose(file);
        ias_los_model_free(model);
        return NULL;
    }

    /* Get the earth model table information */
//    status = H5TBget_table_info(file, EARTH_TABLE_NAME, &number_of_fields,
//            &number_of_records);
//    if (status < 0 || (int)number_of_records != 1)
//    {
//        IAS_LOG_ERROR("Could not get earth model table information for %s; "
//                "retrieved %d records", model_filename, (int)number_of_records);
//        H5Fclose(file);
//        ias_los_model_free(model);
//        return NULL;
//    }
//
//    if (ias_model_read_table(file, EARTH_TABLE, NOT_USED, NOT_USED, model)
//            != SUCCESS)
//    {
//        IAS_LOG_ERROR("Reading model earth model table for %s", model_filename);
//        H5Fclose(file);
//        ias_los_model_free(model);
//        return NULL;
//    }

    /* Get the sensor table information */
    status = H5TBget_table_info(file, SENSOR_TABLE_NAME, &number_of_fields,
            &number_of_records);
    if (status < 0 || (int)number_of_records != IAS_MAX_SENSORS)
    {
        IAS_LOG_ERROR("Could not get model sensor table information for %s; "
                "retrieved %d records", model_filename, (int)number_of_records);
        H5Fclose(file);
        ias_los_model_free(model);
        return NULL;
    }

    if (ias_model_read_table(file, SENSOR_TABLE, NOT_USED, NOT_USED, model)
            != SUCCESS)
    {
        IAS_LOG_ERROR("Reading model sensor table for %s", model_filename);
        H5Fclose(file);
        ias_los_model_free(model);
        return NULL;
    }

    /* Get the precision table information */
//    status = H5TBget_table_info(file, PRECISION_TABLE_NAME, &number_of_fields,
//            &number_of_records);
//    if (status < 0 || number_of_records != 1)
//    {
//        IAS_LOG_ERROR("Could not get model precision table information for %s; "
//                "retrieved %d records", model_filename, (int)number_of_records);
//        H5Fclose(file);
//        ias_los_model_free(model);
//        return NULL;
//    }
//
//    if (ias_model_read_table(file, PRECISION_TABLE, NOT_USED, NOT_USED, model)
//            != SUCCESS)
//    {
//        IAS_LOG_ERROR("Reading model precision table for %s", model_filename);
//        H5Fclose(file);
//        ias_los_model_free(model);
//        return NULL;
//    }

    /* Read the image model times from epoch */
    if (ias_model_read_frame_times(file, model) != SUCCESS)
    {
        IAS_LOG_ERROR("Reading the image model times from epoch for %s",
                model_filename);
        H5Fclose(file);
        ias_los_model_free(model);
        return NULL;
    }

    if (ias_model_read_band_model(file, model) != SUCCESS)
    {
        IAS_LOG_ERROR("Reading model band model for %s", model_filename);
        H5Fclose(file);
        ias_los_model_free(model);
        return NULL;
    }

//    if (ias_model_read_attitude_model(file, &model->spacecraft.attitude)
//            != SUCCESS)
//    {
//        IAS_LOG_ERROR("Reading model attitude model for %s", model_filename);
//        H5Fclose(file);
//        ias_los_model_free(model);
//        return NULL;
//    }

//    if (ias_model_read_ephemeris_model(file, &model->spacecraft.ephemeris)
//            != SUCCESS)
//    {
//        IAS_LOG_ERROR("Reading model ephemeris model for %s", model_filename);
//        H5Fclose(file);
//        ias_los_model_free(model);
//        return NULL;
//    }

//    if (ias_model_read_ssm_models(file, &model->sensor) != SUCCESS)
//    {
//        IAS_LOG_ERROR("Reading SSM records for %s", model_filename);
//        H5Fclose(file);
//        ias_los_model_free(model);
//        return NULL;
//    }
//
//    if (ias_model_read_jitter_table(file, &model->sensor) != SUCCESS)
//    {
//        IAS_LOG_ERROR("Reading jitter table records for %s", model_filename);
//        H5Fclose(file);
//        ias_los_model_free(model);
//        return NULL;
//    }

    H5Fclose(file);
    return model;
}

/*************************************************************************

NAME: ias_model_write

PURPOSE: External routine to write the contents of the model structure to
         the HDF5 file.

RETURNS: SUCCESS -- successfully wrote structure contents to file
         ERROR -- error in writing to file
**************************************************************************/
int ias_model_write
(
    const char *model_filename,     /* I: HDF model file name to write to */
    const IAS_LOS_MODEL *los_model  /* I: LOS model structure to write */
)
{
    hid_t file;                         /* File handle for HDF */
    int model_format_version = MODEL_FORMAT_VERSION;

    /* Open the output file */
    file = H5Fcreate(model_filename, H5F_ACC_TRUNC, H5P_DEFAULT,
            H5P_DEFAULT);
    if (file < 0)
    {
        IAS_LOG_ERROR("Could not create model file: %s", model_filename);
        return ERROR;
    }

    /* Create a model format version */
    if (H5LTset_attribute_int(file, "/", "Model Format Version",
                &model_format_version, 1) < 0)
    {
        IAS_LOG_ERROR("Could not create model format version for %s",
                model_filename);
        H5Fclose(file);
        return ERROR;
    }

    /* Create and set the file type attribute */
    if (H5LTset_attribute_string(file, "/", IAS_FILE_TYPE_ATTRIBUTE,
                MODEL_FILE_TYPE) < 0)
    {
        IAS_LOG_ERROR("Writing file type attribute to file %s", model_filename);
        H5Fclose(file);
        return ERROR;
    }

    /* Write each of the tables */
    if (ias_model_write_table(file, GENERAL_TABLE, NOT_USED, NOT_USED,
                los_model) != SUCCESS)
    {
        IAS_LOG_ERROR("Writing model general table for %s", model_filename);
        H5Fclose(file);
        return ERROR;
    }

    if (ias_model_write_table(file, EARTH_TABLE, NOT_USED, NOT_USED, los_model)
            != SUCCESS)
    {
        IAS_LOG_ERROR("Writing model earth model table for %s",
                model_filename);
        H5Fclose(file);
        return ERROR;
    }

    if (ias_model_write_table(file, SENSOR_TABLE, NOT_USED, NOT_USED, los_model)
            != SUCCESS)
    {
        IAS_LOG_ERROR("Writing model sensor table for %s", model_filename);
        H5Fclose(file);
        return ERROR;
    }

    if (ias_model_write_table(file, PRECISION_TABLE, NOT_USED, NOT_USED,
                los_model) != SUCCESS)
    {
        IAS_LOG_ERROR("Writing model precision table for %s", model_filename);
        H5Fclose(file);
        return ERROR;
    }

    /* Write the dynamic array and record structures */
    if (ias_model_write_frame_times(file, los_model) != SUCCESS)
    {
        IAS_LOG_ERROR("Writing image model epoch times for %s", model_filename);
        H5Fclose(file);
        return ERROR;
    }

    if (ias_model_write_band_model(file, los_model) != SUCCESS)
    {
        IAS_LOG_ERROR("Writing image model band model for %s", model_filename);
        H5Fclose(file);
        return ERROR;
    }

    if (ias_model_write_attitude_model(file, &los_model->spacecraft.attitude)
            != SUCCESS)
    {
        IAS_LOG_ERROR("Writing image model attitude sample records for %s",
                model_filename);
        H5Fclose(file);
        return ERROR;
    }

    if (ias_model_write_ephemeris_model(file, &los_model->spacecraft.ephemeris)
            != SUCCESS)
    {
        IAS_LOG_ERROR("Writing image model ephemeris sample records for %s",
                model_filename);
        H5Fclose(file);
        return ERROR;
    }

    if (ias_model_write_ssm_model(file, &los_model->sensor) != SUCCESS)
    {
        IAS_LOG_ERROR("Writing SSM records for %s", model_filename);
        H5Fclose(file);
        return ERROR;
    }

    if (ias_model_write_jitter_table(file, &los_model->sensor) != SUCCESS)
    {
        IAS_LOG_ERROR("Writing jitter table records for %s", model_filename);
        H5Fclose(file);
        return ERROR;
    }

    H5Fclose(file);
    return SUCCESS;
}
