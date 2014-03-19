/*------------------------------------------------------------------------------
ROUTINE:        ias_linked_list library containing the following functions:
                    ias_linked_list_insert_after
                    ias_linked_list_insert_before
                    ias_linked_list_add_head
                    ias_linked_list_add_tail
                    ias_linked_list_get_head_node
                    ias_linked_list_get_next_node
                    ias_linked_list_get_prev_node
                    ias_linked_list_initialize_node
                    ias_linked_list_count_nodes
                    ias_linked_list_remove_node
                    ias_linked_list_delete

PURPOSE:        This is a library of generic linked list routines.

------------------------------------------------------------------------------*/
#include <stdlib.h>
#include "ias_linked_list.h"

/*--------------------------------------------------------------------------
  Insert a new node after the current (reference) node.
--------------------------------------------------------------------------*/
void ias_linked_list_insert_after
(
    IAS_LINKED_LIST_NODE *current_node, /* I: Node to insert after */
    IAS_LINKED_LIST_NODE *new_node      /* I: New node to insert   */
)
{   
    new_node->next = current_node->next;
    new_node->prev = current_node;
    current_node->next->prev = new_node;
    current_node->next = new_node;
}  /* END ias_linked_list_insert_after */


/*--------------------------------------------------------------------------
  Insert a new node before the current (reference) node.
--------------------------------------------------------------------------*/
void ias_linked_list_insert_before
(
    IAS_LINKED_LIST_NODE *current_node, /* I: Node to insert before */
    IAS_LINKED_LIST_NODE *new_node      /* I: New node to insert    */
)
{
    new_node->next = current_node;
    new_node->prev = current_node->prev;
    current_node->prev->next = new_node;
    current_node->prev = new_node;
}  /* END ias_linked_list_insert_before */


/*--------------------------------------------------------------------------
  Insert a new node at the head of the list.
--------------------------------------------------------------------------*/
void ias_linked_list_add_head
(
    IAS_LINKED_LIST_NODE *base_node, /* I: Base node of the linked list */
    IAS_LINKED_LIST_NODE *new_node   /* I: New node to insert    */
)
{
    ias_linked_list_insert_after( base_node, new_node );
}  /* END ias_linked_list_add_head */


/*--------------------------------------------------------------------------
  Insert a new node at the tail of the list.
--------------------------------------------------------------------------*/
void ias_linked_list_add_tail
(
    IAS_LINKED_LIST_NODE *base_node, /* I: Base node of the linked list */
    IAS_LINKED_LIST_NODE *new_node   /* I: New node to insert    */
)
{
    ias_linked_list_insert_before( base_node, new_node );
}  /* END ias_linked_list_add_tail */


/*--------------------------------------------------------------------------
  Return the head node.  Note this returns the first node inserted after the
  base node.
--------------------------------------------------------------------------*/
IAS_LINKED_LIST_NODE *ias_linked_list_get_head_node
(
    IAS_LINKED_LIST_NODE *base_node  /* I: Base node of the linked list */
)
{
    if ( base_node->next == base_node )
        return NULL;
    else
        return base_node->next;
}  /* END ias_linked_list_get_head_node */


/*--------------------------------------------------------------------------
  Return the node after the reference node.
--------------------------------------------------------------------------*/
IAS_LINKED_LIST_NODE *ias_linked_list_get_next_node
(
    IAS_LINKED_LIST_NODE *base_node,     /* I: Base node of the linked list */
    IAS_LINKED_LIST_NODE *reference_node /* I: Reference node */
)
{
    if ( reference_node->next == base_node )
        return NULL;
    else
        return reference_node->next;
}  /* END ias_linked_list_get_next_node */


/*--------------------------------------------------------------------------
  Return the node before the reference node.
--------------------------------------------------------------------------*/
IAS_LINKED_LIST_NODE *ias_linked_list_get_prev_node
(
    IAS_LINKED_LIST_NODE *base_node,     /* I: Base node of the linked list */
    IAS_LINKED_LIST_NODE *reference_node /* I: Reference node */
)
{
    if ( reference_node->prev == base_node )
        return NULL;
    else
        return reference_node->prev;
}  /* END ias_linked_list_get_prev_node */


/*--------------------------------------------------------------------------
  Initialize a linked list node.  This simply assigns the address of the
  node to the next and prev pointers.
--------------------------------------------------------------------------*/
void ias_linked_list_initialize_node
(
    IAS_LINKED_LIST_NODE *reference_node /* I: Reference node */
)
{
    reference_node->next = reference_node;
    reference_node->prev = reference_node;
}  /* END ias_linked_list_initialize_node */


/*--------------------------------------------------------------------------
  Remove a node from the linked list.  Note this does not free the memory.
--------------------------------------------------------------------------*/
void ias_linked_list_remove_node
(
    IAS_LINKED_LIST_NODE *node_to_remove /* I: The node to remove from list */
)
{
    node_to_remove->prev->next = node_to_remove->next;
    node_to_remove->next->prev = node_to_remove->prev;
}


/*--------------------------------------------------------------------------
  Count the nodes in a linked list.
--------------------------------------------------------------------------*/
unsigned int ias_linked_list_count_nodes
(
    IAS_LINKED_LIST_NODE *base_node  /* I: Base node of the linked list */
)
{
    unsigned int count = 0;
    IAS_LINKED_LIST_NODE *current = base_node->next;

    while ( current != base_node )
    {
        count++;
        current = current->next;
    }

    return count;
}  /* END ias_linked_list_remove_node */


/*--------------------------------------------------------------------------
  Delete all nodes, except the base node, from the linked list.
--------------------------------------------------------------------------*/
void ias_linked_list_delete
(
    IAS_LINKED_LIST_NODE *base_node,    /* I: Base node of the linked list */
    size_t offset                       /* I: Offset into external structure
                                              of the base node */
)
{
    IAS_LINKED_LIST_NODE *current = base_node->next;
    IAS_LINKED_LIST_NODE *next;

    /* Loop through the list */
    while ( current != base_node )
    {
        next = current->next;
        ias_linked_list_remove_node( current );
        free( (void *)current - offset );
        current = next;
    }  /* END while */
        
}  /* END ias_linked_list_delete */

