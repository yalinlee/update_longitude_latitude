/*************************************************************************
Name: ias_los_model_initialize

Purpose: Provides interface to the initialization routines to setup basic
    information in the model based on the acquisition type

Returns:
    SUCCESS or ERROR

Notes:
**************************************************************************/
#include <stdlib.h>
#include "ias_logging.h"
#include "ias_los_model.h"

IAS_LOS_MODEL *ias_los_model_initialize
(
    IAS_ACQUISITION_TYPE acq_type    /* I - acquisition type */
)
{
    IAS_LOS_MODEL *model;

    /* Allocate memory for the model structure. This routine also does the
       basic initialization of values to 0 or known values from the
       satellite attributes. */
    model = ias_los_model_allocate();
    if (!model)
    {
        IAS_LOG_ERROR("Allocating model");
        return NULL;
    }

    return model;
}
