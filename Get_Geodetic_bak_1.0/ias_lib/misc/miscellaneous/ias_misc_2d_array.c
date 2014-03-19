/*************************************************************************
NAME: ias_misc_allocate_2d_array

PURPOSE: Allocate memory for 2D array.

RETURNS: A pointer to a 2D array, or NULL if the routine fails. A pointer
         to an array of void pointers to the storage for each row of the
         array is returned. The returned pointer must be freed by the
         ias_misc_free_2d_array routine.
**************************************************************************/
#include <stddef.h>
#include <stdlib.h>
#include "ias_const.h"
#include "ias_logging.h"
#include "ias_miscellaneous.h"

/* The IAS_2D_ARRAY maintains a 2D array that can be sized at run-time. */
typedef struct ias_2d_array
{
    unsigned int signature; /* Signature used to make sure the pointer
                               math from a row_array_ptr actually gets back to 
                               the expected structure (helps detect errors). */
    int rows;               /* Rows in the 2D array */
    int columns;            /* Columns in the 2D array */
    int member_size;        /* Size of each entry in the array */
    void *data_ptr;         /* Pointer to the data storage for the array */
    void **row_array_ptr;   /* Pointer to an array of pointers to each row in
                               the 2D array */
    double memory_block[0]; /* Block of memory for storage of the array.
                               It is broken into two blocks.  The first 'rows *
                               sizeof(void *)' block stores the pointer the
                               first column in each of the rows.  The remainder
                               of the block is for storing the actual data.
                               Note: the type is double to force the worst case
                               memory alignment on sparc boxes. */
} IAS_2D_ARRAY;

/* Define a unique (i.e. random) value that can be used to verify a pointer
   points to an IAS_2D_ARRAY. This is used to verify the operation succeeds to
   get an IAS_2D_ARRAY pointer from a row pointer. */
#define SIGNATURE 0x326589ab

/* Given an address returned by the allocate routine, get a pointer to the
   entire structure. */
#define GET_ARRAY_STRUCTURE_FROM_PTR(ptr) \
    ((IAS_2D_ARRAY *)((char *)(ptr) - offsetof(IAS_2D_ARRAY, memory_block)))

/*************************************************************************
NAME: ias_misc_allocate_2d_array
**************************************************************************/
void **ias_misc_allocate_2d_array
(
    int rows,            /* I: Number of rows for the 2D array */
    int columns,         /* I: Number of columns for the 2D array */
    size_t member_size   /* I: Size of the 2D array element */
)
{
    int row;
    IAS_2D_ARRAY *array;
    size_t size;
    int data_start_index;

    /* Calculate the size needed for the array memory. The size includes the
       size of the base structure, an array of pointers to the rows in the
       2D array, an array for the data, and additional space
       (2 * sizeof(void*)) to account for different memory alignment rules
       on some machine architectures. */
    size = sizeof(*array) + (rows * sizeof(void*))
        + (rows * columns * member_size) + 2 * sizeof(void*);

    /* Allocate the structure */
    array = malloc(size);
    if (!array)
    {
        IAS_LOG_ERROR("Failure to allocate memory for the array");
        return NULL;
    }

    /* Initialize the member structures */
    array->signature = SIGNATURE;
    array->rows = rows;
    array->columns = columns;
    array->member_size = member_size;

    /* The array of pointers to rows starts at the beginning of the memory 
       block */
    array->row_array_ptr = (void **)array->memory_block;

    /* The data starts after the row pointers, with the index adjusted in
       case the void pointer and memory block pointers are not the same
       size */
    data_start_index = rows * sizeof(void *) / sizeof(array->memory_block[0]);
    if ((rows % 2) == 1)
        data_start_index++;
    array->data_ptr = &array->memory_block[data_start_index];

    /* Initialize the row pointers */
    for (row = 0; row < rows; row++)
    {
        array->row_array_ptr[row] = array->data_ptr 
            + row * columns * member_size;
    }

    return array->row_array_ptr;
}

/*************************************************************************
NAME: ias_misc_get_2d_array_size

PURPOSE: Returns the number of rows and columns in a 2D array.

RETURNS: SUCCESS or ERROR. ERROR will only be returned if a consistency check
    on the pointer passed in fails.
**************************************************************************/
int ias_misc_get_2d_array_size
(
    void **array_ptr,   /* I: Pointer returned by the alloc routine */
    int *rows,          /* O: Pointer to number of rows */
    int *columns        /* O: Pointer to number of columns */
)
{
    IAS_2D_ARRAY *array = GET_ARRAY_STRUCTURE_FROM_PTR(array_ptr);

    /* Verify it is a valid 2D array */
    if (array->signature != SIGNATURE)
    {
        /* Programming error of sort - exit the program */
        IAS_LOG_ERROR("Invalid signature on 2D array - memory "
                "corruption or programming error?");
        return ERROR;
    }

    *rows = array->rows;
    *columns = array->columns;

    return SUCCESS;
}

/*************************************************************************
NAME: ias_misc_free_2d_array

PURPOSE: Free memory for a 2D array allocated by ias_misc_allocate_2d_array

RETURNS: None
**************************************************************************/
int ias_misc_free_2d_array
(
    void **array_ptr    /* I: Pointer returned by the alloc routine */
)
{
    if (array_ptr != NULL)
    {
        /* Convert the array_ptr into a pointer to the structure */
        IAS_2D_ARRAY *array = GET_ARRAY_STRUCTURE_FROM_PTR(array_ptr);

        /* Verify it is a valid 2D array */
        if (array->signature != SIGNATURE)
        {
            /* Programming error of sort - exit the program */
            IAS_LOG_ERROR("Invalid signature on 2D array - memory "
                    "corruption or programming error?");
            return ERROR;
        }
        free(array);
    }

    return SUCCESS;
}
