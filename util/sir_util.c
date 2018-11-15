// Public-domain Unix-style program that provides a user-interface to 
// the Simple INI Reader library, written using only the C Standard Library.
//
// Author: Sebastian Jones (http://www.sebj.co.uk)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIMPLE_INI_READER_IMPLEMENTATION
#include "../simple_ini_reader.h"

// Returns the first argument that doesn't start with '-' or "--", or
// 0 if none is found
char *arg_first_non_option(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (argv[i][1] != '-') 
                ++i;
        }
        else
        {
            return argv[i];
        }
    }

    return 0;
}

// Returns the index of the first argument that matched 'arg', or -1
// if not found.
int arg_index(int argc, char **argv, char *arg)
{
    for (int i = 1; i < argc; ++i)
        if (!strcmp(argv[i], arg))
            return i;

    return -1;
}

// Returns the operand of 'option' or 0 if the option is not found or
// an operand is not specified.
char *arg_operand(int argc, char **argv, char *option)
{
    int option_index = arg_index(argc, argv, option);
    if (option_index == -1)
        return 0;

    int operand_index = option_index + 1;
    if (operand_index >= argc)
        return 0;

    return argv[operand_index];
}

// Prints a usage message to Standard Output
void print_help()
{
    printf("\n\tsir [-s section_name] [-k key_name] [FILENAME]\n\n"
            "\tParses INI data and prints the value of the specified\n"
            "\tkey from the specified section. If 'FILENAME' is not\n"
            "\tspecified, the program attempts to read the data from\n"
            "\tStandard Input (pipes and redirection only).\n\n"
            "\tOPTIONS\n\n"
            "\t-s section_name\t\tLook only in the given section.\n"
            "\t\t\t\tIf omitted, all sections are used.\n\n"
            "\t-k key_name\t\tFind the value of this key only.\n"
            "\t\t\t\tIf omitted, all values are listed.\n\n"
            "\t--help\t\t\tDisplay this help screen.\n\n"
            "\t--list-keys\t\tList key names. If used with the '-s'\n"
            "\t\t\t\toption, lists the key names in that section.\n\n"
            "\t--list-sections\t\tList section names.\n\n"
          );
}

#define arg_exists(argc, argv, arg) arg_index(argc, argv, arg) != -1

int main(int argc, char **argv)
{
    if (arg_exists(argc, argv, "--help"))
    {
        print_help();
        return 0;
    }

    // Parse INI

    SirIni ini = 0;

    char *filename = arg_first_non_option(argc, argv);
    if (filename)
    {
        ini = sir_load_from_file(filename, 0, 0);
    }
    else
    {
        // Attempt to read INI from Standard Input
        fseek(stdin, 0, SEEK_END);
        size_t size = ftell(stdin);

        if (size == EOF)
        {
            print_help();
            return 1;
        }

        rewind(stdin);

        char *str = malloc(size + 1);

        size_t bytes_read = fread(str, 1, size, stdin);
        if (bytes_read < size)
        {
            size = bytes_read;
            str = realloc(str, size + 1);
        }

        str[size] = '\0';

        ini = sir_load_from_str(str, 0, "stdin", 0);
    }

    if (!ini)
    {
        fprintf(stderr, "Something went seriously wrong\n");
        return 1;
    }

    if (sir_has_error(ini))
    {
        fprintf(stderr, "%s\n", ini->error);
        return 1;
    }

    for (int i = 0; i < ini->warnings_count; ++i)
        fprintf(stderr, "%s\n", ini->warnings[i]);

    // Do action specified by args
    char *section = arg_operand(argc, argv, "-s");

    if (arg_exists(argc, argv, "--list-sections"))
    {
        for (int i = 0; i < ini->section_count; ++i)
            if (strcmp(ini->section_names[i], SIR_GLOBAL_SECTION_NAME))
                printf("%s\n", ini->section_names[i]);
    }
    else if (arg_exists(argc, argv, "--list-keys"))
    {
        if (section)
        {
            int names_size;
            const char **names = sir_section_key_names(ini, section, 
                    &names_size);

            for (int i = 0; i < names_size; ++i)
                printf("%s\n", names[i]);
        }
        else
        {
            for (int i = 0; i < ini->key_count; ++i)
                printf("%s\n", ini->key_names[i]);
        }
    }
    else
    {
        char *key = arg_operand(argc, argv, "-k");
        if (key)
        {
            const char *value = sir_section_str(ini, section, key);
            if (sir_has_error(ini))
            {
                fprintf(stderr, "%s\n", ini->error);
                return 1;
            }

            printf("%s\n", value);
        }
        else
        {
            if (section)
            {
                int values_size;
                const char **values = sir_section_key_values(ini, 
                        section, &values_size);

                for (int i = 0; i < values_size; ++i)
                    printf("%s\n", values[i]);
            }
            else
            {
                for (int i = 0; i < ini->key_count; ++i)
                    printf("%s\n", ini->key_values[i]);
            }
        }
    }

    sir_free_ini(ini);

    return 0;
}
