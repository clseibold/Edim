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
* Show outline of markdown files (shown when file is opened and with fileinfo command, '#')

## TODO
* Cancel Prompts for getting information for a command (for example, when it prompt you to enter a line number when one wasn't provided when the command was typed)
* Simple Undo - Undo the last operation (cannot undo an undo; only undo the latest operation, no more; After an undo is done, must wait for another operation before can undo again)
* For C and C++ files (.c, .cpp, and .h), parse the file to get function and variable declarations
  - Ability to list declarations - basically like an outline of the file
  - Ability to show line a declaration is on
  - Ability to show all lines of a function
* Simple syntax highlighting for C, C++, Bash, and Batch
* Repeat the last operation
* Better data structure for the lines that will allow easily moving lines around, deleting then, and inserting them
* Add text before/after string in line
* Find first occurance in line
* Find all occurances
* Replace all in line
* Replace first occurance in file
* Replace all in file
* Ability to change setting on how many lines to show before the line you are currently modifying (to give context)
* Tab completion for opening/saving files
* UTF-8 Support
* Ability to open multiple buffers (files) and switch between them.
* Duplicating a line
* Copying a line into clipboard
* Cutting a line
* Move a line to before/after a given line number
* Better prompt command parsing
* Insert before/after character in line
* Bookmarks
  - Creating a bookmarks
  - Listing all bookmarks
  - Using bookmark in place of line number for all commands
  - Deleting bookmarks
  - Ability to save them in separate (projects/settings) file

## Ideas
* Ability to use relative line numbers based on what was shown previously (from any command that previews any amount of lines, including insert, delete, preview, etc.)

## Commands
* 's' - Save
* '#' - Gives back information on the file, including number of lines, filename, number of characters, filetype, etc. * 'a (line#)' - Insert after the line number
* 'i (line#)' - Insert before the line number
* 'A (line#)' - Appends to a line
* 'I (line#)' - Prepends to a line
* 'r (line#)' - Replace a line with a new line
* 'R (line#) (string)' - Replace the first occurance of the string in the line
* 'x (line#)' - Deletes a line
* 'm (line#)' - Move the line up by one * 'M (line#)' - Move the line down by one * 'f (string)' - Finds the first occurance of the string in the file and prints the line it's on out
* 'u' - Undo the last operation, cannot undo an undo, cannot undo past 1 operation * 'c' - Continue from last line
* 'p' - Preview whole file
* 'P (line#:start) (line#:end)' - Preview a line or set of lines, including the line before and after
* 'd / D' - Save and Exit / Exit (without save)
* 'q / Q' - Save and Quit / Quit (without save)
