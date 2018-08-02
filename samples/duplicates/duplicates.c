// Simple INI Reader Samples - Duplicates
// Author: Sebastian Jones

#include <stdio.h>

#define SIMPLE_INI_READER_IMPLEMENTATION
#include "../../simple_ini_reader.h"

int main(int argc, char **argv)
{
    printf("\n");

    SirIni ini = sir_load_from_file("duplicates.ini", 0, 0);

    if (!ini) // Couldn't malloc the ini struct
        return 1;

    if (sir_has_error(ini))
    {
        printf("%s\n", ini->error);
        return 1;
    }

    // By Default, if a key is found to have the same name as a previous
    // key in the same section, it is ignored.
    const char *str = sir_section_str(ini, "section1", "key");
    printf("This should be 'foo': '%s'\n", str);

    // Keys can have the same name as keys in other sections...
    str = sir_section_str(ini, "section2", "key");
    printf("This should be 'hello world': '%s'\n", str);

    // ...Unless you are searching through all the keys, in which case
    // the duplicate keys are ignored like above.
    str = sir_str(ini, "key");
    printf("This should be 'foo': '%s'\n", str);

    sir_free_ini(ini);

    printf("\n");

    // You can also specify an option to change the behaviour to override
    // previous values when duplicates are found:
    ini = sir_load_from_file("duplicates.ini", 
            SIR_OPTION_OVERRIDE_DUPLICATE_KEYS, 0);

    if (!ini) // Couldn't malloc the ini struct
        return 1;

    if (sir_has_error(ini))
    {
        printf("%s\n", ini->error);
        return 1;
    }

    str = sir_section_str(ini, "section1", "key");
    printf("This should be 'bar': '%s'\n", str);

    // Like above, this only applies to keys in the same section unless
    // you are searching through all keys
    str = sir_str(ini, "key");
    printf("This should be 'hello world': '%s'\n", str);

    sir_free_ini(ini);

    printf("\n");

    return 0;
}
