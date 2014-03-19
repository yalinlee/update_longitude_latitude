/******************************************************************************
NAME: ias_odl_add_or_replace_field

PURPOSE:  Add or replace the specified ODL field to the ODL object tree
          if the input parameter replace is true the attribute name will 
          be replaced, if false will add the attribute.

NOTE:     An array of strings should allow each quoted string in the array to
          contain spaces, however the library code uses strtok to parse out the 
          values when the ODL field is a set or sequence, therefore the spaces
          are removed in this case


RETURN VALUE: SUCCESS or ERROR

******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "toolbox.h"
#include "lablib3.h"
#include "ias_odl.h"
#include "ias_odl_private.h"
#include "ias_logging.h"

extern char ODLErrorMessage[];       /* External Variables */

int ias_odl_add_or_replace_field
(
    OBJDESC *curr_object,       /* I/O: Object tree to populate */
    const char *p_LabelName,    /* I: Field to add */
    IAS_ODL_TYPE ValueType,     /* I: What type the field is */
    const int p_MemorySize,     /* I: Total memory size of attribues values */
    void *p_MemoryAddr,         /* I: Pointer to the attribute information */
    const int nelements,        /* I: Number of attribute values */
    const int action_flag       /* I: Flag indicated the attribute name will 
                                      be replaced */
)
{
    int i;                       /* Looping variable */
    int sz;                      /* Memory size base on data type */
    int count = 0;               /* Count of attribute values */
    int total_indentation;       /* Total number of indent characters */
    int curr_length;             /* Current length of keyword string */
    int tmp_len;                 /* Length of the temporary string */
    char tmp_string[TB_MAXLINE]; /* Temporary string for converting values */
    char *value_to_paste = NULL; /* Pointer to ODL keyword values */
    char newline_blanks[MAX_LEN] = "\n"; /* String for newline and indenting */
    const char *p_ClassName = NULL; /* Group/Object name */
    KEYWORD *new_keyword = NULL; /* Pointer to new ODL keyword */
    KEYWORD *curr_keyword = NULL;/* Pointer to current ODL keyword */
    KEYWORD *ptr_keyword = NULL; /* Pointer to cut ODL keyword */
    char *cptr = NULL;           /* Pointer to the current value */
    OBJDESC *p_lp = NULL;        /* Pointer to ODL object */

    /* Check to make sure a valid Label Name was specified */
    if ((p_LabelName == NULL) || (strlen(p_LabelName) == 0))
    {
        IAS_LOG_ERROR("Invalid attribute name");
        return ERROR;
    }

    /* Check to make sure there are values to convert */
    if ((p_MemorySize <= 0) || (nelements <= 0))
    {
        IAS_LOG_ERROR("Undefined attribute size");
        return ERROR;
    }

    /* Calculate the indentation size */
    total_indentation = curr_object->level * INDENT_SIZE;
    
    /* ensure indentation does not overrun newline buffer size */
    if (total_indentation > 78)
        total_indentation = 78;

    /* Calculate the length of the output string.  This includes the
       indentation, label name, and equal sign surrounded by spaces " = " */
    curr_length = total_indentation + strlen(p_LabelName) + 3;

    /* Add blanks for the indentation */
    for(i = 0; i < total_indentation ; i++)
        strcat(newline_blanks," ");

    /* Determine the size of the data input */
    sz = p_MemorySize / nelements;
    cptr = (char *)p_MemoryAddr;

    if (nelements > 1)
    {
        /* Start the array of strings with a left parenthesis */
        CopyString(value_to_paste, "(");
        curr_length++;
    }

    while (count < nelements)
    {
        /* Convert the value at the current position to a
           formatted string */
        switch (ValueType)
        {
            case IAS_ODL_String:
                sprintf(tmp_string, "\"%s\"", (char *)cptr);
                break;
            case IAS_ODL_ArrayOfString:
                sprintf(tmp_string, "\"%s\"", (char *)cptr);
                cptr += sz;
                break;
            case IAS_ODL_Int:
                sprintf(tmp_string, "%d", *(int *)cptr);
                cptr += sizeof(int);
                break;
            case IAS_ODL_Long:
                sprintf(tmp_string, "%ld", *(long *)cptr);
                cptr += sizeof(long);
                break;
            case IAS_ODL_Float:
                sprintf(tmp_string, "%G", *(float *)cptr);
                cptr += sizeof(float);
                break;
            case IAS_ODL_Double:
                sprintf(tmp_string, "%-4.14G", *(double *)cptr);
                cptr += sizeof(double);
                break;
            case IAS_ODL_Sci_Not:
                sprintf(tmp_string, "%-4.11G", *(double *)cptr);
                cptr += sizeof(double);
                break;
        }

        /* Remove any extra spaces exceptt in array of strings type */
          if ( ValueType != IAS_ODL_String )
             ias_odl_remove_character(tmp_string,' ');

        /* Determine if the line needs to be wrapped.
           The 2 accounts for the comma and space or
           the ending paranthesis.
        */
        tmp_len = strlen(tmp_string);
        if ((curr_length + tmp_len + 2) > MAX_LEN)
        {
            /* Add a new line and indentation */
            AppendString(value_to_paste, newline_blanks);

            /* Reset the current string length */
            curr_length = tmp_len + strlen(newline_blanks);
        }

        /* Append the formatted string to the value to paste */
        AppendString(value_to_paste, tmp_string);
        curr_length += tmp_len;

        /* Increment the element counter */
        count++;

        if (count < nelements)
        {
            /* Append a comma and space to separate values */
            AppendString(value_to_paste, ", ");
            curr_length += 2;
        }
    }

    if (nelements > 1)
    {
        /* End the array of strings with a left parenthesis */
        AppendString(value_to_paste, ")");
        curr_length++;
    }
    
    if (action_flag == TRUE)
    { 
        /* find ODL object */
        if ((p_lp = OdlFindObjDesc(curr_object, p_ClassName, 
                    p_LabelName, NULL, 1, 
                    ODL_RECURSIVE_DOWN)) == NULL)
        {
            if ((long)strlen(ODLErrorMessage) <= 1 )
            {
                IAS_LOG_ERROR("%s", ODLErrorMessage);
                IAS_LOG_ERROR("Object %s not found with %s keyword", 
                    (p_ClassName) ? (p_ClassName) : "null", p_LabelName);
            }
            free(value_to_paste);
            return ERROR;
        }

        /* search for keyword */
        if ((curr_keyword = OdlFindKwd(p_lp, p_LabelName, NULL, 1,
            ODL_RECURSIVE_DOWN)) == NULL)
        {
            if ((long)strlen(ODLErrorMessage) <= 1 )
            {
                IAS_LOG_ERROR("%s", ODLErrorMessage);
                IAS_LOG_ERROR("Keyword %s not found", p_LabelName);
            }
            free(value_to_paste);
            return ERROR;
        }

        /* cut old keyword */
        ptr_keyword = OdlCutKwd(curr_keyword);
    } /* end of replace field */

    /* Paste the new label, with values, into the ODL Object Tree */
    new_keyword = OdlNewKwd((char *)p_LabelName, (char *)value_to_paste,
                            NULL, NULL, NULL, 0);

    /* Free the space allocated to the value_to_paste pointer */
    free(value_to_paste);

    curr_keyword = OdlPasteKwd(new_keyword, curr_object);
    if (!curr_keyword)
    {
        IAS_LOG_ERROR("Add attribute to tree failed");
        return ERROR ;
    }
    /* free old keyword */
    OdlFreeKwd(ptr_keyword);

    return SUCCESS;
}
