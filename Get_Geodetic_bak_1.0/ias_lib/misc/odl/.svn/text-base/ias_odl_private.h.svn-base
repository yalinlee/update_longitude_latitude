#ifndef IAS_ODL_PRIVATE_H
#define IAS_ODL_PRIVATE_H

#define INDENT_SIZE 4
#define MAX_LEN 80


int ias_odl_remove_character
(
    char *string,  /* I/O: Input string to remove characters from */
    const char c   /* I: Character to remove from the input string */
);

int ias_odl_add_or_replace_field
(
    OBJDESC *ODLTree,           /* I/O: ODL Object Tree to populate */
    const char *p_LabelName,    /* I: Field to add */
    IAS_ODL_TYPE ValueType,     /* I: What type the field is */
    const int p_MemorySize,     /* I: Total memory size of attribute values */
    void *p_MemoryAddr,         /* I: Pointer to the attribute information */
    const int nelements,        /* I: Number of attribute values */
    const int replace           /* I: Flag indicating the attribute name will
                                      be replaced */
);
#endif
