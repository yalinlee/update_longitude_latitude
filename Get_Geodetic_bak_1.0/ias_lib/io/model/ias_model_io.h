#ifndef IAS_MODEL_IO_H
#define IAS_MODEL_IO_H

#include "ias_los_model.h"

/* Prototypes for the globally accessible read and write routines */
int ias_model_write
(
    const char *model_filename,     /* I: HDF model file name to write to */
    const IAS_LOS_MODEL *los_model  /* I: LOS model structure to write */
);

/* Caller must free the allocated model structure with ias_los_model_free()
   when done with it */
IAS_LOS_MODEL *ias_model_read
(
    const char *model_filename     /* I: HDF input model file name */
);

int ias_model_is_model_file
(
    const char *model_filename /* I: HDF input file name */
);

#endif
