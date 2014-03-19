/*
 * read_parameters.h
 *
 *  Created on: Mar 12, 2014
 *      Author: cqw
 */
#include <limits.h>

#include "ias_math.h"
#include "ias_l0r.h"
#include "ias_cpf.h"

#include "MQSend.h"

#ifndef GET_GEODETIC_INIT_H_
#define GET_GEODETIC_INIT_H_

#define SUCCESS  0
#define ERROR   -1
#define WARNING -2


/* length of a Work Order ID */
#define IAS_WORKORDER_ID_LENGTH 12
/* size of a Work Order ID string, including null terminator */
#define IAS_WORKORDER_ID_SIZE (IAS_WORKORDER_ID_LENGTH + 1)
/* chars in a path name including nul */
#define PATH_MAX   4096


/* OMF, CPF, L0R ancillary data, and parameter file data */
typedef struct parameters
{
    char work_order_id[IAS_WORKORDER_ID_SIZE]; /* Uniquely identifies the
                                                  work order */
    char work_order_directory[PATH_MAX];
    char log_filename[PATH_MAX];			/* LOG filename */
//    char omf_filename[PATH_MAX];     		/* OMF filename */
    char ephemeris_filename[PATH_MAX];     	/* ancillary filename */
    char cpf_filename[PATH_MAX];     		/* CPF filename */
    char mwdImage_filename[PATH_MAX];     	/* mwdImage product location */
    char output_filename[PATH_MAX];     	/* output file location */
    IAS_SATELLITE_ID satellite_id;
} PARAMETERS;


int read_parameters
(
	const char *parm_filename, /* I: Parameter filename */
	PARAMETERS *parameters,    /* O: Parameters retrieved from parameter file,
									 OMF, and CPF */
	MQ_PARAMS *mq_params       /* O: MQ parameters in ODL file */
);






#endif /* GET_GEODETIC_INIT_H_ */
