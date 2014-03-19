/*
 * read_write_mwdImage.h
 *
 *  Created on: Mar 18, 2014
 *      Author: cqw
 */

#ifndef READ_WRITE_MWDIMAGE_H_
#define READ_WRITE_MWDIMAGE_H_


#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "ias_logging.h"
#include "read_parameter.h"


#define MEM_BLOCK_SIZE 1073741824UL
#define REDUNDANCY 1024*1024
#define J2000_SUB_UTC_EPOCH 946727935861UL
#define LOCK_TIMES 2
#define PROCESS_TIME 5

typedef struct frame_header
{
	char satName[2];
	int frame_number;
	int frame_length;
	double longtitude;
	double latitude;
	long long time;
	unsigned short band;
	char mode[4];
}FRAME_HEADER;




typedef struct mwdImage_buffer_info
{
	char* mem_mapped_buffer;
	long long num_bytes_in_buffer;
	size_t oli_frame_start_bytes_in_buffer[4096];
	int num_oli_frame;
}MWDIMAGE_BUFFER_INFO;


int get_process_time_needed
(
	PARAMETERS* param,
	size_t* process_times_needed
);

int read_mwdImage
(
		PARAMETERS* param, 							 //I:parmaeters
		int process_times,							 //I:the process time of the mwdImage file
		MWDIMAGE_BUFFER_INFO *mwdImage_buffer_info,   //O:pointer of mapped buffer
													 //	 and the info of oli frame
		double ephemeris_start_time,
		double ephemeris_end_time
);

int frame_header_sync
(
	char* memblock, 				//I:the mapped buffer need to be synced
	size_t block_length,			//I:the length of the mapped buffer
	size_t* star_sync_position	//O:the start sync bytes of the mapped buffer
);

int write_mwdImage
(
	PARAMETERS* param, 							 //I:parmaeters
	int process_times,							 //I:the process time of the mwdImage file
	MWDIMAGE_BUFFER_INFO *mwdImage_buffer_info   //O:pointer of mapped buffer
													 //	 and the info of oli frame
);
#endif /* READ_WRITE_MWDIMAGE_H_ */
