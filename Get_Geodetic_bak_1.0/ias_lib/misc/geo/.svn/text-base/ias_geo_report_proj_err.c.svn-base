/*******************************************************************************
NAME: ias_geo_report_proj_err

PURPOSE: To report projection transformation package errors.

RETURN VALUE:   Type = int
Value    Description
-----    -----------
SUCCESS  Successful completion
ERROR    Operation failed

ALGORITHM DESCRIPTION:
    Receive a GCTP error code and print the correct error message.

NOTES:
Errors generated internal to the GCTP are listed below.

  ERROR   ROUTINE     MESSAGE
  -----   ------- ---------------------------
ERR = 0           SUCCESSFUL COMPLETION
ERR = 005         ILLEGAL SOURCE OR TARGET UNIT CODE
ERR = 006         OUTPUT FILENAME NOT SPECIFIED
ERR = 021 PJ02Z0  ILLEGAL ZONE NO 
ERR = 022 PJ02Z0  ERROR OPENING STATE PLANE PARAMETER FILE
ERR = 041 PJ04Z0  EQUAL LATITUDES FOR ST. PARALLELS ON OPPOSITE SIDES OF EQUATOR
ERR = 053 PJ05Z0  TRANSFORMATION CANNOT BE COMPUTED AT THE POLES
ERR = 123 PJ12Z0  POINT PROJECTS INTO CIRCLE OF RADIUS =',F12.2, METERS
ERR = 133 PJ13Z0  POINT MAPS INTO INFINITY
ERR = 153 PJ15Z0  POINT CANNOT BE PROJECTED
ERR = 195 PI19Z0  LATITUDE FAILED TO CONVERGE AFTER',I3,' ITERATIONS
ERR = 214 PJ21Z0  50 ITERATIONS WITHOUT CONV
ERR = 215 PJ21Z0  PROJECTION TRANSFORMATION TRANSFORMATION FAILED
ERR = 233 PJ23Z0  MISSING PROJECTION PARAMETERS
ERR = 234 PJ23Z0  TOO MANY ITERATIONS FOR INVERSE ROBINSON
ERR = 236 PJ23Z0  TOO MANY ITERATIONS IN INVERSE
ERR = 241 PJ23Z0  FAILED TO CONVERGE AFTER MANY ITERATIONS
ERR = 252 PJ23Z0  INPUT DATA ERROR
ERR = 1001        ILLEGAL INPUT SYSTEM CODE
ERR = 1002        ILLEGAL OUTPUT SYSTEM CODE
ERR = 1003        ILLEGAL INPUT UNIT CODE
ERR = 1004        ILLEGAL OUTPUT UNIT CODE
ERR = 1005        INCONSISTANT UNIT AND SYSTEM CODES FOR INPUT
ERR = 1006        INCONSISTANT UNIT AND SYSTEM CODES FOR OUTPUT
ERR = 1007        ILLEGAL INPUT ZONE CODE
ERR = 1008        ILLEGAL OUTPUT ZONE CODE
ERR = 1116 PAKSZ0 ILLEGAL DMS FIELD 
ERR = 1211        SHPEROID CODE RESET TO DEFAULT

*****************************************************************************/
#include <string.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_geo.h"

int ias_geo_report_proj_err 
(
    int err               /* I: Error returned from the GCTP call */
)
{
    switch (err)
    {
        case -2:
            IAS_LOG_ERROR("Point lies in break");
            return ERROR;
        case 0:
            return SUCCESS;
        case 6:
            IAS_LOG_ERROR("Output file name not specified");
            return ERROR;
        case 215:
            IAS_LOG_ERROR("Projection transformation failed");
            return ERROR;
        case 241:
            IAS_LOG_ERROR("Failed to converge after many iterations");
            return ERROR;
        case 234:
            IAS_LOG_ERROR("Too many iterations for inverse Robinson");
            return ERROR;
        case 236:
            IAS_LOG_ERROR("Too many iterations in inverse");
            return ERROR;
        case 252:
            IAS_LOG_ERROR("Input data error");
            return ERROR;
        case 1116:
            IAS_LOG_ERROR("Illegal DMS field");
            return ERROR;
        case 1005:
            IAS_LOG_ERROR("Inconsistent unit and system codes for input");
            return ERROR;
        case 1001:
            IAS_LOG_ERROR("Illegal input system code");
            return ERROR;
        case 1003:
            IAS_LOG_ERROR("Illegal input unit code");
            return ERROR;
        case 1007:
            IAS_LOG_ERROR("Illegal input zone code");
            return ERROR;
        case 133:
            IAS_LOG_ERROR("Point projects into infinity");
            return ERROR;
        case 195:
            IAS_LOG_ERROR("Latitude failed to converge after many iterations");
            return ERROR;
        case 1006:
            IAS_LOG_ERROR("Inconsistent unit and system codes for output");
            return ERROR;
        case 1002:
            IAS_LOG_ERROR("Illegal output system code");
            return ERROR;
        case 1004:
            IAS_LOG_ERROR("Illegal output unit code");
            return ERROR;
        case 1008:
            IAS_LOG_ERROR("Illegal output zone code");
            return ERROR;
        case 53: 
            IAS_LOG_ERROR("Transformation cannot be computed at the poles");
            return ERROR;
        case 153:
            IAS_LOG_ERROR("Point cannot be projected");
            return ERROR;
        case 123:
            IAS_LOG_ERROR("Point projects into a circle of unacceptable"
                " radius");
            return ERROR;
        case 214:
            IAS_LOG_ERROR("Fifty iterations performed without conversion");
            return ERROR;
        case 1211:
            IAS_LOG_ERROR("Spheroid code reset to default");
            return ERROR;
        case 41:
            IAS_LOG_ERROR("Equal latitudes for std parallels on opposite"
                " sides of equator");
            return ERROR;
        case 21:
            IAS_LOG_ERROR("Illegal zone number");
            return ERROR;
        case 22:
            IAS_LOG_ERROR("Opening state plane parameter file");
            return ERROR;
        case 5:
            IAS_LOG_ERROR("Illegal source or target unit code");
            return ERROR;
        case 233:
            IAS_LOG_ERROR("Missing projection parameters");
            return ERROR;
        default:
            IAS_LOG_ERROR("Unrecognized error case");
            return ERROR;
    }

    return SUCCESS;
}
