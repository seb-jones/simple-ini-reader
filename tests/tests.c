/*
 * MAIN
 */

#define SIR_STATIC
#define SIMPLE_INI_READER_IMPLEMENTATION
#include "../simple_ini_reader.h"

#include <stdarg.h>

#ifdef _WIN32

#include <windows.h>

long long time_in_usecs()
{
    LARGE_INTEGER freq, ctr;

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&ctr);

    ctr.QuadPart *= 1000000;
    ctr.QuadPart /= freq.QuadPart;

    return ctr.QuadPart;
}

void print(const char *format, ...)
{
    static char s[2048];
    va_list vl;

    va_start(vl, format);
    vsnprintf(s, 2048, format, vl);
    va_end(vl);

    OutputDebugString(s);
}

#endif

#ifdef __unix__

#include <time.h>

long long time_in_usecs()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

    return (ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
}

void print(const char *format, ...)
{
    va_list vl;

    va_start(vl, format);
    vprintf(format, vl);
    va_end(vl);
}

#endif

int main(int argc, char **argv)
{
    long long start_time, end_time;

    start_time = time_in_usecs();

    SirIni ini;

    // TEST 0 - Fail to load
    {
        ini = sir_load_from_file("this_file_doesnt_exist", 0, 0);

        if (!sir_has_error(ini)) print("TEST 0 FAILED\n");
    }

    // TEST 1 - Basic Functions
    {
        // These should succeed
        ini = sir_load_from_file("test1.ini", 0, 0);

        sir_section_str(ini, SIR_GLOBAL_SECTION_NAME, "key1");
        if (sir_has_error(ini)) print("TEST 1 FAILED: %s\n", ini->error);

        sir_section_str(ini, "section 1", "key2");
        if (sir_has_error(ini)) print("TEST 1 FAILED: %s\n", ini->error);

        sir_section_str(ini, "section 2", "key3");
        if (sir_has_error(ini)) print("TEST 1 FAILED: %s\n", ini->error);

        sir_str(ini, "key1");
        if (sir_has_error(ini)) print("TEST 1 FAILED: %s\n", ini->error);

        sir_str(ini, "key2");
        if (sir_has_error(ini)) print("TEST 1 FAILED: %s\n", ini->error);

        sir_str(ini, "key3");
        if (sir_has_error(ini)) print("TEST 1 FAILED: %s\n", ini->error);

        // These should fail
        sir_section_str(ini, SIR_GLOBAL_SECTION_NAME, "key2");
        if (!sir_has_error(ini)) print("TEST 1 FAILED\n");

        sir_section_str(ini, "section 1", "key1");
        if (!sir_has_error(ini)) print("TEST 1 FAILED\n");

        sir_section_str(ini, "this_section_wont_be_found", "key1");
        if (!sir_has_error(ini)) print("TEST 1 FAILED\n");

        sir_str(ini, "this_key_wont_be_found");
        if (!sir_has_error(ini)) print("TEST 1 FAILED\n");

        const char *str = sir_section_str(0, SIR_GLOBAL_SECTION_NAME, "key1");
        if (str) print("TEST 1 FAILED\n");

        str = sir_str(0, "key1");
        if (str) print("TEST 1 FAILED\n");

        sir_free_ini(ini);
    }

    // TEST 2 - Parse to Type
    {
        // These should succeed
        ini = sir_load_from_file("test2.ini", 0, 0);

        long l = sir_long(ini, "long");
        if (sir_has_error(ini)) print("TEST 2 FAILED: %s", ini->error);
        if (l != 70000000) print("TEST 2 FAILED\n");

        unsigned long ul = sir_unsigned_long(ini, "ulong");
        if (sir_has_error(ini)) print("TEST 2 FAILED: %s", ini->error);
        if (ul != 2100000) print("TEST 2 FAILED\n");

        double d = sir_double(ini, "double");
        if (sir_has_error(ini)) print("TEST 2 FAILED: %s", ini->error);
        if (d != 3.14) print("TEST 2 FAILED\n");

        char b = sir_bool(ini, "bool1");
        if (sir_has_error(ini)) print("TEST 2 FAILED: %s", ini->error);
        if (!b) print("TEST 2 FAILED\n");

        b = sir_bool(ini, "bool2");
        if (sir_has_error(ini)) print("TEST 2 FAILED: %s", ini->error);
        if (b) print("TEST 2 FAILED\n");

        b = sir_bool(ini, "bool3");
        if (sir_has_error(ini)) print("TEST 2 FAILED: %s", ini->error);
        if (!b) print("TEST 2 FAILED\n");

        b = sir_bool(ini, "bool4");
        if (sir_has_error(ini)) print("TEST 2 FAILED: %s", ini->error);
        if (b) print("TEST 2 FAILED\n");

        b = sir_bool(ini, "bool5");
        if (sir_has_error(ini)) print("TEST 2 FAILED: %s", ini->error);
        if (!b) print("TEST 2 FAILED\n");

        // These should fail
        sir_long(ini, "long_too_big");
        if (!sir_has_error(ini)) print("TEST 2 FAILED\n");

        sir_long(ini, "long_too_small");
        if (!sir_has_error(ini)) print("TEST 2 FAILED\n");

        sir_long(ini, "long_no_digits");
        if (!sir_has_error(ini)) print("TEST 2 FAILED\n");

        sir_long(ini, "long_blank");
        if (!sir_has_error(ini)) print("TEST 2 FAILED\n");

        sir_double(ini, "double_blank");
        if (!sir_has_error(ini)) print("TEST 2 FAILED\n");

        sir_long(ini, "double_no_digits");
        if (!sir_has_error(ini)) print("TEST 2 FAILED\n");

        sir_long(ini, "bool_blank");
        if (!sir_has_error(ini)) print("TEST 2 FAILED\n");

        sir_long(ini, "bool_not_parsable");
        if (!sir_has_error(ini)) print("TEST 2 FAILED\n");

        sir_free_ini(ini);
    }

    // TEST 3 - Functions that return malloced arrays
    {
        ini = sir_load_from_file("test3.ini", 0, 0);

        int size;
        const char **csv = sir_csv(ini, "csv", &size);
        if (sir_has_error(ini))
            print("TEST 3 FAILED: %s\n", ini->error);
        else if (size != 4)
            print("TEST 3 FAILED\n");
        else
            sir_free_csv(ini, csv);

        const char **names = sir_section_key_names(ini, "global", &size);
        if (sir_has_error(ini))
            print("TEST 3 FAILED: %s\n", ini->error);
        else if (size != 3)
            print("TEST 3 FAILED\n");
        else
            sir_free(ini, (void *)names);

        const char **values = 
            sir_section_key_values(ini, "another_section", &size);
        if (sir_has_error(ini))
            print("TEST 3 FAILED: %s\n", ini->error);
        else if (size != 1)
            print("TEST 3 FAILED\n");
        else
            sir_free(ini, (void *)values);
    }

    // TEST 4 - Options
    {
        const char *str = 0;

        ini = sir_load_from_file("test4.ini", SIR_OPTION_IGNORE_EMPTY_VALUES, 0);
        if (sir_has_error(ini)) 
        {
            print("TEST 4 FAILED: %s\n", ini->error);
        }
        else
        {
            str = sir_str(ini, "key0");
            if (str) print("TEST 4 FAILED\n");

            sir_free_ini(ini);
        }

        ini = sir_load_from_file("test4.ini", SIR_OPTION_OVERRIDE_DUPLICATE_KEYS, 0);
        if (sir_has_error(ini)) 
        {
            print("TEST 4 FAILED: %s\n", ini->error);
        }
        else
        {
            str = sir_str(ini, "key1");
            if (strcmp(str, "baz")) print("TEST 4 FAILED\n");

            str = sir_section_str(ini, "global", "key1");
            if (strcmp(str, "bar")) print("TEST 4 FAILED\n");

            sir_free_ini(ini);
        }

        ini = sir_load_from_file("test4.ini", SIR_OPTION_DISABLE_QUOTES, 0);
        if (sir_has_error(ini)) 
        {
            print("TEST 4 FAILED: %s\n", ini->error);
        }
        else
        {
            str = sir_str(ini, "key2");
            if (strcmp(str, "\"hello\"")) print("TEST 4 FAILED\n");

            sir_free_ini(ini);
        }

        ini = sir_load_from_file("test4.ini", SIR_OPTION_DISABLE_HASH_COMMENTS, 0);
        if (sir_has_error(ini)) 
        {
            print("TEST 4 FAILED: %s\n", ini->error);
        }
        else
        {
            str = sir_str(ini, 
                    "#thisisacomment\n\n[another_section]\n\nkey4");
            if (!str) 
                print("TEST 4 FAILED\n");

            sir_free_ini(ini);
        }

        ini = sir_load_from_file("test4.ini", SIR_OPTION_DISABLE_COLON_ASSIGNMENT, 0);
        if (sir_has_error(ini)) 
        {
            print("TEST 4 FAILED: %s\n", ini->error);
        }
        else
        {
            str = sir_str(ini, "key4:colon\n\nkey5");
            if (!str)
                print("TEST 4 FAILED\n");

            sir_free_ini(ini);
        }

        ini = sir_load_from_file("test4.ini", SIR_OPTION_DISABLE_COMMENT_ANYWHERE, 0);
        if (sir_has_error(ini))
        {
            print("TEST 4 FAILED: %s\n", ini->error);
        }
        else
        {
            str = sir_str(ini, "key5");
            if (strcmp(str, "olleh#commentanywhere"))
                print("TEST 4 FAILED\n");

            sir_free_ini(ini);
        }

        ini = sir_load_from_file("test4.ini", SIR_OPTION_DISABLE_CASE_SENSITIVITY, 0);
        if (sir_has_error(ini))
        {
            print("TEST 4 FAILED: %s\n", ini->error);
        }
        else
        {
            str = sir_str(ini, "key");
            if (strcmp(str, "hello"))
                print("TEST 4 FAILED\n");

            sir_free_ini(ini);
        }
    }

    // TEST 5 - Test Warnings and Errors
    {
        ini = sir_load_from_file("test5.ini", 0, 0);

        for (int i = 0; i < ini->warnings_count; ++i)
            print("%s\n", ini->warnings[i]);

        sir_free_ini(ini);

        // Test Disabling Warnings
        ini = sir_load_from_file("test5.ini", SIR_OPTION_DISABLE_WARNINGS, 0);
        if (ini->warnings_count != 0) print("TEST 5 FAILED\n");

        sir_free_ini(ini);

        // Test Disabling Errors
        ini = sir_load_from_file("test5.ini", SIR_OPTION_DISABLE_ERRORS, 0);

        sir_str(ini, "this_wont_be_found");

        if (sir_has_error(ini)) print("TEST 5 FAILED");
    }

    end_time = time_in_usecs();

    print("\nTests 1-5: %f Seconds\n",
            ((double)end_time - (double)start_time) / 1000000.0);

    // Large Files
    {
        start_time = time_in_usecs();
        ini = sir_load_from_file("test6.ini", 0, 0);
        end_time = time_in_usecs();
        print("\nTests 6: %f Seconds\n",
            ((double)end_time - (double)start_time) / 1000000.0);
        sir_free_ini(ini);

        start_time = time_in_usecs();
        ini = sir_load_from_file("test7.ini", 0, 0);
        end_time = time_in_usecs();
        print("\nTests 7: %f Seconds\n",
            ((double)end_time - (double)start_time) / 1000000.0);
        sir_free_ini(ini);

        start_time = time_in_usecs();
        ini = sir_load_from_file("test8.ini", 0, 0);
        end_time = time_in_usecs();
        print("\nTests 8: %f Seconds\n",
            ((double)end_time - (double)start_time) / 1000000.0);
        sir_free_ini(ini);
    }

    return 0;
}
