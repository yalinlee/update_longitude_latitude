#ifndef IAS_BPF_DATABASE_H 
#define IAS_BPF_DATABASE_H

#include "ias_db.h"    /* Defines the db data structure   */
#include "ias_bpf.h"    /* Defines the BPF data structure   */

/**************************************************************************

NAME: ias_bpf_database.h

PURPOSE: Header file defining publicly accessible Bias Parameter File (BPF)
         database I/O routines.

ALGORITHM REFERENCES: None

**************************************************************************/

int ias_bpf_db_insert
(
    struct ias_db_connection *conn,     /* I: Open db connection */
    IAS_BPF *bpf          /* I: populated BPF structure */
);

int ias_bpf_db_populate_version
(
    struct ias_db_connection *conn,     /* I: Open db connection */
    IAS_BPF *bpf          /* I/O: populated BPF structure */
);

#endif
