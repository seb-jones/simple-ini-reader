// Simple INI Reader Samples - Iterating
// Author: Sebastian Jones

#include <stdio.h>

#define SIMPLE_INI_READER_IMPLEMENTATION
#include "../../simple_ini_reader.h"

int main(int argc, char **argv)
{
    SirIni ini = sir_load_from_file("iterating.ini", 0, 0);

    if (!ini) // Couldn't malloc the ini struct
        return 1;

    if (sir_has_error(ini))
    {
        printf("%s\n", ini->error);
        return 1;
    }

    // Key names and values are stored in parrallel arrays and can be
    // iterated over trivially:
    printf("\nKEY NAMES:\n");
    for (int i = 0; i < ini->key_count; ++i)
        printf("%s\n", ini->key_names[i]);

    printf("\nKEY VALUES:\n");
    for (int i = 0; i < ini->key_count; ++i)
        printf("%s\n", ini->key_values[i]);

    // You can also get arrays containing the key names and values from
    // a specific section. Note that these functions use malloc due
    // to how the keys and sections are stored internally.
    int size;
    const char **names = sir_section_key_names(ini, "section1", &size);

    printf("\nSECTION 1 KEY NAMES\n");
    for (int i = 0; i < size; ++i)
        printf("%s\n", names[i]);

    const char **values = sir_section_key_values(ini, "section2", &size);

    printf("\nSECTION 2 KEY VALUES\n");
    for (int i = 0; i < size; ++i)
        printf("%s\n", values[i]);

    // Don't forget to free them!
    sir_free(ini, names);
    sir_free(ini, values);

    sir_free_ini(ini);

    return 0;
}
