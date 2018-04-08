# Lineeditor
A simple interactive lineeditor program.

* To build on Windows
  - `build.bat clang` for 64-bit clang, add "x86" for 32-bit.
  - `build.bat gcc` for 64-bit gcc, add "x86" for 32-bit.
  - `build.bat` or `build.bat msvc` for msvc, use `build.bat msvc x86` for 32-bit.
* To build on Linux
  - run `./build_linux` (uses gcc)

## Current Features
* New File
* Open File
* Save File
* Insert line(s) after line number
* Insert line(s) before line number
* Append to line
* Prepend to line
* Replace line
* Replace first occurance of string in line
* Delete line
* Preview whole file
* Cancel operation (using Ctrl-X+Enter)

## TODO
* Cancel Prompts for getting information for a command (for example, when it prompt you to enter a line number when one wasn't provided when the command was typed)
* Simple Undo - Undo the last operation (cannot undo an undo; only undo the latest operation, no more; After an undo is done, must wait for another operation before can undo again)
* For C and C++ files (.c, .cpp, and .h), parse the file to get function and variable definitions
  - Ability to list declarations - basically like an outline of the file
  - Ability to show line a declaration is on
* Support outline for markdown files
* Simple syntax highlighting for C, C++, Bash, and Batch
* Repeat the last operation
* Move line up or down 1
* Better data structure for the lines that will allow easily moving lines around, deleting then, and inserting them
