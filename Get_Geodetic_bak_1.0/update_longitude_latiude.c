/*
 * update_longitude_latiude.c
 *
 *  Created on: Mar 17, 2014
 *      Author: cqw
 */

#ifndef UPDATE_LONGITUDE_LATIUDE_C_
#define UPDATE_LONGITUDE_LATIUDE_C_

#include "update_longitude_latitude.h"

void update_longitude_latitude
(
//	double* ephemeris_start_time,      //I:start time of ephemeris data
//	double* ephemeris_end_time,		   //I:end time of ephemeris data
	UPDATE_LONGITUDE_LATITUDE_ARGS* update_longitude_latitude_args
//	int start_oli_frame_to_update,		//the first frame to update
//	int end_oli_frame_to_update,		//the last frame to update
//	IAS_LOS_MODEL *model,			   //I:pointer to the LOS model
//	MWDIMAGE_BUFFER_INFO *mwdImage_buffer_info  //I: mapped buffer information)
)
{
	int start_oli_frame_to_update = update_longitude_latitude_args->start_oli_frame_to_update;
	int end_oli_frame_to_update = update_longitude_latitude_args->end_oli_frame_to_update;
	IAS_LOS_MODEL *model = update_longitude_latitude_args->model;
	MWDIMAGE_BUFFER_INFO *mwdImage_buffer_info = update_longitude_latitude_args->mwdImage_buffer_info;

	FRAME_HEADER* frame_head;
	frame_head = (FRAME_HEADER*)malloc(sizeof(*frame_head));
	//Check the time to make sure that
	int i;

	int n_band = 7;
	int n_sca = 9;
	double n_sample = 100;
	double target_elev = 0;
	IAS_SENSOR_DETECTOR_TYPE dettype = IAS_NOMINAL_DETECTOR;

	for(i = start_oli_frame_to_update ; i < end_oli_frame_to_update ; i++)
	{
		memcpy(&(frame_head->time),mwdImage_buffer_info->mem_mapped_buffer
				    +mwdImage_buffer_info->oli_frame_start_bytes_in_buffer[i]+26,
				    sizeof(long long));
		ias_los_model_input_line_samp_to_geodetic
					(frame_head->time, n_sample,n_band, n_sca,target_elev,model,
					 dettype, NULL ,&frame_head->latitude,&frame_head->longtitude);
		memcpy(mwdImage_buffer_info->mem_mapped_buffer+
				mwdImage_buffer_info->oli_frame_start_bytes_in_buffer[i]+10,
				&(frame_head->longtitude), sizeof(double));
		memcpy(mwdImage_buffer_info->mem_mapped_buffer+
				mwdImage_buffer_info->oli_frame_start_bytes_in_buffer[i]+10,
				&(frame_head->latitude), sizeof(double));
	}
}

#endif /* UPDATE_LONGITUDE_LATIUDE_C_ */
