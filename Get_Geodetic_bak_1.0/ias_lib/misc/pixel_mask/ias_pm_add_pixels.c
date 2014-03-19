/*----------------------------------------------------------------------------
NAME:    ias_pm_add_pixels

PURPOSE: Add a span of pixels to the pixel mask.  If the span
         overlaps with an existing span, the overlapping areas are
         merged.

RETURNS: SUCCESS or ERROR

-----------------------------------------------------------------------------*/
#include <stdlib.h>
#include "pm_local.h"
#include "ias_linked_list.h"
#include "ias_logging.h"
#include "ias_const.h"

/* local defines for the check_merge function */
#define PREVIOUS 0
#define NEXT     1

/*----------------------------------------------------------------------------
NAME:    insert_new_node

PURPOSE: Allocate and insert a new node before or after the node
         passed in.

RETURNS: SUCCESS or ERROR
-----------------------------------------------------------------------------*/
static int insert_new_node
(
    IAS_LINKED_LIST_NODE *node,/* I: Pointer to node in pixel mask */
    int detector_index,        /* I: detector number of the span */
    int start_pixel_index,     /* I: Starting pixel of the span */
    int span_length,           /* I: Length of the span */
    PIXEL_MASK_TYPE mask,      /* I: Pixel mask */
    int insert_before_flag     /* I: flag specifying how to insert */
)
{
    IAS_PIXEL_MASK_SPAN *new_span; /* pointer to span to add */

    /* Allocate the new span */
    new_span = malloc( sizeof( *new_span ));
    if ( !new_span )
    {
        IAS_LOG_ERROR("Allocating a new span");
        return ERROR;
    }
    ias_linked_list_initialize_node( &new_span->node );

    /* Add parameter information to the new span */
    new_span->detector_index = detector_index;
    new_span->starting_pixel_index = start_pixel_index;
    new_span->length_of_span = span_length;
    new_span->pixel_mask = mask;

    /* Insert sapn in the proper location */
    if ( insert_before_flag )
        ias_linked_list_insert_before( node, &new_span->node );
    else
        ias_linked_list_insert_after( node, &new_span->node );

    return SUCCESS;
}  /* END insert_new_node */


/*----------------------------------------------------------------------------
NAME:    split_node

PURPOSE: Split a node in a pixel mask double linked list into two
         separate nodes.  This is used during the operation to add 
         a new span to the list and the the new span overlaps with
         an existing span in the list.

RETURNS: SUCCESS or ERROR
-----------------------------------------------------------------------------*/
static int split_node
(
    IAS_LINKED_LIST_NODE *node, /* I: Pointer to a node in a pixel mask */
    int split_length            /* I: Length to make the first node in the
                                      split */
)
{
    IAS_PIXEL_MASK_SPAN *split_span; /* pointer to the new span split off */
    IAS_PIXEL_MASK_SPAN *curr_span;  /* pointer to the current span */

    /* Get the span structure containing the linked list node */
    curr_span = GET_OBJECT_OF( node, IAS_PIXEL_MASK_SPAN, node );

    /* Allocate a new span */
    split_span = malloc( sizeof( *split_span ));
    if ( !split_span )
    {
        IAS_LOG_ERROR("Allocating new pixel mask span");
        return ERROR;
    }
    ias_linked_list_initialize_node( &split_span->node );

    /* Copy span information */
    split_span->detector_index = curr_span->detector_index;
    split_span->pixel_mask = curr_span->pixel_mask;

    /* Adjust the starting pixel and length */
    split_span->starting_pixel_index =
        curr_span->starting_pixel_index + split_length;
    split_span->length_of_span = curr_span->length_of_span - split_length;
    curr_span->length_of_span = split_length;

    /* Insert the split span after the current node */
    ias_linked_list_insert_after( node, &split_span->node );

    return SUCCESS;
}  /* END split_node */

/*----------------------------------------------------------------------------
NAME:    check_merge

PURPOSE: Check if the provided IAS_PIXEL_MASK_SPAN can be merged with an
    adjacent span in the provided direction. If so, merge to two spans and
    remove the adjacent span. 

RETURNS: N/A
-----------------------------------------------------------------------------*/
static void check_merge
(
    IAS_PIXEL_MASK_SPAN *curr_span,  /* I/O: Pointer to merge point */
    int direction                /* I: direction to check i.e. prev or next */
)
{
    IAS_PIXEL_MASK_SPAN *check_span;
    int curr_pixel; 
    int check_pixel;
    
    if (direction == PREVIOUS)
    {
        check_span = GET_OBJECT_OF( curr_span->node.prev, IAS_PIXEL_MASK_SPAN,
            node );

        curr_pixel = curr_span->starting_pixel_index;
        check_pixel = check_span->starting_pixel_index 
            + check_span->length_of_span;

        if (check_pixel == curr_pixel 
                && check_span->pixel_mask == curr_span->pixel_mask)
        {
            curr_span->starting_pixel_index = check_span->starting_pixel_index;
            curr_span->length_of_span += check_span->length_of_span;
            ias_linked_list_remove_node(&(check_span->node));
            free(check_span);
        }
    }
    else if (direction == NEXT)
    {
        check_span = GET_OBJECT_OF( curr_span->node.next, IAS_PIXEL_MASK_SPAN,
            node );

        curr_pixel = curr_span->starting_pixel_index 
                        + curr_span->length_of_span;
        check_pixel = check_span->starting_pixel_index;

        if (curr_pixel == check_pixel
                && curr_span->pixel_mask == check_span->pixel_mask)
        {
            curr_span->length_of_span += check_span->length_of_span;
            ias_linked_list_remove_node(&(check_span->node));
            free(check_span);
        }
    }
    else
    {
        /* do nothing */
    }

}  /* END check_merge */

int ias_pm_add_pixels
(
    IAS_PIXEL_MASK *pixel_mask_ptr, /* I: Pointer to a pixel mask structure */
    int detector_index,             /* I: Detectory number */
    int start_pixel_index,          /* I: Starting pixel */
    int length,                     /* I: Length of span */
    PIXEL_MASK_TYPE mask            /* I: Pixel mask for the span */
)
{
    IAS_LINKED_LIST_NODE *base; /* Base node of detector linked list of spans */
    IAS_LINKED_LIST_NODE *next; /* Next node pointer */
    IAS_PIXEL_MASK_SPAN *span; /* pointer to each span of the linked list */
    int new_span_length = length; /* Length of the new span, this is
                             initially a copy of the input parameter and
                             will be adjusted as needed. */
    int new_span_start = start_pixel_index;  /* Starting pixel of new span,
                             this is initially a copy of the input parameter
                             and will be adjusted as needed. */
    int new_span_end;     /* convenience for end pixel of new span */
    int curr_span_end;    /* convenience for end pixel of current span */
    int curr_span_start;  /* convenience for start pixel of current span */
    int temp_length;      /* temporary span length variable */

    /* A span length less than 1 is not legal */
    if (new_span_length < 1)
    {
        IAS_LOG_ERROR("Span length of %d is not legal (must be 1 or larger)",
            new_span_length);
        return ERROR;
    }

    /* Get the base node, if any, of the detector linked list */
    base = pixel_mask_ptr->detector_lut[detector_index];

    /* If there are no spans recorded, the base node pointer will be NULL.
    Allocate and fill a new node. */
    if ( base == NULL )
    {
        /* Allocate a new base node and assign it to the look up table */
        base = malloc( sizeof( *base ));
        if ( !base )
        {
            IAS_LOG_ERROR("Allocating new linked list node");
            return ERROR;
        }
        ias_linked_list_initialize_node( base );
        pixel_mask_ptr->detector_lut[detector_index] = base;

        /* Now allocate a new span */
        span = malloc( sizeof( *span ));
        if ( !span )
        {
            IAS_LOG_ERROR("Allocating new span");
            return ERROR;
        }
        /* Initialize the new span structure */
        ias_linked_list_initialize_node( &span->node );
        span->detector_index = detector_index;
        span->starting_pixel_index = new_span_start;
        span->length_of_span = new_span_length;
        span->pixel_mask = mask;

        /* Add it to the linked list */
        ias_linked_list_insert_after( base, &span->node );

        /* There is no more to be done so return */
        return SUCCESS;
    }  /* END if base == NULL */

    /* For convenience calculate the ending pixel location */
    new_span_end = new_span_start + new_span_length - 1;

    /* There was already a span recorded for this detector so add the span
    by looping through the linked list nodes and finding where it belongs. */
    GET_OBJECT_FOR_EACH_ENTRY( span, base, IAS_PIXEL_MASK_SPAN, node )
    {
        curr_span_start = span->starting_pixel_index;
        curr_span_end = curr_span_start + span->length_of_span - 1;

        /* If the span to add is completely before the current span,
        insert a new span before the current span. */
        if ( new_span_end < curr_span_start )
        {
            /* If the span to add and the current are connected and their pixel
            masks match extend the current span to include the new span */
            if ( new_span_end == (curr_span_start - 1) 
                    && mask == span->pixel_mask )
            {
                span->starting_pixel_index = new_span_start;
                span->length_of_span += new_span_length;

                if (ias_linked_list_get_prev_node(base, &span->node) != NULL)
                    check_merge(span, PREVIOUS);
            }
            else
            {
                insert_new_node( &span->node, detector_index, new_span_start,
                    new_span_length, mask, TRUE );
            }

            break;  /* That's all that needs to be done. */
        }

        /* Check that the span is completely after the current span.  If so,
           there is nothing to do except drop to the last check for a NULL
           next node. */
        else if ( new_span_start > curr_span_end )
        {
            ; /* Intentional empty statement */
        }

        /* Check if the new span position match the current span position, If 
           so, combine the masks and check if the span can be merged. */
        else if ( new_span_start == curr_span_start 
                && new_span_end == curr_span_end )
        {
            if ( mask != (span->pixel_mask & mask) )
            {
                span->pixel_mask |= mask;

                if (ias_linked_list_get_prev_node(base, &span->node) != NULL)
                    check_merge(span, PREVIOUS);
                if (ias_linked_list_get_next_node(base, &span->node) != NULL)
                    check_merge(span, NEXT);
            }
            break;
        }

        /* Check if the new span is contained within the current span and the
           new mask is already set. If so, nothing to do. */
        else if ( new_span_start >= curr_span_start 
                && new_span_end <= curr_span_end
                && mask == (span->pixel_mask & mask) )
        {
            break;
        }

        /* There is overlap so find the location to add the new span */
        else
        {
            /* If the new span starts before the current span, insert the
            new node before the current node and adjust start and length. */
            if ( new_span_start < curr_span_start )
            {
                if ( mask == span->pixel_mask )
                {
                    temp_length = curr_span_start - new_span_start;
                    span->starting_pixel_index = new_span_start;
                    span->length_of_span += temp_length;
                    curr_span_start = new_span_start;
                }
                else
                {
                    temp_length = curr_span_start - new_span_start;
                    insert_new_node( &span->node, detector_index,
                        new_span_start, temp_length, mask, TRUE );
                    curr_span_start = span->starting_pixel_index;
                    new_span_length -= temp_length;
                }
            }

            /* If the new span starts after the start of the current span,
            split the span. */
            if ( new_span_start > curr_span_start )
            {
                if ( mask != (span->pixel_mask & mask) )
                {
                    split_node(&span->node, new_span_start - curr_span_start);
                    next = ias_linked_list_get_next_node( base, &span->node );
                    span = GET_OBJECT_OF( next, IAS_PIXEL_MASK_SPAN, node );
                }
                curr_span_start = new_span_start;
            }

            /* The new span and current span now start at the same location. */

            /* If the current span extends past the end of the new span,
            split the current span at the end of the new span. */
            if ( curr_span_end > new_span_end 
                    && mask != (span->pixel_mask & mask) )
            {
                split_node( &span->node, new_span_length );
            }

            /* Now new span end equals or exceeds the end of current span */
            /* Logical OR the new pixel mask with the current span */
            span->pixel_mask |= mask;

            /* Attempt to merge with adjacent spans with the same mask */
            if (ias_linked_list_get_prev_node(base, &span->node) != NULL)
                check_merge(span, PREVIOUS);

            if (ias_linked_list_get_next_node(base, &span->node) != NULL)
                check_merge(span, NEXT);

            /* Calculate the new starting index and span length */
            new_span_start = span->starting_pixel_index + span->length_of_span;
            new_span_length -= (new_span_start - curr_span_start);

            /* If the new span length is zero break out of loop */
            if ( new_span_length <= 0 )
                break;
 
        }  /* END else */

        /* If the next node in the list is NULL, we are at the end of the
        linked list and should add the new span after the current span. */
        if ( ias_linked_list_get_next_node( base, &span->node ) == NULL )
        {
            if ( new_span_start == (curr_span_end + 1) 
                    && mask == span->pixel_mask )
            {
                span->length_of_span += new_span_length;
            }
            else
            {
                insert_new_node( &span->node, detector_index, new_span_start,
                    new_span_length, mask, FALSE );
            }

            break;
        }

    }  /* END for loop GET_OBJECT_FOR_EACH_ENTRY */

    return SUCCESS;
}  /* END ias_pm_add_pixels() */
