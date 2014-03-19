/*
 * MQSend.h
 *
 *  Created on: Mar 16, 2012
 *      Author: xq
 */

#include "string.h"
#include "stdlib.h"
#include <time.h>
#include "amqp.h"
#include "amqp_framing.h"

#include "ias_const.h"
#include "MQSend.h"
#include "utils.h"

/* These variables are used in MQ_Init*/
int sockfd;
amqp_connection_state_t conn;
amqp_basic_properties_t props;

static int mq_init_success = 0;

int MQ_Init()
{
	int status;/* flag for  error !*/
	int port;
	port = atoi(mq_params.Port);
	/*MQ INIT START*/
	conn = amqp_new_connection();

	status = die_on_error(sockfd = amqp_open_socket(mq_params.Host, port),
			"Opening socket");
	if (status != SUCCESS) {
		mq_init_success = 0;/* MQ init is wrong! */
		return ERROR;
	}
	amqp_set_sockfd(conn, sockfd);
	die_on_amqp_error(
			amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN,
					mq_params.UserName, mq_params.PassWord), "logging in");

	amqp_channel_open(conn, 1);
	die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");

	props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;

	props.content_type = amqp_cstring_bytes("text/plain");

	props.delivery_mode = 2; /*persistent delivery mode*/
	/*MQ INIT END*/

	mq_init_success = 1;/* flag for MQ init successful! */
	return SUCCESS;
}

int getmqmessage
(
	char *status, /* I:the status of the mq message */
	char *message,/* I:mq message */
	char *buffer /* O:the out put mq message */
)
{
	char TimeStamp[30];
	time_t nowtime;
	struct tm *timeinfo;
	time(&nowtime);
	timeinfo = localtime(&nowtime);
	strftime(TimeStamp, sizeof(TimeStamp), "%Y-%m-%d %H:%M:%S", timeinfo);

	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, status);
	strcat(buffer, "@");
	strcat(buffer, TimeStamp);
	strcat(buffer, "@");
	strcat(buffer, message);

	return EXIT_SUCCESS;
}

int MQSend(int level, char *mqmessage)
{

	char msg_level[NAME_MAX];
	char buff[NAME_MAX];
//	int status;
	/**if it is the first time to send MQ message,connect;or send MQ message directly*/
	if (get_mq_init_success())
	{
		switch (level) {
		case 0:
		case 1:
			strcpy(msg_level, "Running");
			break;
		case 2:
			strcpy(msg_level, "Warning");
			break;
		case 3:
			strcpy(msg_level, "Error");
			break;
		case 4:
			strcpy(msg_level, "Started");
			break;
		case 5:
			strcpy(msg_level, "Completed");
			break;
		case 6:
			strcpy(msg_level, "Output");
			break;
		default:
			strcpy(msg_level, "StatusCode error");
			return ERROR;
		}

		getmqmessage(msg_level, mqmessage, buff);
		die_on_error(
				amqp_basic_publish(conn, 1,
						amqp_cstring_bytes(mq_params.ExchangeName),
						amqp_cstring_bytes(mq_params.OrderId), 0, 0, &props,
						amqp_cstring_bytes(buff)), "Publish");
	}
	return SUCCESS;
}

int get_mq_init_success() {
	return mq_init_success;
}
