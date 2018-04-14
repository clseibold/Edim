# Code Documentation
## Files
* `lineeditor.h` - Header file included by all of the C files. Contains declarations for all files.
* `buffer.c` - Functions for opening a file into a buffer, closing a buffer, saving a buffer to a file, and any text/line manipulations that can be made to the buffer.
* `parsing.c` - Contains functions for getting input from user (including the new input system) as well as creating, recreating, and showing the outline for a buffer/file.
* `main.c` - The entry point. Contains the main menu.
* `editor.c` - All the functions for the Editor state.
* `colors.c` - Functions for printing colored output for Windows and Linux.
* `streatchybuffer.c` - Functions for the stretchy buffer dynamic array implementation (originally created by Sean Barratt?)

## Stretchy Buffer Dynamic Array
This implementation is based off of the one from the Bitwise project/tutorial series, which is in turn based off of the stretchy buffer and dynamic array from Sean Barratt's stb library.

* `buf_push` - Pushes an item onto the buffer. If buffer is NULL (doesn't exist), buffer is created first.
* `buf_len` - Returns length of buffer
* `buf_cap` - Returns capacity of buffer
* `buf_end` - Returns last element of buffer + 1
* `buf_add` - Allocates memory and increases length of buffer, but doesn't set the memory. Returns a pointer to the first element that was allocated.
* `buf_pop` - Subtracts one from the length of the buffer
* `buf_pop_all` - Sets buffer's length to 0
* `buf_free` - Free's the memory allocated for the buffer
