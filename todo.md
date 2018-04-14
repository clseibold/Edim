* Bookmarks
  - Add bookmark
  - Remove bookmark
  - Save bookmarks in project file
   - Default bookmarks
    - $ for last line
    - 1 for first line
  - Outlines can generate bookmarks as well
  - One character bookmark vs. full name bookmarks
  - Allow all commands to use bookmarks in place of line number
  - Bookmarks for ranges?
* When no line number passed into commands, use current line number instead of prompting for one
  - ~~On startup, current line number defaults to last line in file~~
  - Add a command to change current line number
  - Passing in a line number to a command will change the current line number to:
    - the line number if the command only affected that line
    - or, for the insert before/after commands, the last line inserted
    - or, for the find commands, the occurance (or first occurance) that was found
* Ability to join two or more lines together
* ~~Allow creating new file when file to open doesn't exist (instead of just crashing)~~
* Line numbers that are relative to the current line position; to make them, append '+' or '-' to number
* Ability to run shell commands from within the editor and show the command's output
* Ability to show contents of a function from C outline
  - Show outline of an individual function
* Autoindentation on text input
  - Virtual Whitespace system (like 4coder)?
* On main menu (or in editor), add command to show listing in current directory (or directory of file)
  - also allow showing tree of current directory (or directory of file)
* 'fn' for finding after cursor and 'fp' for finding before cursor. Concurrent fn's without a string passed in will continue searching. 'fn' and 'fp', just like 'f', will change the current cursor to the line the occurance was found on.
