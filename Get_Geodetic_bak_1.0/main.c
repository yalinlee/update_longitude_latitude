/*
 * main.c
 *
 *  Created on: Mar 2, 2014
 *      Author: lyl
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ias_los_model.h"
#include "ias_geo.h"
#include "ias_model_io.h"
#include "ias_logging.h"
#include "ias_ancillary.h"
#include "read_parameter.h"
#include "read_ephemeris_data.h"
#include "read_write_mwdImage.h"
#include "threadpool.h"
#include "update_longitude_latitude.h"

#define DEBUG_GENERATE_DATA_FILES 1
#define NUM_THREAD 3





int main(int argc, char **argv)
{
	/* check the number of argument */
	if (argc != 2)
	{
		IAS_LOG_ERROR("Wrong parameter!");
		return ERROR;
	}

	/* declare some parameters */
	PARAMETERS parameters;			//structure of parameters
	MQ_PARAMS mq_params;			//structure of MQ parameters
	IAS_L0R_EPHEMERIS *l0r_ephemeris = NULL;   //pointer to L0R ephemeris data structure
	long long num_frame_of_ephemeris;			// number of ephemeris frames
	IAS_LOS_MODEL *model;						//pointer to the model structure
	IAS_CPF *cpf = NULL;          				/* Structure for CPF */
	double ephemeris_start_time;	//start time of ephemeris data
	double ephemeris_end_time;		//end time of ephemeris data
	IAS_ANC_EPHEMERIS_DATA *anc_ephemeris_data = NULL;
	int invalid_ephemeris_count = 0;
	MWDIMAGE_BUFFER_INFO *mwdImage_buffer_info;
	size_t process_times_needed;
	int status = ERROR;
	int i,j;


	memset(&parameters, 0, sizeof(parameters));
	memset(&mq_params, 0, sizeof(mq_params));
	/* Read parameters and MQ params from ODL */
	status = read_parameters(argv[1],&parameters,&mq_params);
	if(status != SUCCESS)
	{
		IAS_LOG_ERROR("failed to read parameters!\n");
		exit(EXIT_FAILURE);
	}

	/*MQ connection */
	status = MQ_Init();
	if(status != SUCCESS)
	{
		IAS_LOG_WARNING("MQ connect Error !\n");
	}

	status = MQSend(4,"ADP module started !\n");
	if(status != SUCCESS)
	{
		IAS_LOG_WARNING("module started , MQSend Error !\n");
	}



	status = ias_sat_attr_initialize(parameters.satellite_id);
	if (status != SUCCESS)
	{
		IAS_LOG_ERROR("Initializing IAS Satellite Attributes Library");
		return ERROR;
	}


	/* Initialize the model structure */
	IAS_ACQUISITION_TYPE acquisition_type = IAS_EARTH;
	model = ias_los_model_initialize(acquisition_type);
	if (!model)
	{
		IAS_LOG_ERROR("Initializing model");
		return EXIT_FAILURE;
	}


	/* read information from cpf, and set it into model*/
	cpf = ias_cpf_read(parameters.cpf_filename);

	if(ias_los_model_set_cpf_for_MWD(cpf,model) != SUCCESS)
	{
		IAS_LOG_ERROR("Copy cpf value into model");
		return ERROR;
	}


	/* read ephemeris file into lor_ephemeris */
	status = read_ephemeris_data_for_MWD(&parameters,&l0r_ephemeris,&num_frame_of_ephemeris);
	if(status != SUCCESS )
	{
		IAS_LOG_ERROR("Could not read ephemeris file into lor_ephemeris.\n");
		exit(EXIT_FAILURE);
	}



	long long l0r_ephemeris_count = num_frame_of_ephemeris;
	/* Preprocess the ephemeris data. */
	if (ias_ancillary_preprocess_ephemeris_for_MWD(cpf, l0r_ephemeris,
			l0r_ephemeris_count,acquisition_type,&anc_ephemeris_data,
			&invalid_ephemeris_count,
			&ephemeris_start_time,&ephemeris_end_time) != SUCCESS)
	{
		IAS_LOG_ERROR("Processing ephemeris data");
		return ERROR;
	}


	if(ias_sc_model_set_ancillary_ephemeris(anc_ephemeris_data,
			&model->spacecraft) != SUCCESS)
	{
		IAS_LOG_ERROR("Setting ephemeris data into model");
		return ERROR;
	}


	mwdImage_buffer_info = malloc(sizeof(*mwdImage_buffer_info));
	memset(mwdImage_buffer_info,0,sizeof(*mwdImage_buffer_info));

	UPDATE_LONGITUDE_LATITUDE_ARGS *update_longitude_latitude_args;
	update_longitude_latitude_args = (UPDATE_LONGITUDE_LATITUDE_ARGS*)malloc
										(sizeof(UPDATE_LONGITUDE_LATITUDE_ARGS));
	/* Read mwdImage through several times */

	/* Decide how many times to read file to the buffer */
	status = get_process_time_needed(&parameters,&process_times_needed);
	if(status != SUCCESS)
	{
		IAS_LOG_ERROR("failed to get the process times needed!\n");
		return ERROR;
	}


	for(i = 0; i < process_times_needed; i++)
	{
		status = read_mwdImage(&parameters,i,mwdImage_buffer_info,
				ephemeris_start_time,ephemeris_end_time);

		threadpool_t* pool;
		pool = threadpool_create(NUM_THREAD);
		for(j = 0; j < NUM_THREAD; j++)
		{
			update_longitude_latitude_args->start_oli_frame_to_update = j*mwdImage_buffer_info->num_oli_frame/NUM_THREAD;
			if(j == NUM_THREAD-1)
			{
				update_longitude_latitude_args->end_oli_frame_to_update = mwdImage_buffer_info->num_oli_frame;
			}
			else
			{
				update_longitude_latitude_args->end_oli_frame_to_update = j*mwdImage_buffer_info->num_oli_frame/NUM_THREAD
																			+mwdImage_buffer_info->num_oli_frame/NUM_THREAD;
			}
			update_longitude_latitude_args->model = model;
			update_longitude_latitude_args->mwdImage_buffer_info = mwdImage_buffer_info;

			threadpool_add(pool,(void*)&update_longitude_latitude,(void*)update_longitude_latitude_args);
		}

//		/* Added the remain data to the last thread */
//		update_longitude_latitude_args->start_oli_frame_to_update = j*(mwdImage_buffer_info->num_oli_frame/(NUM_THREAD-1))*(NUM_THREAD-2);
//		update_longitude_latitude_args->end_oli_frame_to_update = j*mwdImage_buffer_info->num_oli_frame;
//		threadpool_add(pool,(void*)&update_longitude_latitude,(void*)update_longitude_latitude_args);

		threadpool_destroy(pool);

		status = write_mwdImage(&parameters,i,mwdImage_buffer_info);
	}

//
//
//	IAS_SATELLITE_ID satellite_id;
//	int satellite_number = 8;
//	/* Get the satellite ID */
//	satellite_id = ias_sat_attr_get_satellite_id_from_satellite_number(
//			satellite_number);
//	if (satellite_id == ERROR)
//	{
//		IAS_LOG_ERROR("Unable to determine satellite ID from satellite "
//				"number %d", satellite_number);
//		return ERROR;
//	}

	/* Since the only purpose for retrieving the satellite ID is to
	   initialize the library, do that here */

//	const char *system_model_file = "/home/cqw/Desktop/systematic_model.140032.h5";
//	IAS_LOS_MODEL *model1;
//	model1 = ias_model_read(system_model_file);

//	double n_lines, n_sample;
//	int n_band, n_sca;
//	double target_elev;
//	double target_latd, target_long;
//	IAS_SENSOR_DETECTOR_TYPE dettype;
//
//	n_lines = 7500;
//	n_sample = 100;
//	n_band = 7;
//	n_sca = 9;
//	target_elev = 0;
//	dettype = IAS_NOMINAL_DETECTOR;
//
//	ias_los_model_input_line_samp_to_geodetic
//		(n_lines, n_sample,n_band, n_sca,target_elev,model,dettype, NULL ,&target_latd,&target_long);

	return SUCCESS;
}
