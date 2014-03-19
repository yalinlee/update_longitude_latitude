/*
 * read_write_mwdImage.c
 *
 *  Created on: Mar 18, 2014
 *      Author: cqw
 */


#include "read_write_mwdImage.h"

int get_process_time_needed(PARAMETERS* param, size_t* process_times_needed)
{
	int fd;
	int status = ERROR;
	struct stat file_state;
	long long ret;

	/* To obtain file size */
	fd = open(param->mwdImage_filename,O_RDWR);
	IAS_LOG_DEBUG("fd = %d\n", fd);
	if(fd == -1)
	{
		IAS_LOG_ERROR("failed to open the file!\n");
		return ERROR;
	}
	status = fstat(fd,&file_state);
	if(status != SUCCESS)
	{
		IAS_LOG_ERROR("failed to obtain the file state.\n");
		return ERROR;
	}
	IAS_LOG_DEBUG("Size: %llu\n",(uint64_t)file_state.st_size);

	ret = file_state.st_size;
	*process_times_needed = ret / MEM_BLOCK_SIZE;
	if((ret % MEM_BLOCK_SIZE) != 0)
	{
		*process_times_needed += 1;
	}

	return SUCCESS;
}


int read_mwdImage(PARAMETERS* param, int process_times,
					MWDIMAGE_BUFFER_INFO *mwdImage_buffer_info,
					double ephemeris_start_time, double ephemeris_end_time)
{
	char* memblock;
	long long mem_block_size;
	int fd;
	int status = ERROR;
	FRAME_HEADER *frame_head;
	frame_head = (FRAME_HEADER*)malloc(sizeof(FRAME_HEADER));
	long long start_sync_position = 0;

	/* To obtain file size */
	fd = open(param->mwdImage_filename,O_RDWR);
	IAS_LOG_DEBUG("fd = %d\n", fd);
	if(fd == -1)
	{
		IAS_LOG_ERROR("failed to open the file!\n");
		return ERROR;
	}

	/* If it is the first time to read file */
	if(process_times == 0)
	{
		/* Allocate memory for memblock */
		memblock = (char*)malloc(MEM_BLOCK_SIZE*sizeof(char));

		/* Read file into memblock */
		mem_block_size = read(fd,memblock,MEM_BLOCK_SIZE*sizeof(char));
		if(mem_block_size == -1)
		{
			IAS_LOG_ERROR("failed in pread!\n");
			return ERROR;
		}

		/* check the image time, if image time > ephemeris time,
		 * move to next oli frame, return the position of the
		 * oli frame whoes image time < ephemeris time
		 */
		long long block_index = 0;
		long long num_frame = 0;
		while(block_index < mem_block_size)
		{
			memcpy(&(frame_head->frame_length),memblock+6+block_index,
							sizeof(int));
			memcpy(&(frame_head->mode),memblock+36+block_index,
					sizeof(char)*4);
			if(frame_head->mode[0] == 'O' && frame_head->mode[1] == 'L'
					&& frame_head->mode[2] == 'I')
			{
				if((frame_head->time - J2000_SUB_UTC_EPOCH) < ephemeris_start_time*1000)
				{
					mwdImage_buffer_info->oli_frame_start_bytes_in_buffer[num_frame++]
														 = block_index;
					block_index += frame_head->frame_length;
				}
				else
				{
					block_index += frame_head->frame_length;
				}

			}
			else //frame mode is TIRS or PAN
			{
				block_index += frame_head->frame_length;
			}
		}
		mwdImage_buffer_info->num_oli_frame = num_frame;
	}
	/* If it is the last time to read file */
	else if(process_times == PROCESS_TIME - 1)
	{
		/* Allocate memory for memblock */
		memblock = (char*)malloc((MEM_BLOCK_SIZE+REDUNDANCY)*sizeof(char));

		/* Read file into memblock */
		lseek(fd,MEM_BLOCK_SIZE*process_times-REDUNDANCY,SEEK_SET);
		mem_block_size = read(fd,memblock,(MEM_BLOCK_SIZE+REDUNDANCY)*sizeof(char));
		if(mem_block_size == -1)
		{
			IAS_LOG_ERROR("failed in pread!\n");
			return ERROR;
		}

		/* Sync the frame */
		status = frame_header_sync(memblock,mem_block_size,&start_sync_position);
		if(status != SUCCESS)
		{
			IAS_LOG_ERROR("Cann't sync the frame header!\n");
			return ERROR;
		}

		long long block_index = 0;
		long long num_frame = 0;
		while(block_index < mem_block_size)
		{
			memcpy(&(frame_head->frame_length),memblock+start_sync_position+6+block_index,
					sizeof(int));
			memcpy(&(frame_head->mode),memblock+start_sync_position+36+block_index,
					sizeof(char)*4);
			if(frame_head->mode[0] == 'O' && frame_head->mode[1] == 'L'
					&& frame_head->mode[2] == 'I')
			{
				if(frame_head->time - J2000_SUB_UTC_EPOCH < ephemeris_end_time*1000)
				{
					mwdImage_buffer_info->oli_frame_start_bytes_in_buffer[num_frame++]
										 = start_sync_position+block_index;
					block_index += frame_head->frame_length;
				}
				else
				{
					break;
				}
			}
			else //frame mode is TIRS or PAN
			{
				block_index += frame_head->frame_length;
			}
		}
		mwdImage_buffer_info->num_oli_frame = num_frame;
	}
	/* Others time to read the file */
	else
	{
		/* Allocate memory for memblock */
		memblock = (char*)malloc((MEM_BLOCK_SIZE+REDUNDANCY)*sizeof(char));

		/* Read file into memblock */
		lseek(fd,MEM_BLOCK_SIZE*process_times-REDUNDANCY,SEEK_SET);
		mem_block_size = read(fd,memblock,(MEM_BLOCK_SIZE+REDUNDANCY)*sizeof(char));
		if(mem_block_size == -1)
		{
			IAS_LOG_ERROR("failed in pread!\n");
			return ERROR;
		}

		/* Sync the frame */
		status = frame_header_sync(memblock,mem_block_size,&start_sync_position);
		if(status != SUCCESS)
		{
			IAS_LOG_ERROR("Cann't sync the frame header!\n");
			return ERROR;
		}


		long long block_index = 0;
		long long num_frame = 0;
		while(block_index < mem_block_size)
		{
			memcpy(&(frame_head->frame_length),memblock+start_sync_position+6+block_index,
					sizeof(int));
			memcpy(&(frame_head->mode),memblock+start_sync_position+36+block_index,
					sizeof(char)*4);
			if(frame_head->mode[0] == 'O' && frame_head->mode[1] == 'L'
					&& frame_head->mode[2] == 'I')
			{
				mwdImage_buffer_info->oli_frame_start_bytes_in_buffer[num_frame++]
									 = start_sync_position+block_index;
				block_index += frame_head->frame_length;
			}
			else //frame mode is TIRS or PAN
			{
				block_index += frame_head->frame_length;
			}
		}
		mwdImage_buffer_info->num_oli_frame = num_frame;
	}


	/* store the buffer information into mwdImage_buffer_info */
	mwdImage_buffer_info->mem_mapped_buffer = memblock;
	mwdImage_buffer_info->num_bytes_in_buffer = mem_block_size;


	close(fd);
	return 0;
}


int write_mwdImage(PARAMETERS* param, int process_times,
					MWDIMAGE_BUFFER_INFO *mwdImage_buffer_info)
{
	char* memblock;
	long long ret;
	int fd;

	/* To obtain file size */
	fd = open(param->output_filename,O_RDWR|O_CREAT);
	IAS_LOG_DEBUG("fd = %d\n", fd);
	if(fd == -1)
	{
		IAS_LOG_ERROR("failed to open the file!\n");
		return ERROR;
	}

	/* Write the buffer into file */
	memblock = mwdImage_buffer_info->mem_mapped_buffer;
	/* If it is the first time to read file */
	if(process_times == 0)
	{

		/* write  memblock into file*/
		ret = write(fd,memblock+mwdImage_buffer_info->oli_frame_start_bytes_in_buffer[0],
					mwdImage_buffer_info->num_bytes_in_buffer-mwdImage_buffer_info->oli_frame_start_bytes_in_buffer[0]);
		if(ret == -1)
		{
			IAS_LOG_ERROR("failed in write!\n");
			return ERROR;
		}
	}
	/* If it is the last time to read file */
	else if(process_times == PROCESS_TIME - 1)
	{
		lseek(fd,MEM_BLOCK_SIZE*process_times-REDUNDANCY,SEEK_SET);
		/* write  memblock into file*/
		ret = write(fd,memblock,mwdImage_buffer_info->oli_frame_start_bytes_in_buffer
				[mwdImage_buffer_info->num_oli_frame]);
		if(ret == -1)
		{
			IAS_LOG_ERROR("failed in write!\n");
			return ERROR;
		}
	}
	else
	{
		lseek(fd,MEM_BLOCK_SIZE*process_times-REDUNDANCY,SEEK_SET);
		/* write  memblock into file*/
		ret = write(fd,memblock,mwdImage_buffer_info->num_bytes_in_buffer);
		if(ret == -1)
		{
			IAS_LOG_ERROR("failed in write!\n");
			return ERROR;
		}
	}

	free(memblock);
	close(fd);
	return 0;
}




int frame_header_sync(char* memblock, size_t block_length,
						size_t* start_sync_position)
{
	int frame_length1;
	int frame_length2;
	char* buffer = memblock;
	long long buffer_index = 0;
	FRAME_HEADER* frame_header;

	while(buffer_index < block_length-1)
	{
		if((buffer[buffer_index] == 'L') && (buffer[buffer_index+1] == '8'))
			break;
		else
			buffer_index++;
	}

	if(buffer_index >= block_length-1)
	{
		IAS_LOG_ERROR("failed to find the fisrt frame head.\n");
		return ERROR;
	}

	frame_header = (FRAME_HEADER*)malloc(sizeof(FRAME_HEADER));
//	memcpy(&(frame_header->satName), buffer+buffer_index,sizeof(char)*2);
	memcpy(&(frame_header->frame_length),buffer+buffer_index+6,sizeof(int));
	frame_length1 = frame_header->frame_length;

	if((buffer[buffer_index+frame_length1] == 'L')
			&& (buffer[buffer_index+frame_length1+1] == '8'))
	{
		memcpy(&(frame_header->frame_length),
				buffer+buffer_index+frame_length1+6,sizeof(int));
		frame_length2 = frame_header->frame_length;
		if((buffer[buffer_index+frame_length1+frame_length2] == 'L')
				&&(buffer[buffer_index+frame_length1+frame_length2+1] == '8'))
		{
			*start_sync_position = buffer_index;
			return SUCCESS;
		}
		else
		{
			IAS_LOG_ERROR("failed to sync third frame.\n");
			return ERROR;
		}
	}
	else
	{
		IAS_LOG_ERROR("failed to sync second frame.\n");
		return ERROR;
	}
}


