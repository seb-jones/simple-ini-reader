// Public Domain, single-file INI reader using only the C Standard Library.
// Author: Sebastian Jones
//
// Features
// ========
//
// Currently Supported:
//  - Keys using '=' and ':'
//  - Comments using ';' and '#', anywhere on a line
//  - Double-quotes to preserve whitespace
//  - Reading values as string
//  - Converting values to long, unsigned long, double or bool.
//  - Converting values to an array of strings, splitting by comma (,).
//  - Optional case-insensitivity
//  - Options to ignore or override keys with duplicated names
//  - Optional warnings to detect probable mistakes in an INI
//  - Optional errors
//  - Customizable malloc, realloc, free
//
// Currently NOT Supported:
//  - Ignoring newlines with '\'
//  - Programmatic interpretation of sub-sections/nested sections. You
//    can make the illusion of sub-sections by indenting with white-
//    space or using a name like [A.B.C] but the parser interprets all
//    sections as being on the same level.
//
// UNTESTED, but might work:
//  - Unicode
//  - Thread-safety. Loading different files in seperate threads should 
//    be okay in theory, because none of these functions are meant to
//    touch any outside state other than their ini pointer parameter.
//  - Files bigger than 2GB. Currently we use the 'fseek(END), ftell()' 
//    trick to get the file size and we use ints to store array sizes most of
//    the time. But if you have an INI file bigger than 2GB you might want
//    to think about choosing a more appropriate file format.
//
// Terminology
// ===========
//
// This library describes INI files in terms of sections and keys. Sections are
// surrounded by square brackets, e.g.:
//
//      [ graphics ]
//
// Keys have a name and a value, delimited by an equals sign (=) or a
// colon (:). You may not have more than one key per line. e.g.:
//
//      window_width = 1920
//      window_height:1080
//
// Surrounding whitespace is trimmed by default. Key values may be surrounded 
// by double-quotes ("") to preserve whitespace.
//
// Comments can be placed anywhere on a line using semi-colon (;) or 
// hash (#), e.g.:
//
//      ; this is a comment
//      window_height = 1080 #this comment is also okay
//
// Any keys define before a section has been defined are said to be in the 
// 'global' section. e.g:
//
//      this_key=is_global
//
//      [ this_is_the_first_section ]
//
//      this_key=is_not_global
//
// Basic Usage
// ===========
//
// (For detailed examples, please see the 'samples' folder in the repo)
//
// Define SIMPLE_INI_READER_IMPLEMENTATION in one of your c/c++ files before
// including simple_ini_reader.h, e.g:
//
//      #define SIMPLE_INI_READER_IMPLEMENTATION
//      #include "simple_ini_reader.h"
//
// Load an INI file and get values from keys, e.g:
//
//      SirIni ini = sir_load_from_file("foo.ini", options, 0);
//
//      const char *str = sir_str(ini, "key_name");
//
//      sir_free_ini(ini);
//
// You can also get values from keys belonging to specific sections:
//
//      str = sir_section_str(ini, "section_name", "key_name");
//      glob_str = sir_section_str(ini, SIR_GLOBAL_SECTION_NAME, "key_name");
//
// There are some conversion functions for primitive types. All have the same
// parameters:
//
//      long l = sir_section_long(ini, "section_name", "key_name");
//      unsigned long ul = sir_section_unsigned_long();
//      double d = sir_section_double();
//      char b = sir_section_bool();
//
// There is also a function that tries to split a string by commas:
//
//      int *csv_size;
//      const char **csv = sir_csv(ini,  "key_name", &csv_size);
//
//      sir_free_csv(csv);
//
// Custom Memory Management
// ========================
//
// You can override the stdlib malloc, realloc and free by defining the
// following macros:
//
//      SIR_MALLOC (ctx, size)
//      SIR_FREE   (ctx, mem)
//      SIR_REALLOC(ctx, mem, size)
//
// You can optionally pass a context as a (void *) to one of the load
// functions:
//
//      SirIni sir_load_from_str(char *s, SirOptions options, 
//                                  const char *name, void *mem_ctx);
//
//      SirIni sir_load_from_file(const char *filename, 
//                                  SirOptions options, void *mem_ctx);
//
// By default the malloc, realloc and free macros above expand to the
// stdlib functions and thus ignore the mem_ctx. It is purely optional.

#ifndef SIMPLE_INI_READER_HEADER
#define SIMPLE_INI_READER_HEADER

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

typedef enum SirOptions
{
    SIR_OPTION_NONE                     = 0x000,

    // Key values that start with a null terminator '\0' are considered empty
    SIR_OPTION_IGNORE_EMPTY_VALUES      = 0x001,

    // See the 'duplicates' sample for an example of using this
    SIR_OPTION_OVERRIDE_DUPLICATE_KEYS  = 0x002,

    // Double quotes will be part of the value, rather than being parsed out
    SIR_OPTION_DISABLE_QUOTES           = 0x004,

    // Only ';' will denote a comment
    SIR_OPTION_DISABLE_HASH_COMMENTS    = 0x008,

    // Only '=' will be used to seperate key names and values
    SIR_OPTION_DISABLE_COLON_ASSIGNMENT = 0x010,

    // A line will only be a comment if '\n' is directly before the comment
    SIR_OPTION_DISABLE_COMMENT_ANYWHERE = 0x020,

    // Both section names and key names and values will be case-insensitive
    SIR_OPTION_DISABLE_CASE_SENSITIVITY = 0x040,

    // Will save a small amount of memory and may improve performance
    SIR_OPTION_DISABLE_ERRORS           = 0x080,

    // Disabling warnings may improve performance since an extra pass over
    // the string is done to detect warnings, and each warning string is
    // 512 bytes by default
    SIR_OPTION_DISABLE_WARNINGS         = 0x100,
}
SirOptions;

typedef struct SirSectionRange
{
    int start;
    int end;
}
SirSectionRange;

typedef struct SirSection
{
    SirSectionRange *ranges;
    int ranges_count;
}
SirSection;

typedef struct SirIniStruct
{
    void *mem_ctx;
    char *data;
    SirSection *sections;
    const char **section_names;
    const char **key_names;
    const char **key_values;
    const char *filename;
    char *error;
    char *error_msg;
    const char **warnings;
    int section_count;
    int key_count;
    SirOptions options;
    int error_size;
    int warnings_count;
    int warnings_size;
}
SirIniStruct;

typedef SirIniStruct * SirIni;

#ifdef SIR_STATIC
#define SIRDEF static
#else
#define SIRDEF extern
#endif

// 
// 'PUBLIC' FUNCTIONS
// ==================

// Parse the given string as an INI file. Note that this will modify the
// string 's'. 'options' should be one or more of the SIR_OPTIONS_ defined in
// enum SirOptions bitwise-OR'd together, or 0. 'name' is optional and is used
// when making warnings and errors. 'mem_ctx' is optional and only relevant 
// if you use custom memory allocation.
SIRDEF SirIni sir_load_from_str(char *s, SirOptions options, 
        const char *name, void *mem_ctx);

// Same as sir_load_from_str(), except that 'filename' is the name of a file
// that will be loaded using stdio functions.
SIRDEF SirIni sir_load_from_file(const char *filename, SirOptions options, 
        void *mem_ctx);

// Frees the given ini.
SIRDEF void sir_free_ini(SirIni ini);

// Retrieves the value of the key 'key_name' in the section 'section_name' as
// a (const char *).
SIRDEF const char *sir_section_str(SirIni ini, const char *section_name, 
        const char *key_name);

// Retrieves the value of the key 'key_name' in the section 'section_name' and
// converts it to a long using strtol()
SIRDEF long sir_section_long(SirIni ini, const char *section_name, 
        const char *key_name);

// Retrieves the value of the key 'key_name' in the section 'section_name' and
// converts it to a unsigned long using strtoul()
SIRDEF long sir_section_unsigned_long(SirIni ini, const char *section_name, 
        const char *key_name);

// Retrieves the value of the key 'key_name' in the section 'section_name' and
// converts it to a double using strtod()
SIRDEF double sir_section_double(SirIni ini, const char *section_name, 
        const char *key_name);

// Retrieves the value of the key 'key_name' in the section 'section_name' and
// converts it to 1 if it is non-zero or the string 'true', or 0 if it is zero
// or the string 'false'. Returns -1 if the string couldn't be interpreted in
// one of these ways. Always case-insensitive.
SIRDEF char sir_section_bool(SirIni ini, const char *section_name, 
        const char *key_name);

// Retrieves the value of the key 'key_name' in the section 'section_name' and
// converts it to an array of (const char *) by splitting the string at every
// ',' character. The size of the resulting array is stored in the location
// pointed to by 'csv_size_ret'. Note that this function performs a memory
// allocation.
SIRDEF const char **sir_section_csv(const SirIni ini, 
        const char *section_name, const char *key_name, int *csv_size_ret);

// Frees a CSV that was returned by the above function.
SIRDEF void sir_free_csv(SirIni ini, const char **csv);

// Returns an array of all the key names that belong in the section 
// 'section_name'. The size of the resulting array is stored in the location
// pointed to by 'values_size_ret'. Note that this function performs a 
// memory allocation.
SIRDEF const char **sir_section_key_names(SirIni ini, 
        const char *section_name, int *names_size_ret);

// Returns an array of all the key values that belong in the section 
// 'section_name'. The size of the resulting array is stored in the location
// pointed to by 'values_size_ret'. Note that this function performs a 
// memory allocation.
SIRDEF const char **sir_section_key_values(SirIni ini, 
        const char *section_name, int *values_size_ret);

// Used to free the arrays given by sir_section_key_names() and 
// sir_section_key_values(). This is simply wrapper for the SIR_FREE()
// macro, but is required to give the macro the memory context.
SIRDEF void sir_free(SirIni ini, void *mem);

// Returns 1 if there is an error. SIR Functions will always clear the error
// on success.
SIRDEF char sir_has_error(SirIni ini);

// These macros can be used to search through all the keys in an INI.
#define sir_str(ini, key_name) sir_section_str(ini, 0, key_name)

#define sir_long(ini, key_name) sir_section_long(ini, 0, key_name)

#define sir_unsigned_long(ini, key_name) \
    sir_section_unsigned_long(ini, 0, key_name)

#define sir_double(ini, key_name) sir_section_double(ini, 0, key_name)

#define sir_bool(ini, key_name) sir_section_bool(ini, 0, key_name)

#define sir_csv(ini, key_name, csv_size_ret) \
    sir_section_csv(ini, 0, key_name, csv_size_ret)

#if !(defined(SIR_MALLOC) && defined(SIR_REALLOC) && defined(SIR_FREE))
#define SIR_MALLOC(ctx, size)        malloc(size)
#define SIR_FREE(ctx, mem)           free(mem)
#define SIR_REALLOC(ctx, mem, size)  realloc(mem, size)
#endif

//
// CONFIGURATION MACROS
// ====================
//
// You can define these macros before including this header to override these.
//
#ifndef SIR_ERROR_STRING_SIZE
#define SIR_ERROR_STRING_SIZE 512
#endif

#ifndef SIR_WARNING_STRING_SIZE 
#define SIR_WARNING_STRING_SIZE 512
#endif

#ifndef SIR_WARNINGS_SIZE_INCR
#define SIR_WARNINGS_SIZE_INCR 5
#endif

#ifndef SIR_COMMENT_CHAR
#define SIR_COMMENT_CHAR ';'
#endif

#ifndef SIR_COMMENT_CHAR_ALT
#define SIR_COMMENT_CHAR_ALT '#'
#endif

#ifndef SIR_KEY_ASSIGNMENT_CHAR
#define SIR_KEY_ASSIGNMENT_CHAR '='
#endif

#ifndef SIR_KEY_ASSIGNMENT_CHAR_ALT
#define SIR_KEY_ASSIGNMENT_CHAR_ALT ':'
#endif

#ifndef SIR_GLOBAL_SECTION_NAME
#define SIR_GLOBAL_SECTION_NAME "global"
#endif

// 'PRIVATE' FUNCTIONS
// ===================
static char sir__to_lowercase(char c);
static char sir__str_equal_case(const char *s1, 
        const char *s2, char case_insensitive);
static char sir__str_equal(const SirIni ini, const char *s1, const char *s2);
static char sir__is_comment_char(const SirIni ini, char c);
static char sir__is_assignment_char(const SirIni ini, char c);
static int sir__skip_whitespace(const char *str);
static char *sir__trim_whitespace(char *str);
static int sir__skip_to_char(const char *str, char c);
static int sir__parse_to_char(char *str, char c, char **parsed_str_ret);
static int sir__parse_to_assignment_char(const SirIni ini, char *str, 
        char **parsed_str_ret);
static void sir__add_to_char_counts(char c, int *line_number, 
        int *char_number);
static char sir__warnings_enabled(SirIni ini);
static char sir__errors_enabled(SirIni ini);
static void sir__add_warning(SirIni ini, int line_number, int char_number,
        const char *msg);
static void sir__set_error(SirIni ini, const char *format, const char *s1, 
        const char *s2);
static void sir__clear_error_str(SirIni ini);
static SirIni sir__create_ini(char disable_errors, 
        char disable_warnings, void *mem_ctx);
SIRDEF SirSection *sir__get_section(SirIni ini, const char *section_name);
static const char **sir__section_key_array(SirIni ini, 
        const char *section_name, int *size_ret, const char **key_array);


// 'PRIVATE' MACROS
// ================
#define SIR__SECTION_NAME_OPEN_CHAR  '['
#define SIR__SECTION_NAME_CLOSE_CHAR ']'
#define SIR__KEY_END_CHAR            '\n'
#define SIR__BOOL_TRUE_STRING        "true"
#define SIR__BOOL_FALSE_STRING       "false"
#define SIR__INI_NO_FILENAME_STRING  "ini"

#endif // SIMPLE_INI_READER_HEADER



#ifdef SIMPLE_INI_READER_IMPLEMENTATION

static char sir__to_lowercase(char c)
{
    if (c >= 'a' && c <= 'z') return c - ('a' - 'A');
    else                      return c;
}

static char sir__str_equal_case(const char *s1, 
        const char *s2, char case_insensitive)
{
    if      (!s1 && !s2) return 1;
    else if (!s1 || !s2) return 0;

    char c1, c2;

    while (*s1 && *s2)
    {
        c1 = *s1;
        c2 = *s2;

        if (case_insensitive)
        {
            c1 = sir__to_lowercase(c1);
            c2 = sir__to_lowercase(c2);
        }

        if (c1 != c2) return 0;

        ++s1;
        ++s2;
    }

    if   (*s1 == *s2) return 1;
    else              return 0;
}

static char sir__str_equal(const SirIni ini, const char *s1, const char *s2)
{
    return sir__str_equal_case(s1, s2, 
            ini->options & SIR_OPTION_DISABLE_CASE_SENSITIVITY);
}

static char sir__is_comment_char(const SirIni ini, char c)
{
    return (c == SIR_COMMENT_CHAR || 
            (ini && 
             (!(ini->options & SIR_OPTION_DISABLE_HASH_COMMENTS)) && 
             c == SIR_COMMENT_CHAR_ALT));
}

static char sir__is_assignment_char(const SirIni ini, char c)
{
    return (c == SIR_KEY_ASSIGNMENT_CHAR || 
            (ini && 
             (!(ini->options & SIR_OPTION_DISABLE_COLON_ASSIGNMENT)) && 
             c == SIR_KEY_ASSIGNMENT_CHAR_ALT));
}

static int sir__skip_whitespace(const char *str)
{
    if (!str) return 0;

    int n = 0;

    while (*str && *str <= ' ')
    {
        ++str;
        ++n;
    }

    return n;
}

static char *sir__trim_whitespace(char *str)
{
    if (!str) return 0;

    str += sir__skip_whitespace(str);

    size_t i = strlen(str) - 1;

    while (str[i] <= ' ') --i;

    str[i + 1] = '\0';

    return str;
}

static int sir__skip_to_char(const char *str, char c)
{
    if (!str) return 0;

    int n = 0;

    while (*str && *str != c)
    {
        ++str;
        ++n;
    }

    return n;
}

static int sir__parse_to_char(char *str, char c, char **parsed_str_ret)
{
    if (!str) return 0;

    char *start = str;

    int n = sir__skip_to_char(str, c);    

    str += n;

    if (parsed_str_ret) *parsed_str_ret = start;

    if (*str == '\0')
    {
        return -1;
    }
    else
    {
        *str = '\0';
        return n;
    }
}

static int sir__parse_to_assignment_char(const SirIni ini, char *str, 
        char **parsed_str_ret)
{
    if (!ini || !str) return 0;

    if (ini->options & SIR_OPTION_DISABLE_COLON_ASSIGNMENT)
    {
        return sir__parse_to_char(str, SIR_KEY_ASSIGNMENT_CHAR, 
                parsed_str_ret);
    }
    else
    {
        int a = sir__skip_to_char(str, SIR_KEY_ASSIGNMENT_CHAR);
        int b = sir__skip_to_char(str, SIR_KEY_ASSIGNMENT_CHAR_ALT);

        if (a < b)
            return sir__parse_to_char(str, SIR_KEY_ASSIGNMENT_CHAR, 
                    parsed_str_ret);
        else
            return sir__parse_to_char(str, SIR_KEY_ASSIGNMENT_CHAR_ALT, 
                    parsed_str_ret);
    }
}

static void sir__add_to_char_counts(char c, int *line_number, 
        int *char_number)
{
    if (c == '\n')
    {
        if (line_number) *line_number += 1;
        if (char_number) *char_number = 1;
    }
    else
    {
        if (char_number) *char_number += 1;
    }
}

static char sir__warnings_enabled(SirIni ini)
{
    return (!(ini->options & SIR_OPTION_DISABLE_WARNINGS));
}

static char sir__errors_enabled(SirIni ini)
{
    return (!(ini->options & SIR_OPTION_DISABLE_ERRORS));
}

static void sir__add_warning(SirIni ini, int line_number, int char_number,
        const char *msg)
{
    if (!ini || !msg || !sir__warnings_enabled(ini)) return;

    // An Int can't be more than 10 digits so these sprintfs should be safe
    char ln_str[11], cn_str[11];

    sprintf(ln_str, "%i", line_number);
    ln_str[10] = '\0';

    sprintf(cn_str, "%i", char_number);
    cn_str[10] = '\0';

    char *str = SIR_MALLOC(ini->mem_ctx, SIR_WARNING_STRING_SIZE);

    size_t n = 0;

    int i = 0;

    const char *read = ini->filename;
    char *write = str;
    char *warning = ": warning: ";
    char *colon = ":";

    while (*read && n < SIR_WARNING_STRING_SIZE - 1)
    {
        *write++ = *read++;
        ++n;

        if (!*read)
        {
            if (i == 0)
                read = colon;
            else if (i == 1)
                read = ln_str;
            else if (i == 2)
                read = colon;
            else if (i == 3)
                read = cn_str;
            else if (i == 4)
                read = warning;
            else if (i == 5)
                read = msg;

            ++i;
        }
    }

    *write = '\0';

    ini->warnings[ini->warnings_count] = str;

    ++ini->warnings_count;

    if (ini->warnings_count >= ini->warnings_size)
    {
        ini->warnings_size = ini->warnings_size + 
            SIR_WARNINGS_SIZE_INCR;

        ini->warnings = SIR_REALLOC(ini->mem_ctx, (void *)ini->warnings,
                sizeof(*ini->warnings) * ini->warnings_size);
    }
}

static void sir__set_error(SirIni ini, const char *format, const char *s1, 
        const char *s2)
{
    if (!ini || !format || !sir__errors_enabled(ini)) return;

    size_t size = ini->error_size;

    char *str = ini->error;

    size_t n = 0;
    int    i = 0;

    while (*format && n < size - 1)
    {
        if (*format == '%')
        {
            if ((i == 0 && s1) || (i == 1 && s2))
            {
                const char *s = (i == 0) ? s1 : s2;

                while (*s && n < size - 1)
                {
                    *str = *s;

                    ++n;
                    ++s;
                    ++str;
                }

                ++i;
            }
        }
        else 
        {
            *str = *format;
            ++n;
            ++str;
        }

        ++format;
    }

    *str = '\0';
}

static void sir__clear_error_str(SirIni ini)
{
    if (ini && sir__errors_enabled(ini))
        *ini->error = '\0';
}

SIRDEF char sir_has_error(SirIni ini)
{
    return (ini && sir__errors_enabled(ini) && *ini->error != '\0');
}

static SirIni sir__create_ini(char disable_errors, 
        char disable_warnings, void *mem_ctx)
{
    SirIni ini = SIR_MALLOC(mem_ctx, sizeof(*ini));

    if (!ini) return 0;

    memset(ini, 0, sizeof(*ini));

    ini->mem_ctx = mem_ctx;

    if (disable_errors)
    {
        ini->error_size = 0;
        ini->error = SIR_MALLOC(ini->mem_ctx, 1);
        *ini->error = '\0';
    }
    else
    {
        ini->error_size = SIR_ERROR_STRING_SIZE;
        ini->error = SIR_MALLOC(ini->mem_ctx, ini->error_size);
        ini->error_msg = SIR_MALLOC(ini->mem_ctx, ini->error_size);
    }

    if (disable_warnings)
    {
        ini->warnings_size = 0;
    }
    else
    {
        ini->warnings_size = SIR_WARNINGS_SIZE_INCR;
        ini->warnings = SIR_MALLOC(ini->mem_ctx, 
                sizeof(*ini->warnings) * ini->warnings_size);
    }

    return ini;
}

SIRDEF void sir_free_ini(SirIni ini)
{
    if (ini)
    {
        int i;

        for (i = 0; i < ini->section_count; ++i)
            if (ini->sections[i].ranges)
                SIR_FREE(ini->mem_ctx, ini->sections[i].ranges);

        for (i = 0; i < ini->warnings_count; ++i)
            if (ini->warnings[i])
                SIR_FREE(ini->mem_ctx, (void *)ini->warnings[i]);

        if (ini->data)          SIR_FREE(ini->mem_ctx, ini->data);
        if (ini->sections)      SIR_FREE(ini->mem_ctx, ini->sections);
        if (ini->section_names) SIR_FREE(ini->mem_ctx, 
                (void *)ini->section_names);
        if (ini->key_names)     SIR_FREE(ini->mem_ctx, (void *)ini->key_names);
        if (ini->key_values)    SIR_FREE(ini->mem_ctx, 
                (void *)ini->key_values);
        if (ini->filename)      SIR_FREE(ini->mem_ctx, (void *)ini->filename);
        if (ini->error)         SIR_FREE(ini->mem_ctx, ini->error);
        if (ini->error_msg)     SIR_FREE(ini->mem_ctx, ini->error_msg);
        if (ini->warnings)      SIR_FREE(ini->mem_ctx, (void *)ini->warnings);

        SIR_FREE(ini->mem_ctx, ini);
    }
}

SIRDEF SirIni sir_load_from_str(char *s, SirOptions options, 
        const char *name, void *mem_ctx)
{
    SirIni ini = sir__create_ini(options & SIR_OPTION_DISABLE_ERRORS,
            options & SIR_OPTION_DISABLE_WARNINGS, mem_ctx);

    if (!ini) return 0;

    if (name)
    {
        ini->filename = SIR_MALLOC(mem_ctx, strlen(name) + 1);
        strcpy((char *)ini->filename, name);
    }
    else
    {
        ini->filename = SIR_MALLOC(mem_ctx, 
                strlen(SIR__INI_NO_FILENAME_STRING) + 1);
        strcpy((char *)ini->filename, SIR__INI_NO_FILENAME_STRING);
    }

    ini->options = options;
    ini->data = s;

    // Remove Comments and Count Sections and Keys
    ini->section_count = 1;

    char *str = ini->data;

    while (*str)
    {
        if (sir__is_comment_char(ini, *str))
        {
            if (!(ini->options & SIR_OPTION_DISABLE_COMMENT_ANYWHERE) || 
                    (ini->options & SIR_OPTION_DISABLE_COMMENT_ANYWHERE 
                     && (str == ini->data || (str > ini->data && 
                             *(str - 1) == '\n'))))
            {
                while (*str && *str != '\n')
                {
                    *str = ' ';
                    ++str;
                }
            }
        }

        if (*str == SIR__SECTION_NAME_OPEN_CHAR) ++ini->section_count;
        else if (sir__is_assignment_char(ini, *str))  ++ini->key_count;

        ++str;
    }

    // Check for Warnings
    if (!(ini->options & SIR_OPTION_DISABLE_WARNINGS))
    {
        int line_number = 1;
        int char_number = 1;
        str = ini->data;
        while (*str)
        {
            // Skip Whitespace
            while (*str && *str <= ' ')
            {
                sir__add_to_char_counts(*str, &line_number, &char_number);
                ++str;
            }

            if (*str == SIR__SECTION_NAME_OPEN_CHAR)
            {
                while (*str && *str != SIR__SECTION_NAME_CLOSE_CHAR)
                {
                    if (*str == '\n')
                    {
                        sir__add_warning(ini, line_number, char_number, 
                                "Newline found in section name. Did you "
                                "forget to close the section name with ']'?");
                    }
                    else if (sir__is_assignment_char(ini, *str))
                    {
                        sir__add_warning(ini, line_number, char_number,
                                "'=' found in section name. Did you "
                                "forget to close the section name with ']'?");
                    }

                    sir__add_to_char_counts(*str, &line_number, &char_number);

                    ++str;
                }

                sir__add_to_char_counts(*str, &line_number, &char_number);
                ++str;
            }
            else
            {
                while (*str && !sir__is_assignment_char(ini, *str))
                {
                    if (*str == SIR__SECTION_NAME_OPEN_CHAR)
                    {
                        sir__add_warning(ini, line_number, char_number, 
                                "'[' found in key name");
                    }
                    else if (*str == SIR__SECTION_NAME_CLOSE_CHAR)
                    {
                        sir__add_warning(ini, line_number, char_number,
                                "']' found in key name");
                    }

                    sir__add_to_char_counts(*str, &line_number, &char_number);

                    ++str;
                }

                sir__add_to_char_counts(*str, &line_number, &char_number);
                ++str;

                while (*str && *str != SIR__KEY_END_CHAR)
                {
                    if (*str == SIR__SECTION_NAME_OPEN_CHAR)
                    {
                        sir__add_warning(ini, line_number, char_number, 
                                "'[' found in key value");
                    }
                    else if (*str == SIR__SECTION_NAME_CLOSE_CHAR)
                    {
                        sir__add_warning(ini, line_number, char_number,
                                "']' found in key value");
                    }

                    sir__add_to_char_counts(*str, &line_number, &char_number);

                    ++str;
                }

                sir__add_to_char_counts(*str, &line_number, &char_number);
                ++str;
            }
        }
    }

    // Parse
    str = ini->data;

    ini->sections = SIR_MALLOC(mem_ctx, 
            sizeof(*ini->sections) * ini->section_count);
    ini->section_names = SIR_MALLOC(mem_ctx, 
            sizeof(*ini->section_names) * ini->section_count);
    ini->key_names = SIR_MALLOC(mem_ctx, 
            sizeof(*ini->key_names) * ini->key_count);
    ini->key_values = SIR_MALLOC(mem_ctx, 
            sizeof(*ini->key_values) * ini->key_count);

    memset(ini->sections, 0, 
            sizeof(*ini->sections) * ini->section_count);
    memset((void *)ini->section_names, 0, 
            sizeof(*ini->section_names) * ini->section_count);
    memset((void *)ini->key_names, 0, 
            sizeof(*ini->key_names) * ini->key_count);
    memset((void *)ini->key_values, 0, 
            sizeof(*ini->key_values) * ini->key_count);

    for (int i = 0; i < ini->section_count; ++i)
    {
        ini->sections[i].ranges_count = 1;
        ini->sections[i].ranges = SIR_MALLOC(mem_ctx, 
                sizeof(*ini->sections[i].ranges) * 
                ini->sections[i].ranges_count);
    }

    ini->sections[0].ranges[0].start = 0;
    ini->section_names[0] = SIR_GLOBAL_SECTION_NAME;

    int prev_index  = 0;
    int section_index = 0;
    int key_index     = 0;

    while (*str)
    {
        str += sir__skip_whitespace(str);

        // Section
        if (*str == SIR__SECTION_NAME_OPEN_CHAR)
        {
            char *section_name;

            int range_index = ini->sections[prev_index].ranges_count - 1;

            ini->sections[prev_index].ranges[range_index].end = key_index;

            ++str;

            int n = sir__parse_to_char(str, SIR__SECTION_NAME_CLOSE_CHAR, 
                    &section_name);

            section_name = sir__trim_whitespace(section_name);

            // Check for Duplicates
            int duplicate = -1;
            {
                for (int i = 0; i < section_index + 1; ++i)
                {
                    if (sir__str_equal(ini, 
                                ini->section_names[i], section_name))
                    {
                        duplicate = i;
                        break;
                    }
                }
            }

            if (duplicate != -1)
            {
                if (duplicate != prev_index)
                {
                    ++ini->sections[duplicate].ranges_count;

                    ini->sections[duplicate].ranges = 
                        SIR_REALLOC(mem_ctx, 
                                ini->sections[duplicate].ranges, 
                                sizeof(*ini->sections[duplicate].ranges) * 
                                ini->sections[duplicate].ranges_count);

                    range_index = ini->sections[duplicate].ranges_count - 1;

                    ini->sections[duplicate].ranges[range_index].start = 
                        key_index;
                }

                prev_index = duplicate;
            }
            else
            {
                ++section_index;
                prev_index = section_index;

                ini->sections[section_index].ranges[0].start = key_index;
                ini->section_names[section_index] = section_name;
            }

            if (n == -1)
                break;
            else
                str += n + 1;
        }
        // Key
        else if (*str)
        {
            char *key_name;
            char *key_value;

            // Parse Name
            int n = sir__parse_to_assignment_char(ini, str, &key_name);

            key_name = sir__trim_whitespace(key_name);

            // Check for Duplicate Name (-1 means no duplicate)
            int duplicate = -1;
            {
                SirSection *section = &ini->sections[section_index];
                int end = 0;
                int start = 0;

                for (int i = 0; i < section->ranges_count; ++i)
                {
                    if (i < section->ranges_count - 1)
                        end = section->ranges[i].end;
                    else
                        end = key_index;

                    start = section->ranges[i].start;

                    for (int j = start; j < end; ++j)
                    {
                        if (sir__str_equal(ini, ini->key_names[j], key_name))
                        {
                            duplicate = j;
                            break;
                        }
                    }
                }
            }

            // Parse Value
            if (n != -1)
            {
                str += n + 1;

                char quoted = 0;

                if (!(ini->options & SIR_OPTION_DISABLE_QUOTES))
                {
                    char *quoted_str = str;
                    while (*quoted_str && *quoted_str != SIR__KEY_END_CHAR)
                    {
                        if (*quoted_str == '\"')
                        {
                            quoted = 1;
                            break;
                        }

                        ++quoted_str;
                    }
                }

                if (quoted)
                {
                    str += sir__skip_to_char(str, '\"') + 1;

                    str += sir__parse_to_char(str, '\"', &key_value) + 1;
                }
                else
                {
                    str += sir__parse_to_char(str, SIR__KEY_END_CHAR, 
                            &key_value) + 1;

                    key_value = sir__trim_whitespace(key_value);
                }
            }
            else 
            {
                key_value = "";
            }

            // Apply Value to Name
            if (!(*key_value == '\0' && 
                        ini->options & SIR_OPTION_IGNORE_EMPTY_VALUES))
            {
                if (duplicate != -1)
                {
                    if (ini->options & 
                            SIR_OPTION_OVERRIDE_DUPLICATE_KEYS)
                    {
                        ini->key_values[duplicate] = key_value;
                    }
                }
                else
                {

                    ini->key_names[key_index] = key_name;
                    ini->key_values[key_index] = key_value;

                    ++key_index;
                }
            }

            if (n == -1) break;
        }
    }

    ini->sections[prev_index].ranges[
        ini->sections[prev_index].ranges_count - 1].end = key_index;

    // Shrink if necessary
    if (section_index  + 1 < ini->section_count)
    {
        ini->section_count = section_index + 1;

        ini->sections = SIR_REALLOC(mem_ctx, (void *)ini->sections,
                sizeof(*ini->sections) * ini->section_count);
        ini->section_names = SIR_REALLOC(mem_ctx, 
                (void *)ini->section_names,
                sizeof(*ini->section_names) * ini->section_count);
    }

    if (key_index < ini->key_count)
    {
        ini->key_count = key_index;

        ini->key_names = SIR_REALLOC(mem_ctx, (void *)ini->key_names, 
                sizeof(*ini->key_names) * ini->key_count);
        ini->key_values = SIR_REALLOC(mem_ctx, (void *)ini->key_values, 
                sizeof(*ini->key_values) * ini->key_count);
    }

    sir__clear_error_str(ini);

    return ini;

}

SIRDEF SirIni sir_load_from_file(const char *filename, 
        SirOptions options, void *mem_ctx)
{
    // create a temporary ini file in case of errors
    SirIni ini = sir__create_ini(0, 0, mem_ctx);

    ini->filename = SIR_MALLOC(mem_ctx, strlen(filename) + 1);
    strcpy((char *)ini->filename, filename);

    // Load Entire File
    FILE *file = fopen(filename, "r");

    if (!file) 
    {
        sir__set_error(ini, strerror(errno), 0, 0);
        return ini;
    }

    if (fseek(file, 0, SEEK_END) == -1)
    {
        fclose(file);
        sir__set_error(ini, strerror(errno), 0, 0);
        return ini;
    }

    long size = ftell(file);
    if (size == -1L)
    {
        fclose(file);
        sir__set_error(ini, strerror(errno), 0, 0);
        return ini;
    }

    rewind(file);

    char *data = SIR_MALLOC(mem_ctx, size + 1);
    if (!data)
    {
        fclose(file);
        sir__set_error(ini, "SIR_MALLOC failed", 0, 0);
        return ini;
    }

    size_t bytes_read = fread(data, 1, size, file);
    if (ferror(file))
    {
        SIR_FREE(mem_ctx, data);
        fclose(file);
        sir__set_error(ini, strerror(errno), 0, 0);
        return ini;
    }

    if (bytes_read < (size_t)size)
    {
        size = bytes_read;
        data = SIR_REALLOC(mem_ctx, data, size + 1);

        if (!data)
        {
            fclose(file);
            sir__set_error(ini, strerror(errno), 0, 0);
            return ini;
        }
    }

    fclose(file);

    data[size] = '\0';

    sir_free_ini(ini);

    return sir_load_from_str(data, options, filename, mem_ctx);
}

SIRDEF SirSection *sir__get_section(SirIni ini, const char *section_name)
{
    if (!ini) return 0;

    if (!section_name)
    {
        sir__set_error(ini, 
                "the parameter 'section_name' is not optional", 0, 0);
        return 0;
    }

    for (int i = 0; i < ini->section_count; ++i)
    {
        if (ini->section_names[i] && 
                sir__str_equal(ini, ini->section_names[i], section_name))
        {
            sir__clear_error_str(ini);
            return &ini->sections[i];
        }
    }

    sir__set_error(ini, "section '%' not found", section_name, 0);
    return 0;
}

static const char **sir__section_key_array(SirIni ini, 
        const char *section_name, int *size_ret, const char **key_array)
{
    if (!ini) return 0;

    if (!section_name)
    {
        sir__set_error(ini, 
                "the parameter 'section_name' is not optional", 0, 0);
        return 0;
    }

    SirSection *section = sir__get_section(ini, section_name);

    if (sir_has_error(ini))
        return 0;

    int i;
    int key_count = 0;
    for (i = 0; i < section->ranges_count; ++i)
    {
        key_count += section->ranges[i].end - section->ranges[i].start;
    }

    const char **array = SIR_MALLOC(ini->mem_ctx, 
            sizeof(*array) * key_count);

    int index = 0;

    for (i = 0; i < section->ranges_count; ++i)
    {
        int start = section->ranges[i].start;
        int end   = section->ranges[i].end;

        for (int j = start; j < end; ++j)
        {
            array[index] = key_array[j];
            ++index;
        }
    }

    if (size_ret) *size_ret = key_count;

    sir__clear_error_str(ini);

    return array;

}

SIRDEF const char **sir_section_key_names(SirIni ini, 
        const char *section_name, int *values_size_ret)
{
    return sir__section_key_array(ini, section_name, values_size_ret, 
            ini->key_names);
}

SIRDEF const char **sir_section_key_values(SirIni ini, 
        const char *section_name, int *values_size_ret)
{
    return sir__section_key_array(ini, section_name, values_size_ret, 
            ini->key_values);
}

SIRDEF void sir_free(SirIni ini, void *mem)
{
    SIR_FREE(ini->mem_ctx, (void *)mem);
}

SIRDEF const char *sir_section_str(SirIni ini, const char *section_name, 
        const char *key_name)
{
    if (!ini) return 0;

    if (!key_name) 
    {
        sir__set_error(ini, "the parameter 'key_name' is not optional", 
                0, 0);
        return 0;
    }

    SirSection *section = 0;

    if (section_name)
    {
        section = sir__get_section(ini, section_name);

        if (sir_has_error(ini)) 
            return 0;
    }

    if (section)
    {
        int start;
        int end;

        for (int i = 0; i < section->ranges_count; ++i)
        {
            start = section->ranges[i].start;
            end   = section->ranges[i].end;

            for (int j = start; j < end; ++j)
            {
                if (ini->key_names[j] && sir__str_equal(ini, 
                            ini->key_names[j], key_name))
                {
                    sir__clear_error_str(ini);
                    return ini->key_values[j];
                }
            }
        }

        sir__set_error(ini, "key '%' not found in section '%'", 
                key_name, section_name);

        return 0;
    }
    else
    {
        const char *key_value = 0;

        for (int i = 0; i < ini->key_count; ++i)
        {
            if (ini->key_names[i] && 
                    sir__str_equal(ini, ini->key_names[i], key_name))
            {
                if (ini->options & SIR_OPTION_OVERRIDE_DUPLICATE_KEYS)
                {
                    key_value = ini->key_values[i];
                }
                else
                {
                    sir__clear_error_str(ini);
                    return ini->key_values[i];
                }
            }
        }

        if (key_value)
        {
            sir__clear_error_str(ini);
            return key_value;
        }

        sir__set_error(ini, "key '%' not found", key_name, 0);
        return 0;
    }
}

SIRDEF long sir_section_long(SirIni ini, const char *section_name, 
        const char *key_name)
{
    const char *str = sir_section_str(ini, section_name, key_name);

    if (sir_has_error(ini) || !str) return 0;

    char *endptr;

    errno = 0;
    long l = strtol(str, &endptr, 0);

    if (errno == ERANGE)
    {
        if (l == LONG_MAX)
        {
            sir__set_error(ini, 
                    "'%' is more than the maximum value of a long interger.",
                    str, 0);
        }
        else if (l == LONG_MIN)
        {
            sir__set_error(ini, 
                    "'%' is less than the minimum value of a long interger.",
                    str, 0);
        }
        else
        {
            sir__set_error(ini, 
                    "'%' is outside the range of values of a long integer.", 
                    str, 0);
        }

        return 0;
    }
    else if (endptr == str)
    {
        sir__set_error(ini, 
                "'%' could not be converted to a long integer.", str, 0);
        return 0;
    }
    else
    {
        return l;
    }
}

SIRDEF long sir_section_unsigned_long(SirIni ini, const char *section_name, 
        const char *key_name)
{
    const char *str = sir_section_str(ini, section_name, key_name);

    if (sir_has_error(ini) || !str) return 0;

    char *endptr;

    errno = 0;
    unsigned long ul = strtoul(str, &endptr, 0);

    if (errno == ERANGE)
    {
        sir__set_error(ini, 
                "'%' is outside the range of values of an unsigned long"
                " integer.", str, 0);

        return 0;
    }
    else if (endptr == str)
    {
        sir__set_error(ini, 
                "'%' could not be converted to an unsigned long integer.", 
                str, 0);
        return 0;
    }
    else
    {
        return ul;
    }
}

SIRDEF double sir_section_double(SirIni ini, const char *section_name, 
        const char *key_name)
{
    const char *str = sir_section_str(ini, section_name, key_name);

    if (sir_has_error(ini) || !str) return 0;

    char *endptr;

    errno = 0;
    double d = strtod(str, &endptr);

    if (errno == ERANGE)
    {
        if (d == HUGE_VAL)
        {
            sir__set_error(ini, 
                    "'%' is more than the maximum value of a double.", 
                    str, 0);
        }
        else if (d == -HUGE_VAL)
        {
            sir__set_error(ini, 
                    "'%' is less than the minimum value of a double.", 
                    str, 0);
        }
        else
        {
            sir__set_error(ini, 
                    "'%' is outside the range of values of a double.", 
                    str, 0);
        }

        return 0;
    }
    else if (endptr == str)
    {
        sir__set_error(ini, 
                "'%' could not be converted to a double.", str, 0);
        return 0;
    }
    else
    {
        return d;
    }
}

SIRDEF char sir_section_bool(SirIni ini, const char *section_name, 
        const char *key_name)
{
    long l = sir_section_long(ini, section_name, key_name);

    if (!sir_has_error(ini))
    {
        if   (l) return 1;
        else     return 0;
    }

    const char *str = sir_section_str(ini, section_name, key_name);

    if (sir_has_error(ini) || !str) return -1;

    str += sir__skip_whitespace(str);

    char s[6];
    size_t len;

    len = strlen(SIR__BOOL_TRUE_STRING);
    strncpy(s, str, len);
    s[len] = '\0';

    if (sir__str_equal_case(s, SIR__BOOL_TRUE_STRING, 1))  return 1;

    len = strlen(SIR__BOOL_FALSE_STRING);
    strncpy(s, str, len);
    s[len] = '\0';

    if (sir__str_equal_case(s, SIR__BOOL_FALSE_STRING, 1)) return 0;

    sir__set_error(ini, "could not parse '%' as a bool", str, 0);
    return -1;
}

SIRDEF const char **sir_section_csv(const SirIni ini, 
        const char *section_name, const char *key_name, int *csv_size_ret)
{
    const char *str = sir_section_str(ini, section_name, key_name);

    if (sir_has_error(ini) || !str)
        return 0;

    str += sir__skip_whitespace(str);

    char *s = SIR_MALLOC(ini->mem_ctx, strlen(str) + 1);
    strcpy(s, str);

    s = sir__trim_whitespace(s);

    int csv_size = 1;

    int i;
    for (i = 0; s[i]; ++i) 
        if (s[i] == ',')
            ++csv_size;

    const char **csv = SIR_MALLOC(ini->mem_ctx, sizeof(*csv) * csv_size);

    char *start = s;
    char *end   = s;

    i = 0;
    while (i < csv_size)
    {
        start += sir__skip_whitespace(start);

        csv[i] = (const char *)start;

        while (*end && *end != ',') ++end;

        *end = '\0';

        ++end;
        start = end;

        ++i;
    }

    if (csv_size_ret) *csv_size_ret = csv_size;

    return csv;
}

SIRDEF void sir_free_csv(SirIni ini, const char **csv)
{
    if (ini && csv)  
    {
        if (*csv) 
            SIR_FREE(ini->mem_ctx, (void *)*csv);

        SIR_FREE(ini->mem_ctx, (void *)csv);
    }
}

#endif // SIMPLE_INI_READER_IMPLEMENTATION
