// Simple INI Reader Samples - Warnings
// Author: Sebastian Jones

#include <stdio.h>

#define SIMPLE_INI_READER_IMPLEMENTATION
#include "../../simple_ini_reader.h"

int main(int argc, char **argv)
{
    SirIni ini = sir_load_from_file("warnings.ini", 0, 0);

    if (!ini) // Couldn't malloc the ini struct
        return 1;

    if (sir_has_error(ini))
    {
        printf("%s\n", ini->error);
        return 1;
    }

    for (int i = 0; i < ini->warnings_count; ++i)
        printf("%s\n", ini->warnings[i]);

    sir_free_ini(ini);

    return 0;
}
