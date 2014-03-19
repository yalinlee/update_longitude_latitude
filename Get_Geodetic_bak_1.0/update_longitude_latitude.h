/*
 * update_longitude_latitude.h
 *
 *  Created on: Mar 17, 2014
 *      Author: lyl
 */

#ifndef UPDATE_LONGITUDE_LATITUDE_H_
#define UPDATE_LONGITUDE_LATITUDE_H_

#include "ias_los_model.h"
#include "ias_geo.h"
#include "ias_model_io.h"
#include "ias_logging.h"
#include "ias_ancillary.h"
#include "read_parameter.h"
#include "read_write_mwdImage.h"



typedef struct update_longitude_latitude_args
{
	int start_oli_frame_to_update;		//the first frame to update
	int end_oli_frame_to_update;		//the last frame to update
	IAS_LOS_MODEL *model;			   //I:pointer to the LOS model
	MWDIMAGE_BUFFER_INFO* mwdImage_buffer_info;  //I: mapped buffer information
}UPDATE_LONGITUDE_LATITUDE_ARGS;


void update_longitude_latitude
(
//	double* ephemeris_start_time,      //I:start time of ephemeris data
//	double* ephemeris_end_time,		   //I:end time of ephemeris data
	UPDATE_LONGITUDE_LATITUDE_ARGS* update_longitude_latitude_args
);
#endif /* UPDATE_LONGITUDE_LATITUDE_H_ */
