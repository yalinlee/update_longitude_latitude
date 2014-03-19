/*
 * read_ephemeris_data.c
 *
 *  Created on: Mar 12, 2014
 *      Author: lyl
 */


#include "read_ephemeris_data.h"


int read_ephemeris_data_for_MWD(PARAMETERS* param,
		IAS_L0R_EPHEMERIS** l0r_ephemeris, long long *num_frame_of_ephemeris)
{
	int i;
	int Num_Byte_Per_Frame_Of_ephemeris = 69;

	/***********************************************************************
	 * read ephemeris file data into a buffer
	 ***********************************************************************/
	char *ephemeris_data = NULL;

	FILE *fp = fopen(param->ephemeris_filename,"r");
	if(fp != NULL)
	{
		//go to the end of the file
		if(fseek(fp,0L,SEEK_END) == 0)
		{
			//get the size of the file
			long long bufsize = ftell(fp);
			*num_frame_of_ephemeris = bufsize / Num_Byte_Per_Frame_Of_ephemeris;
			if(bufsize == -1)
				return ERROR;
			ephemeris_data = malloc(sizeof(char) * bufsize);
			//go back to the start of the file
			if(fseek(fp,0L,SEEK_SET) != 0)
				return ERROR;
			size_t newlen = fread(ephemeris_data,sizeof(char),bufsize,fp);
			if (newlen != bufsize)
			{
				IAS_LOG_ERROR("failed to read ephemeris file into buffer.\n");
				return ERROR;
			}
		}
		fclose(fp);
	}

	/* Allocate memory for lor_ephemeris */
	*l0r_ephemeris = calloc(*num_frame_of_ephemeris,sizeof(**l0r_ephemeris));
	if(*l0r_ephemeris == NULL)
	{
		IAS_LOG_ERROR("failed to allocate memory for l0r_ephemeris.\n");
		return ERROR;
	}

	/* read ephemeris data stored in ephemeris_data into l0r_ephemeris */
	size_t l0r_time_size = sizeof((*l0r_ephemeris)->l0r_time);
	size_t time_tag_sec_orig_size = sizeof((*l0r_ephemeris)->time_tag_sec_orig);
	size_t ecef_velocity_meters_per_sec_size = sizeof((*l0r_ephemeris)->ecef_velocity_meters_per_sec);
	size_t ecef_position_meters_size = sizeof((*l0r_ephemeris)->ecef_position_meters);
	size_t warning_flag_size = sizeof((*l0r_ephemeris)->warning_flag);

	for(i = 0; i < *num_frame_of_ephemeris; i++)
	{
		memcpy(&((*l0r_ephemeris+i)->l0r_time),
			   (ephemeris_data + i*Num_Byte_Per_Frame_Of_ephemeris),
				l0r_time_size );

		memcpy(&((*l0r_ephemeris+i)->time_tag_sec_orig),
			   (ephemeris_data + i*Num_Byte_Per_Frame_Of_ephemeris + l0r_time_size),
			   time_tag_sec_orig_size);


		memcpy(&((*l0r_ephemeris+i)->ecef_position_meters),
			   (ephemeris_data + i*Num_Byte_Per_Frame_Of_ephemeris
					   + l0r_time_size + time_tag_sec_orig_size),
				ecef_position_meters_size);

		memcpy(&((*l0r_ephemeris+i)->ecef_velocity_meters_per_sec),
			   (ephemeris_data + i*Num_Byte_Per_Frame_Of_ephemeris
					   + l0r_time_size + time_tag_sec_orig_size + ecef_position_meters_size),
				ecef_velocity_meters_per_sec_size);

		memcpy(&((*l0r_ephemeris+i)->warning_flag),
			   (ephemeris_data + i*Num_Byte_Per_Frame_Of_ephemeris
					   + l0r_time_size + time_tag_sec_orig_size + ecef_velocity_meters_per_sec_size + ecef_position_meters_size),
				warning_flag_size);
	}


	//write the buffer to the file to check whether the buffer copy is work properly
//	#if DEBUG_GENERATE_DATA_FILES == 1
		FILE *fp1 = fopen("/home/cqw/Desktop/extracted1.dat","wb");
		if(fp1)
		{
//			for(i = 0; i < *num_frame_of_ephemeris; i++)
//			{
				fwrite(*l0r_ephemeris,sizeof(**l0r_ephemeris),*num_frame_of_ephemeris,fp1);
//			}
		}
//	#endif

	return SUCCESS;
}

