/*
 * MQSend.h
 *
 *  Created on: Mar 19, 2012
 *      Author: xq
 */

#ifndef MQSEND_H_
#define MQSEND_H_

#include <limits.h>         /* MQ parameters in ODL file */
/* Modified by XQian 2012.11.27 for MQ parameters to read */
/* MQ parameters in ODL file */
typedef struct MQparameters
{
    char ModuleLogDir[NAME_MAX];     /* log file directory  */
    char OutputDir[NAME_MAX];        /* output file directory */
    char OrderId[NAME_MAX];          /* order ID */
    char Host[NAME_MAX];             /* host name */
    char Port[NAME_MAX];             /* port */
    char ExchangeName[NAME_MAX];     /* ExchangeName */
    char UserName[NAME_MAX];         /* UserName */
    char PassWord[NAME_MAX];         /* PassWord */
} MQ_PARAMS;

MQ_PARAMS mq_params;          /* Modified by XQian 2012.11.27 for MQ parameters to read */

int MQ_Init();

int get_mq_init_success();

int getmqmessage
(
    char *status, /* I:the status of the mq message */
    char *message,/* I:mq message */
    char *buffer /* O:the out put mq message */
);

int MQSend(int level, char *mqmessage);

#endif /* MQCONPARAM_H_ */
