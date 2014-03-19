/*
 * read_parameter.c
 *
 *  Created on: Mar 12, 2014
 *      Author: LYL
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ias_logging.h"
#include "ias_parm.h"
#include "ias_parm_standard_parameters.h"
#include "read_parameter.h"


//int read_parameters
//(
//    const char *parm_filename, 		 /* I: Parameter filename */
//    PARAMETERS *parameters,          /* O: Parameters retrieved from parameter
//                                      	   file, OMF and CPF */
//    MQ_PARAMS *mq_params       		/* O: MQ parameters in ODL file */
//)
//{
//    int read_count = 0;         /* Parameter table size */
//    int status = 0;             /* Function return status */
//    int satellite_number;       /* Satellite number being processed */
//
//    /* Set up and read the application parameter table */
//    IAS_PARM_DECLARE_TABLE(params, 14);
//
//
//    /* Add the satellite number parameter */
//    IAS_PARM_ADD_INT(parameters_table, SATELLITE_NUMBER,
//       		"satellite number (8 stand L8) ",
//       		 IAS_PARM_OPTIONAL, IAS_PARM_NOT_ARRAY, 0, 0, 0, 1,
//       		 NULL, satellite_number , sizeof(satellite_number), 0 );
//
//
//
//    /* Get the work order directory */
//    IAS_PARM_ADD_STRING(parameters_table, WO_DIRECTORY,
//                     "IAS Work Order Directory", IAS_PARM_REQUIRED,
//                     0, NULL, 0, NULL, parameters->work_order_directory,
//                     sizeof(parameters->work_order_directory), 0);
//
//
//    /* Get the log directory */
//    IAS_PARM_ADD_STRING(parameters_table, LOG_DIRECTORY,
//                     "IAS log Directory", IAS_PARM_REQUIRED,
//                     0, NULL, 0, NULL, parameters->log_filename,
//                     sizeof(parameters->log_filename), 0);
//
//
//    /* Get the cpf file directory */
//    IAS_PARM_ADD_STRING(parameters_table, CPF_DIRECTORY,
//                     "IAS cpf file Directory", IAS_PARM_REQUIRED,
//                     0, NULL, 0, NULL, parameters->cpf_filename,
//                     sizeof(parameters->cpf_filename), 0);
//
//
//    /* Get the ephemeris file directory */
//    IAS_PARM_ADD_STRING(parameters_table, EPHEMERIS_DIRECTORY,
//                     "IAS ephemeris file Directory", IAS_PARM_REQUIRED,
//                     0, NULL, 0, NULL, parameters->ephemeris_filename,
//                     sizeof(parameters->ephemeris_filename), 0);
//
//
//    /* Get the mwdImage file directory */
//    IAS_PARM_ADD_STRING(parameters_table, MWDIMAGE_DIRECTORY,
//                     "IAS mwdImage file Directory", IAS_PARM_REQUIRED,
//                     0, NULL, 0, NULL, parameters->mwdImage_filename,
//                     sizeof(parameters->mwdImage_filename), 0);
//
//
//    /* Get the output file directory */
//    IAS_PARM_ADD_STRING(parameters_table, OUTPUT_DIRECTORY,
//                     "IAS output file Directory", IAS_PARM_REQUIRED,
//                     0, NULL, 0, NULL, parameters->output_filename,
//                     sizeof(parameters->output_filename), 0);
//
//
//    /* MQ OutputDir */
//    const char *default_OutputDir[] = {"."};
//    IAS_PARM_ADD_STRING(parameters_table, OutputDir,
//            "Output file directory", IAS_PARM_OPTIONAL,
//            0, NULL, 1, default_OutputDir,
//            mq_params->OutputDir,
//            sizeof(mq_params->OutputDir), 1);
//
//
//    /* MQ Order ID */
//    const char *default_OrderId[] = {"L1"};
//    IAS_PARM_ADD_STRING(parameters_table, OrderId,
//            "OrderId", IAS_PARM_OPTIONAL,
//            0, NULL, 1, default_OrderId,
//            mq_params->OrderId,
//            sizeof(mq_params->OrderId), 1);
//
//
//    /* MQ Host */
//    const char *default_Host[] = {"10.4.2.10"};
//    IAS_PARM_ADD_STRING(parameters_table, Host,
//            "MQ host", IAS_PARM_OPTIONAL,
//            0, NULL, 1, default_Host,
//            mq_params->Host,
//            sizeof(mq_params->Host), 1);
//
//
//    /* MQ Port */
//    const char *default_Port[] = {"5672"};
//    IAS_PARM_ADD_STRING(parameters_table, Port,
//            "MQ Port", IAS_PARM_OPTIONAL,
//            0, NULL, 1, default_Port,
//            mq_params->Port,
//            sizeof(mq_params->Port), 1);
//
//
//    /* MQ ExchangeName */
//    const char *default_ExchangeName[] = {"ldcm_request"};
//    IAS_PARM_ADD_STRING(parameters_table, ExchangeName,
//            "MQ ExchangeName", IAS_PARM_OPTIONAL,
//            0, NULL, 1, default_ExchangeName,
//            mq_params->ExchangeName,
//            sizeof(mq_params->ExchangeName), 1);
//
//
//    /* MQ UserName */
//    const char *default_UserName[] = {"guest"};
//    IAS_PARM_ADD_STRING(parameters_table, UserName,
//            "MQ UserName", IAS_PARM_OPTIONAL,
//            0, NULL, 1, default_UserName,
//            mq_params->UserName,
//            sizeof(mq_params->UserName), 1);
//
//
//    /* MQ PassWord */
//    const char *default_PassWord[] = {"guest"};
//    IAS_PARM_ADD_STRING(parameters_table, PassWord,
//            "MQ PassWord", IAS_PARM_OPTIONAL,
//            0, NULL, 1, default_PassWord,
//            mq_params->PassWord,
//            sizeof(mq_params->PassWord), 1);
//
//
//
//
//    read_count  = IAS_PARM_GET_TABLE_SIZE(parameters_table);
//    status = ias_parm_read(parm_filename, parameters_table, read_count);
//	if (status != SUCCESS)
//	{
//		IAS_LOG_ERROR("Reading application parameters from file %s",
//				parm_filename);
//		return ERROR;
//	}
//
//    /* Get the satellite ID from the satellite number provided */
//	parameters->satellite_id =
//		ias_sat_attr_get_satellite_id_from_satellite_number(satellite_number);
//	if (parameters->satellite_id != SUCCESS)
//	{
//		IAS_LOG_ERROR("Unable to determine satellite ID from satellite "
//				"number %d", satellite_number);
//		return ERROR;
//	}
//
//
//
//    return SUCCESS;
//}


int read_parameters
(
	const char *parm_filename, 		 /* I: Parameter filename */
	PARAMETERS *parameters,          /* O: Parameters retrieved from parameter
										   file, OMF and CPF */
	MQ_PARAMS *mq_params       		/* O: MQ parameters in ODL file */
)
{
	int count = 0;         /* Parameter table size */
	int status = 0;             /* Function return status */

    /*-----------------------------------------------------------------*/
    /* This is the table definition for things from the parameter file */
    /*-----------------------------------------------------------------*/
    IAS_PARM_DECLARE_TABLE( parms, 14 );

//    IAS_PARM_WORK_ORDER_ID( parms, blob->work_order_id,
//        sizeof(blob->work_order_id), 1 );

    int satellite_number = 8;
    IAS_PARM_ADD_INT( parms, SATELLITE_NUMBER,
        "satellite number (8 stand L8)", IAS_PARM_OPTIONAL,
        IAS_PARM_NOT_ARRAY, 0, 0, 0, 1, &satellite_number,
        &satellite_number,
        sizeof(satellite_number) , 0 );


    /* Add work order directory */
	 const char *default_work_order_directory[] = {"./"};
	 IAS_PARM_ADD_STRING( parms, WO_DIRECTORY, "work order directory",
		 IAS_PARM_OPTIONAL,
		 0, NULL, /* no restrictions */
		 1, default_work_order_directory, /* Default file name */
		 parameters->work_order_directory, sizeof(parameters->work_order_directory), 0 );


    /* Add the Log file name */
	 const char *default_log_directory[] = {"./"};
	 IAS_PARM_ADD_STRING( parms, LOG_DIRECTORY, "log file name",
		 IAS_PARM_OPTIONAL,
		 0, NULL, /* no restrictions */
		 1, default_log_directory, /* Default file name */
		 parameters->log_filename, sizeof(parameters->log_filename), 0 );


	/* Add the cpf file name */
	 const char *default_cpf_directory[] = {"./"};
	 IAS_PARM_ADD_STRING( parms, CPF_DIRECTORY, "cpf file name",
		 IAS_PARM_OPTIONAL,
		 0, NULL, /* no restrictions */
		 1, default_cpf_directory, /* Default file name */
		 parameters->cpf_filename, sizeof(parameters->cpf_filename), 0 );


	/* Add the ephemeris file name */
	 const char *default_ephemeris_directory[] = {"./"};
	 IAS_PARM_ADD_STRING( parms, EPHEMERIS_DIRECTORY, "ephemeris file name",
		 IAS_PARM_OPTIONAL,
		 0, NULL, /* no restrictions */
		 1, default_ephemeris_directory, /* Default file name */
		 parameters->ephemeris_filename, sizeof(parameters->ephemeris_filename), 0 );



	/* Add the mwdImage file name */
	 const char *default_mwdImage_directory[] = {"./"};
	 IAS_PARM_ADD_STRING( parms, MWDIMAGE_DIRECTORY, "mwdImage file name",
		 IAS_PARM_OPTIONAL,
		 0, NULL, /* no restrictions */
		 1, default_mwdImage_directory, /* Default file name */
		 parameters->mwdImage_filename, sizeof(parameters->mwdImage_filename), 0 );



	/* Add the output file name */
	 const char *default_output_directory[] = {"./"};
	 IAS_PARM_ADD_STRING( parms, OUTPUT_DIRECTORY, "output file name",
		 IAS_PARM_OPTIONAL,
		 0, NULL, /* no restrictions */
		 1, default_output_directory, /* Default file name */
		 parameters->output_filename, sizeof(parameters->output_filename), 0 );


	 /* Add the MQ Orderid */
	 const char *default_OutputDir[] = {"rps"};
	 IAS_PARM_ADD_STRING( parms, OUTPUTDIR, "MQ OutputDir",
		 IAS_PARM_OPTIONAL,
		 0, NULL, /* no restrictions */
		 1, default_OutputDir, /* Default file name */
		 mq_params->OutputDir, sizeof(mq_params->OutputDir), 0 );

	 /* Add the MQ Orderid */
	 const char *default_mq_order[] = {"rps"};
	 IAS_PARM_ADD_STRING( parms, ORDERID, "MQ Orderid",
		 IAS_PARM_OPTIONAL,
		 0, NULL, /* no restrictions */
		 1, default_mq_order, /* Default file name */
		 mq_params->OrderId, sizeof(mq_params->OrderId), 0 );


	 /* Add the MQ Host */
	 const char *default_mq_host[] = {"10.4.2.10"};
	 IAS_PARM_ADD_STRING( parms, HOST, "MQ Host",
		 IAS_PARM_OPTIONAL,
		 0, NULL, /* no restrictions */
		 1, default_mq_host, /* Default file name */
		 mq_params->Host, sizeof(mq_params->Host), 0 );

	 /* Add the MQ Port */
	 const char *default_mq_port[] = {"5672"};
	 IAS_PARM_ADD_STRING( parms, PORT, "MQ Port",
		 IAS_PARM_OPTIONAL,
		 0, NULL, /* no restrictions */
		 1, default_mq_port, /* Default file name */
		 mq_params->Port, sizeof(mq_params->Port), 0 );

	 /* Add the MQ ExchangeName */
	 const char *default_mq_exchangename[] = {"ldcm_request"};
	 IAS_PARM_ADD_STRING( parms, EXCHANGENAME, "MQ ExchangeName",
		 IAS_PARM_OPTIONAL,
		 0, NULL, /* no restrictions */
		 1, default_mq_exchangename, /* Default file name */
		 mq_params->ExchangeName, sizeof(mq_params->ExchangeName), 0 );

	 /* Add the MQ UserName */
	 const char *default_mq_username[] = {"guest"};
	 IAS_PARM_ADD_STRING( parms, USERNAME, "MQ UserName",
		 IAS_PARM_OPTIONAL,
		 0, NULL, /* no restrictions */
		 1, default_mq_username, /* Default file name */
		 mq_params->UserName, sizeof(mq_params->UserName), 0 );

	 /* Add the MQ Password */
	 const char *default_mq_password[] = {"guest"};
	 IAS_PARM_ADD_STRING( parms, PASSWORD, "MQ Password",
		 IAS_PARM_OPTIONAL,
		 0, NULL, /* no restrictions */
		 1, default_mq_password, /* Default file name */
		 mq_params->PassWord, sizeof(mq_params->PassWord), 0 );



    /* See if the user wanted help or a template */
    status = ias_parm_provide_help( parm_filename, parms,
        IAS_PARM_GET_TABLE_SIZE( parms ), IAS_INPUT_PARAMETERS);
    if ( status != SUCCESS )
    {
        IAS_LOG_ERROR("Returned from parameter file help");
        return ERROR;
    }
    else
    {
        /* Read the parameter file */
        count = IAS_PARM_GET_TABLE_SIZE(parms);
        status = ias_parm_read( parm_filename, parms, count );
        if ( status != SUCCESS )
        {
            IAS_LOG_ERROR( "Reading parameters from: %s",
            		parm_filename );
            return ERROR;
        }

    }

    return SUCCESS;
}  /* END read_parameters */

