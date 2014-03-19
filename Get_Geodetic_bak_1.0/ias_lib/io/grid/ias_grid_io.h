#ifndef _IAS_GRID_IO_H_
#define _IAS_GRID_IO_H_

#include "ias_grid.h"

/* Make a forward declaration to the IAS_GRID_FILE type so external users can
   refer to it without knowing the definition. */
typedef struct ias_grid_file IAS_GRID_FILE;

int ias_grid_initialize 
(
    IAS_GRID_TYPE *grid, /* I/O: Grid structure to read        */
    int bands,           /* I: number of bands in the gridbands array */
    int scas             /* I: maximum number of scas in the gridbands array */
);

int ias_grid_read 
(
    const char *grid_filename,/* I: grid input file name */
    const int *band_numbers,  /* I: Array of bands to read */
    int nbands,               /* I: number of bands in band_numbers or zero for
                                  all bands (in which case, band_numbers can
                                  be NULL) */
    IAS_GRID_TYPE *grid       /* O: Grid structure to read */
);

IAS_GRID_FILE *ias_grid_create
(
    const char *grid_filename   /* I: grid file name to create */
);

int ias_grid_close
(
    IAS_GRID_FILE *file     /* I: pointer to the grid to close */
);

int ias_grid_write 
(
    const char *grid_filename,/* I: Grid file name to write */
    const IAS_GRID_TYPE *grid /* I: Grid structure to read */
);

int ias_grid_header_write
(
    IAS_GRID_FILE *file,        /* I: Grid file pointer */
    const IAS_GRID_TYPE *grid   /* I: Grid structure to write */
);

int ias_grid_band_header_write
(
    IAS_GRID_FILE *file,        /* I: Grid file pointer */
    const IAS_GRID_TYPE *grid   /* I: Grid structure to write */
);

int ias_grid_band_pointers_write
(
    IAS_GRID_FILE *file,        /* I: Grid file pointer */
    const IAS_GRID_TYPE *grid,  /* I: Grid structure to write */
    int band_number             /* I: Band number to write */
);

int ias_grid_header_read
(
    const char *grid_filename,/* I: Grid file name */
    IAS_GRID_TYPE *grid       /* I: Grid structure to read */
);

int ias_grid_band_header_read
(
    const char *grid_filename,/* I: Grid file name*/
    IAS_GRID_TYPE *grid       /* I: Grid structure to read */
);

int ias_grid_band_pointers_read
(
    const char *grid_filename,/* I: Grid file name */
    IAS_GRID_TYPE *grid,      /* I/O: Grid structure to read into */
    int band_number           /* I: Band number to read from the grid file */
);

int ias_grid_malloc_band
(
    IAS_GRID_TYPE *grid,        /* I/O: grid to allocate band in */
    int band_number             /* I: band number to allocate memory for */
);

int ias_grid_malloc 
(
    IAS_GRID_TYPE *grid,    /* I/O: Grid structure to be malloced   */
    const int *band_numbers,/* I: Array of bands to allocate memory for */
    int nbands              /* I: number of bands in band_numbers or zero for
                                  all bands (in which case, band_numbers can
                                  be NULL) */
);

int ias_grid_free_band
(
    IAS_GRID_TYPE *grid,        /* I/O: grid to free band in */
    int band_number             /* I: band number to free */
);

int ias_grid_free 
(
    IAS_GRID_TYPE *grid        /* I/O: Grid structure to be freed */
);

int ias_grid_is_grid_file
(
    const char *grid_filename /* I: File name to check */
);

#endif
