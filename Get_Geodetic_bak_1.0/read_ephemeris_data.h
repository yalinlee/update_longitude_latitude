/*
 * Ephemeris_data_preprocess.h
 *
 *  Created on: Mar 12, 2014
 *      Author: cqw
 */

#ifndef READ_EPHEMERIS_DATA_H_
#define READ_EPHEMERIS_DATA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ias_logging.h"
#include "read_parameter.h"



int read_ephemeris_data_for_MWD
(
	PARAMETERS* param, 					//I: parameters
	IAS_L0R_EPHEMERIS **l0r_ephemeris,	//O: ephemeris buff
	long long *num_frame_of_ephemeris
);

#endif /* READ_EPHEMERIS_DATA_H_ */
