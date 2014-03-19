/**********************************************************************
Name: ias_ancillary_get_start_stop_frame_times

Purpose: Get OLI and TIRS first valid and last vaild frame times and
         their associated index numbers

Returns: Integer status code
         SUCCESS:  Successful completion 
         ERROR:    Operation failed
***********************************************************************/

#include <string.h>
#include <stdlib.h>
#include "ias_l0r.h"
#include "ias_ancillary.h"
#include "ias_logging.h"

int ias_ancillary_get_start_stop_frame_times
(
    IAS_L0R_OLI_FRAME_HEADER *oli_frame_headers,
         /* I: OLI frame headers, could be NULL if OLI data not available */
    IAS_L0R_TIRS_FRAME_HEADER *tirs_frame_headers, 
         /* I: TIRS frame headers, could be NULL if TIRS data not available */
    int oli_frame_header_size,   /* I: Number of OLI frame header records */
    int tirs_frame_header_size,  /* I: Number of TIRS frame header records */
    double *oli_start_frame_time,/* O: OLI start frame time */
    double *oli_stop_frame_time, /* O: OLI stop frame time */
    double *tirs_start_frame_time,/* O: OLI start frame index */
    double *tirs_stop_frame_time, /* O: OLI stop frame index */
    int *oli_start_frame_index,   /* O: TIRS start frame time */
    int *oli_stop_frame_index,    /* O: TIRS stop frame time */
    int *tirs_start_frame_index,  /* O: TIRS start frame index */
    int *tirs_stop_frame_index    /* O: TIRS start frame index */
)
{
    int index;

    if (oli_frame_headers != NULL)
    {
        /* Sanity check */
        if (oli_frame_header_size <= 0)
        {
            IAS_LOG_ERROR("Not enough OLI frame headers available");
            return ERROR;
        }

        /* Find the first OLI valid time in the frame headers */
        for (index = 0; index < oli_frame_header_size; index++)
        {
            if (oli_frame_headers[index].frame_status & IAS_L0R_VERIFIED)
            {
                *oli_start_frame_time = 
                    IAS_L0R_CONVERT_TIME_TO_SECONDS_SINCE_J2000(
                    oli_frame_headers[index].l0r_time);
                *oli_start_frame_index = index;
                break;
            }
        }

        /* Check if there is valid frame status existed */
        if (index >= oli_frame_header_size)
        { 
            IAS_LOG_ERROR("No OLI frame status is valid");
            return ERROR;
        }

        /* Find the last OLI valid time in the frame headers */
        for (index = oli_frame_header_size - 1; index >= 0; index--)
        {
            if (oli_frame_headers[index].frame_status & IAS_L0R_VERIFIED)
            {
                *oli_stop_frame_time = 
                    IAS_L0R_CONVERT_TIME_TO_SECONDS_SINCE_J2000(
                    oli_frame_headers[index].l0r_time);
                *oli_stop_frame_index = index;
                break;
            }
        }
    }

    if (tirs_frame_headers !=NULL)
    {
        /* Sanity check */
        if (tirs_frame_header_size <= 0)
        {
            IAS_LOG_ERROR("Not enough TIRS frame headers available");
            return ERROR;
        }

        /* Find the first TIRS valid time in the frame headers */
        for (index = 0; index < tirs_frame_header_size; index++)
        {
            if (tirs_frame_headers[index].frame_status & IAS_L0R_VERIFIED)
            {
                *tirs_start_frame_time = 
                    IAS_L0R_CONVERT_TIME_TO_SECONDS_SINCE_J2000(
                    tirs_frame_headers[index].l0r_time);
                *tirs_start_frame_index = index;
                break;
            }
        }

        /* Check if there is valid TIRS frame status existed */
        if (index >= tirs_frame_header_size)
        { 
            IAS_LOG_ERROR("No TIRS frame status is valid");
            return ERROR;
        }

        /* Find the last TIRS valid time in the frame headers */
        for (index = tirs_frame_header_size - 1; index >= 0; index--)
        {
            if (tirs_frame_headers[index].frame_status & IAS_L0R_VERIFIED)
            {
                *tirs_stop_frame_time = 
                    IAS_L0R_CONVERT_TIME_TO_SECONDS_SINCE_J2000(
                    tirs_frame_headers[index].l0r_time);
                *tirs_stop_frame_index = index;
                break;
            }
        }
    }

    return SUCCESS;
}
