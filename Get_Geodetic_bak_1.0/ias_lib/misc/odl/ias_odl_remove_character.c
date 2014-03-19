/******************************************************************************
NAME: ias_odl_remove_character

PURPOSE: Remove all occurrences of the character C from the input string 

RETURN VALUE: No return value

******************************************************************************/

void ias_odl_remove_character
(
    char *string,  /* I/O: Input string to remove characters from */
    const char c   /* I: Character to remove from the input string */
)
{
    int i, j;     /* Looping variables */

    /* Loop through each character of the input string looking for the
       input character to remove.
    */
    for (i = j = 0; string[i] != '\0'; i++)
    {
        /* Skip the input character if found */
        if (string[i] != c)
        {
            string[j++] = string[i];
        }
    }

    /* Terminate the output string */
    string[j] = '\0';
}
