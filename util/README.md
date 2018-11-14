# Simple INI Reader Command-Line Utility
This utility provides a basic shell interface to the Simple INI Reader library.
It is written in the style of a Unix command (can be used with pipes and 
redirection, features a minimal set of options, etc.), but only uses C Standard
Library functions, so it should be portable.

## Usage
Input is given either in the form of a filename argument or as Standard Input
if no filename is specified. The program uses the first argument it finds that
doesn't look like an option as the filename. Any other filenames specified are
ignored.

The `-k key_name` option finds the first key named `key_name` and prints it
to Standard Output. If the `-k` option is not given, all values are printed.

The `-s section_name` option tells the program that you only want to look at
keys in `section_name`.

The `--list-sections` option prints the name of each section. Note that no name
is printed for the INI's global section.

The `--list-keys` option prints each key name (as opposed to the key values
printed by `-k`). Note that this may be combined with the `-s` option to get
the key names from a specific section.

## Compilation
Simply compile `sir_util.c` with a C compiler. For example:

```
gcc -o sir sir_util.c
```
