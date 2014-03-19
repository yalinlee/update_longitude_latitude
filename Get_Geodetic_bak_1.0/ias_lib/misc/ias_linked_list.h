#ifndef IAS_LINKED_LIST_H
#define IAS_LINKED_LIST_H

#include <stddef.h>

/*----------------------------------------------------------------------------
   Generic linked list library

The linked list library uses a structure, containing a previous and next
pointer, for all access and traversal of a linked list.  Users of the
library must embed the IAS_LINKED_LIST_NODE structure within their own
structure.

To use this library define a structure that contains the node structure:

    typedef struct my_struct
    {
        IAS_LINKED_LIST_NODE node;
        int member;
        ...
    } MY_STRUCT;

The IAS_LINKED_LIST_NODE does not have to be the first member of the
structure.  Use a IAS_LINKED_LIST_NODE as the head of the list:

    IAS_LINKED_LIST_NODE my_linked_list_head;

which must be initialized:

    ias_linked_list_initialize_node( &my_linked_list_head );

To add your structure nodes to the linked list:

    IAS_LINKED_LIST_NODE *current = some_node_of_linked_list;
    MY_STRUCT *my_struct_node;
    my_struct_node = malloc( sizeof( MY_STRUCT ));
    ias_linked_list_initialize_node( &my_struct_node->node );
    ias_linked_list_insert_after( current, &my_struct_node->node );

There are routines to insert before a node, after a node and to add a new
head or tail.  The head node must be used for the add routines.  A pointer
to any node, including the head node, may be used for the insert routines.

    ias_linked_list_insert_before( current, &my_struct_node->node );
    ias_linked_list_insert_after( current, &my_struct_node->node );
    ias_linked_list_add_head( &my_linked_list_head, &my_struct_node->node );
    ias_linked_list_add_tail( &my_linked_list_head, &my_struct_node->node );

Three routines to get pointers to nodes are provided.  Note that the head
node returned is the first MY_STRUCT node inserted after the
my_linked_list_head node.  Each of these routines return a pointer to a
IAS_LINKED_LIST_NODE or NULL if at the head or tail of the list.  Note:
these functions will not return the base node.

    ias_linked_list_get_head_node( &my_linked_list_head );
    ias_linked_list_get_next_node( &my_linked_list_head, current );
    ias_linked_list_get_prev_node( &my_linked_list_head, current );

One routine returns the number of nodes in the list (not including the
my_linked_list_head node).

    ias_linked_list_count_nodes( &my_linked_list_head );

One routine initializes the next and prev values in the IAS_LINKED_LIST_NODE
structure to point to itself.  The insert and add routines rely on the nodes
being initialized in this manner.

    ias_linked_list_initialize_node( &my_struct_node->node );

One routine removes a node from the list.  It does not free the memory.

    ias_linked_list_remove_node( current );

One routine deletes all nodes, except the my_linked_list_head node.  The
memory for the nodes is freed.  Since the generic routine has no knowledge
of the structure containing the IAS_LINKED_LIST_NODE structure, the offset
of the node structure to the containing object's address must be provided.

    ias_linked_list_delete( &my_linked_list_head, offset );

In practice it may be desirable to define a macro in the user's code to
simplify the call.  Example:

    #define LINKED_LIST_DELETE ( list ) \
        ias_linked_list_delete( (list), offsetof( MY_STRUCT, node ));

Finally there are two convenience macros for accessing the objects
containing the node structure.  The first simply returns the containing
object pointer:

    my_struct_node = GET_OBJECT_OF( current );

The other is a loop that accesses each object in the list:

    GET_OBJECT_FOR_EACH_ENTRY( my_struct_node, &my_linked_list_head,
        MY_STRUCT, node )

This macro is expanded as a for loop and must be treated as such: don't end
it with a semicolon and add the loop body.  Example:

    GET_OBJECT_FOR_EACH_ENTRY( my_struct_node, &my_linked_list_head,
        MY_STRUCT, node )
    {
        value = my_struct_node->node_member;
        ...
    }

-----------------------------------------------------------------------------*/
/* This is the linked list node structure.  It will be used to access the
nodes of the linked list. */
typedef struct ias_linked_list_node
{
    struct ias_linked_list_node *next;
    struct ias_linked_list_node *prev;
} IAS_LINKED_LIST_NODE;


/* Macros */
/*------------------------------------------------------------------------
 Get a pointer to the object containing the linked list node structure.
           object: pointer to object
    parent_struct: object's partent structure type
      node_member: name of object's linked list node member
------------------------------------------------------------------------*/
#define GET_OBJECT_OF(object, parent_struct, node_member) \
    ((parent_struct *)((char *)(object) - \
    (unsigned long)(&((parent_struct *)0)->node_member)))

/*------------------------------------------------------------------------
Loop through a linked list.  At each interation of the for loop the first
argument is a pointer to an object containing the linked list node
structure.
           object: pointer to object
             head: pointer to head node of the linked list
    parent_struct: object's partent structure type
      node_member: name of object's linked list node member
------------------------------------------------------------------------*/
#define GET_OBJECT_FOR_EACH_ENTRY(object, head, parent_struct, node_member) \
    for ((object) = GET_OBJECT_OF((head)->next, parent_struct, node_member); \
        &(object)->node_member != (head); \
        (object) = GET_OBJECT_OF((object)->node_member.next, parent_struct, \
        node_member))


/* Function prototypes */
void ias_linked_list_insert_after
(
    IAS_LINKED_LIST_NODE *current_node, /* I: Node to insert after */
    IAS_LINKED_LIST_NODE *new_node      /* I: New node to insert   */
);

void ias_linked_list_insert_before
(
    IAS_LINKED_LIST_NODE *current_node, /* I: Node to insert before */
    IAS_LINKED_LIST_NODE *new_node      /* I: New node to insert    */
);

void ias_linked_list_add_head
(
    IAS_LINKED_LIST_NODE *base_node, /* I: Base node of the linked list */
    IAS_LINKED_LIST_NODE *new_node   /* I: New node to insert    */
);

void ias_linked_list_add_tail
(
    IAS_LINKED_LIST_NODE *base_node, /* I: Base node of the linked list */
    IAS_LINKED_LIST_NODE *new_node   /* I: New node to insert    */
);

IAS_LINKED_LIST_NODE *ias_linked_list_get_head_node
(
    IAS_LINKED_LIST_NODE *base_node  /* I: Base node of the linked list */
);

unsigned int ias_linked_list_count_nodes
(
    IAS_LINKED_LIST_NODE *base_node  /* I: Base node of the linked list */
);

IAS_LINKED_LIST_NODE *ias_linked_list_get_next_node
(
    IAS_LINKED_LIST_NODE *base_node,     /* I: Base node of the linked list */
    IAS_LINKED_LIST_NODE *reference_node /* I: Reference node */
);

IAS_LINKED_LIST_NODE *ias_linked_list_get_prev_node
(
    IAS_LINKED_LIST_NODE *base_node,     /* I: Base node of the linked list */
    IAS_LINKED_LIST_NODE *reference_node /* I: Reference node */
);

void ias_linked_list_initialize_node
(
    IAS_LINKED_LIST_NODE *reference_node /* I: Reference node */
);

void ias_linked_list_remove_node
(
    IAS_LINKED_LIST_NODE *node_to_remove /* I: The node to remove from list */
);

void ias_linked_list_delete
(
    IAS_LINKED_LIST_NODE *base_node,    /* I: Base node of the linked list */
    size_t offset                       /* I: Offset into external structure
                                              of the base node */
);

#endif
