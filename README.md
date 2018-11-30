~~NOTE: Currently, the latest commits are bugged, so please use the files from [a7c4f](https://github.com/krixano/Edim/tree/a7c4f61b4c7cf4642bb9c232bd5f8b8b7b6e685f) until these bugs are fixed.~~ Many bugs have been fixed recently. There are still a few more to work on.

**NOTE2: EdimCoder has been moved to Pijul's The Nest (https://nest.pijul.com) and now uses the Pijul VCS (https://pijul.org). You can find the Pijul repository here: https://nest.pijul.com/krixano/EdimCoder**

# Edim Line Editor
A simple interactive line editor program inspired by vim, ed, 4coder, and modern editors.
This readme was written using Edim.

## Building and Running

### Build Debug Executable
* To build on Windows
  - `build.bat clang` for 64-bit clang, add "x86" for 32-bit.
  - `build.bat gcc` for 64-bit gcc, add "x86" for 32-bit.
  - `build.bat` or `build.bat msvc` for msvc, use `build.bat msvc x86` for 32-bit.
* To build on Linux
  - run `./build_linux` (uses gcc)

To run the debug executable, on Windows run `build run`. On Linux, the executable is in the `build/debug` directory.

### Build Release Executable
* To build on Windows
  - `build.bat msvc x64 release` for msvc 64-bit, change "x64" to "x86" for 32-bit

To run the release executable, on Windows run `build run release`.

## Current Features
* New File
* Open File
* Save File
* Insert line(s) before/after line number
* Prepend/Append to line
* Replace line
* Replace first occurance of string in line
* Delete line
  - Will also show the line that was moved up into the deleted line's place
* Cancel operation (using Ctrl-X+Enter)
* Shows previous line of current line being operated on to give context
* Ability to preview whole file in a similar fashion to more/less
  - including starting from a given line
* Ability to preview an individual line (along with the line before and after to give context)
* Ability to preview a range of lines
* Ability to show file information
  - Filename
  - Number of lines
* Show the first line that was moved down due to appending before/after a line.
* Move a line up or down one
* Colored Output
* Find first occurance of string in file (and print the line out)
* Find first occurance of string in a given line
* Show outline of markdown files (shown when file is opened and with fileinfo command, '#')
* Show outline of C files (shows function implementations)
* When opening file, if it doesn't exist, go straight to the editor to create the file.
* Ability to open multiple buffers (files), switch between them, and close them.
* Set current line number to specified line ('j (line#)') or to last line in file ('j$').
* When a command accepts a line number as first argument and one wasn't passed in, use current line instead.
* Bookmarks **NEW (11/20/2018)**
  - Creating a bookmarks
  - Listing all bookmarks
  - Using bookmark in place of line number for all commands

## TODO
* Cancel Prompts for getting information for a command (for example, when it prompt you to enter a line number when one wasn't provided when the command was typed)
* Simple Undo - Undo the last operation (cannot undo an undo; only undo the latest operation, no more; After an undo is done, must wait for another operation before can undo again)
* For C and C++ files (.c, .cpp, and .h), parse the file to get function and variable declarations
  - Ability to list declarations - basically like an outline of the file
  - Ability to show line a declaration is on
  - Ability to show all lines of a function
* Simple syntax highlighting for C, C++, Bash, and Batch
* Repeat the last operation
* Better data structure for the lines that will allow easily moving lines around, deleting them, and inserting them
* Add text before/after string in line
* Find all occurances
* Replace all in line
* Replace first occurance in file
* Replace all in file
* Ability to change setting on how many lines to show before the line you are currently modifying (to give context)
* Tab completion for opening/saving files
* UTF-8 Support
* Duplicating a line
* Copying a line into clipboard
* Cutting a line
* Move a line to before/after a given line number
* Better prompt command parsing
* Insert before/after character in line
* Bookmarks
  - Switch to hashmaps
  - Deleting bookmarks
  - Ability to save them in separate (projects/settings) file
* Ability to split a line into two lines

## Ideas
* Ability to use relative line numbers based on current line number

## Commands
* 'info' - Gives back information on the file, including number of lines, filename, number of characters, filetype, etc.
* 'j (line#)' - Set's current line to line number (no output). Use 'j$' to set last line as current line.
* 'a (line#)' - Insert after the line number
* 'i (line#)' - Insert before the line number
* 'A (line#)' - Appends to a line
* 'I (line#)' - Prepends to a line
* 'r (line#)' - Replace a line with a new line
* 'R (line#) (string)' - Replace the first occurance of the string in the line
* 'x (line#)' - Deletes a line
* 'm (line#)' - Move the line up by one
* 'M (line#)' - Move the line down by one
* 'f (string)' - Finds the first occurance of the string in the file and prints the line it's on out
* 'F (line#) (string)' - Find the first occurance of the string in the line and print the line out showing you where the occurance is
* 'u' - Undo the last operation, cannot undo an undo, cannot undo past 1 operation - *unimplemented*
* 'c' - Continue from last line in file
* 'p' - Preview whole file
* 'P (line#:start):(line#:end)' - Preview a line or set of lines, including the line before and after
* 'b' - List all currently open buffers
* 'b (buffer#)' - Switch current buffer to buffer #
* 'bn' - Switch current buffer to next buffer. Will wrap around when hits end.
* 'bp' - Switch current buffer to previous buffer. Will wrap around when hits beginning.
* d(line#:start):(line#:end) (string)' - Create bookmark with line range start:end and name string
* 'w (string)' - Print out line range of bookmark with name string
* 'g' - List out all bookmarks
* 'o' - Open file in new buffer
* 'n' - Create new file in new buffer
* 's' - Save current buffer
* 'e / E' - Exit current buffer / Exit current buffer (without save)
* 'q / Q' - Quit, closing all buffers / Quit, closing all buffers (without save)

Any command that accepts a line number or line range - denoted by `(line#:start):(line#:end)` - can also accept a bookmark. Bookmarks are prefixed with `#`. Example: `P #test`.
