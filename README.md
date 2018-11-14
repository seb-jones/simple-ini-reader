# Simple INI Reader
A fast, simple, public domain INI Reader written in C. This repo includes a
single-file library, and a Unix-style command-line utility that acts as a 
complete program example and can also be used to read INI files using shell
scripts.

## Why Use This?
* Public domain, so no attribution is required.
* Written using only the C Standard Library, so it should be portable (tested on Windows and Ubuntu).
* Fast and has a small memory footprint, especially if custom allocators are used and warnings and errors are disabled.
* It provides a very simple, user-friendly interface.

## Basic Usage

### Library

Simply drop `simple_ini_reader.h` into your source folder and add the following to one of your C/C++ files:
```
#define SIMPLE_INI_READER_IMPLEMENTATION
#include "simple_ini_reader.h"
```
`simple_ini_reader.h` has basic documentation commented at the top of the file. For more detailed examples check the `samples` folder.

### Utility

The source code for the command-line utility is located in `utils`. See the README file in that folder for more infomation.

## License
This software is dual-licensed to the public domain and under the following
license: you are granted a perpetual, irrevocable license to copy, modify,
publish, and distribute this file as you see fit.

## Contact
You can send feedback to [contact@sebj.co.uk](mailto:contact@sebj.co.uk)
