#ifndef IAS_BPF_DATABASE_LOCAL_H 
#define IAS_BPF_DATABASE_LOCAL_H

#include "ias_bpf.h"    /* Defines the BPF data structure   */

/**************************************************************************

NAME: ias_bpf_database_local.h

PURPOSE: Header file defining Bias Parameter File (BPF) database I/O 
         routines which are local to the library.

ALGORITHM REFERENCES: None

NOTES: The struct ias_db_connection * input parameters are not modified 
       by these routines (this marked I:), but the underlying database
       routines do modify the structure, so they are not const parameters.


**************************************************************************/

int ias_bpf_db_insert_file_attributes
(
    struct ias_db_connection *conn,     /* I: Open db connection */
    int bpf_id,                         /* I: ID of BPF being written */
    IAS_BPF *bpf                  /* I: BPF information */
);

int ias_bpf_db_insert_orbit_parameters
(
    struct ias_db_connection *conn,     /* I: Open db connection */
    int bpf_id,                         /* I: BPF being written */
    IAS_BPF *bpf                  /* I: BPF information */
);

int ias_bpf_db_insert_bias_model
(
    struct ias_db_connection *conn,     /* I: Open db connection */
    int bpf_id,                         /* I: ID of BPF being written */
    IAS_BPF *bpf                        /* I: BPF information */
);

int ias_bpf_db_start
(
    struct ias_db_connection *conn,     /* I: database connection */
    IAS_BPF *bpf                  /* I: BPF information */
);

int ias_bpf_db_create_group
(
    struct ias_db_connection *conn,     /* I: database connection */
    int bpf_id,                         /* I: ID of BPF being written */
    char *bpf_file_name,                /* I: Name of BPF being written */
    char *group_name,                   /* I: Name of group to create */
    int group_order                     /* I: Order of this group in the BPF */
);

#define PARAMETER_NAME_LENGTH 200
#define PARAMETER_VALUE_LENGTH 400

struct PARAMETER_NAME_ID {
    char parameter_name[PARAMETER_NAME_LENGTH];
    int parameter_id;
};

struct PARAMETER_NAME_ID *ias_bpf_db_get_parameter_ids
(
    struct ias_db_connection *conn,     /* I: database connection */
    int *num_parameters                 /* O: number of parameters found */
);

int ias_bpf_db_validate_contents
(
    IAS_BPF *bpf                  /* I/O: BPF data to validate. */
);

#endif
