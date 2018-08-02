// Simple INI Reader Samples - Basics
// Author: Sebastian Jones

#include <stdio.h>

#define SIMPLE_INI_READER_IMPLEMENTATION
#include "../../simple_ini_reader.h"

int main(int argc, char **argv)
{
    SirIni ini = sir_load_from_file("basics.ini", 0, 0);

    if (!ini) // Couldn't malloc the ini struct
        return 1;

    if (sir_has_error(ini))
    {
        printf("%s\n", ini->error);
        return 1;
    }

    // Note that you can use the SIR_GLOBAL_SECTION_NAME macro 
    // instead of "global"
    const char *str = sir_section_str(ini, "global", "key");
    if (sir_has_error(ini))
        printf("%s\n", ini->error);
    else 
        printf("key = \"%s\"\n", str);

    str = sir_section_str(ini, "section1", "key1");
    if (sir_has_error(ini))
        printf("%s\n", ini->error);
    else 
        printf("key1 = \"%s\"\n", str);

    // These functions convert using the stdlib functions, and handle stdlib
    // errors.
    long l = sir_section_long(ini, "section2", "key2");
    if (sir_has_error(ini))
        printf("%s\n", ini->error);
    else 
        printf("key2 = \"%li\"\n", l);
    
    double d = sir_section_double(ini, "section2", "key3");
    if (sir_has_error(ini))
        printf("%s\n", ini->error);
    else 
        printf("key3 = \"%f\"\n", d);

    char b = sir_section_bool(ini, "section2", "key4");
    if (sir_has_error(ini))
        printf("%s\n", ini->error);
    else 
        printf("key4 = \"%i\"\n", b);

    // _section can be ommitted from the above functions to search all keys in
    // the INI file, regardless of what section they belong to

    str = sir_str(ini, "key3");
    if (sir_has_error(ini))
        printf("%s\n", ini->error);
    else 
        printf("\nkey3 = \"%s\"\n", str);

    // Free the INI
    sir_free_ini(ini);

    return 0;
}
