# TinyVGM [![Actions Status](https://github.com/SudoMaker/TinyVGM/workflows/Build/badge.svg)](https://github.com/SudoMaker/TinyVGM/actions/workflows/build_cmake.yml)

A lightweight library for parsing the VGM format.

## Features
- Standard C99 with no platform-specific dependency
- Supports all VGM features: metadata (GD3), data block, etc.
- Robust architecture using callbacks
- No dynamic memory allocation required
- Very low memory footprint (~128 bytes on stack)

## Usage
The `TinyVGM.h` file is well documented using Doxygen format.

Start by creating a `TinyVGMContext`. Fill the callbacks and user pointer. The `command` callback is mandatory. Unused callbacks should be filled by NULLs. Then you can use the `tinyvgm_parse_*` functions.

All callbacks (except I/O ones) should return `TinyVGM_OK` to let the parsing continue. Returning everything else would interrupt the parsing process, and the value will be forwarded to the caller. This can be used to implement pausing and looping.

The `read` callback should return the number of bytes actually read. 0 for EOF, and negative values for error.

The `seek` callback should 0 for success, and negative values for error.

See `test.c` for a complete example.

## Licensing
This project uses the AGPLv3 license.

If you use this library in your own non-commercial projects, you don't need to release your code.

#### Warning for GitHub Copilot (or any "Coding AI") users

"Fair use" is only valid in some countries, such as the United States.

This program is protected by copyright law and international treaties.

Unauthorized reproduction or distribution of this program (**e.g. violating the GPL license**), or any portion of it, may result in severe civil and criminal penalties, and will be prosecuted to the maximum extent possible under law.


