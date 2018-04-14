#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <stdlib.h>

#include "lineeditor.h"

internal EditorState editorState_openAnotherFile(char *rest, int restLength);

internal void editorState_insertAfter(int line);
internal void editorState_insertBefore(int line);
internal void editorState_appendTo(int line);
internal void editorState_prependTo(int line);
internal void editorState_replaceLine(int line);
internal void editorState_replaceString(int line, char *str, int strLength);

internal void editorState_findStringInLine(int line, char *str, int strLength);
internal void editorState_findStringInFile(char *str, int strLength);
internal void editorState_deleteLine(int line);
internal void editorState_moveUp(int line);
internal void editorState_moveDown(int line);

/* Get input for new file */
State editorState(EditorState state, char args[MAXLENGTH / 4], int argsLength) {
    static EditorState subState = ED_EDITOR;
    static int initialSet = 0;
    static EditorState subStatePrev;
    
    if (state != ED_KEEP && !initialSet) {
        subState = state;
        initialSet = 1;
    }
    
    subStatePrev = subState;
    
    switch (subState) {
        case ED_NEW:
        {
            {
                Buffer buffer;
                buffer_initEmptyBuffer(&buffer);
                buffer.modified = true;
                buf_push(buffers, buffer);
                currentBuffer = buf_end(buffers) - 1;
            }
            
            if (argsLength > 1) {
                int i = 0;
                while (args[i] != ' ' && args[i] != '\n' && args[i] != '\0') {
                    buf_push(currentBuffer->openedFilename, args[i]);
                    ++i;
                }
            }
            
            if (initialSet) {
                printf("Opening a new file.\n");
                printf("Press Ctrl-D (or Ctrl-Z on Windows) on new line to denote End Of Input\n\n");
            }
            
            // Make sure currently stored text has been cleared out.
            assert(buf_len(currentBuffer->lines) == 0);
            
            subState = ED_EDITOR;
            subState = editorState_editor();
            if (subState == ED_KEEP) subState = subStatePrev;
        } break;
        case ED_OPEN:
        {
            {
                Buffer buffer;
                buffer_initEmptyBuffer(&buffer);
                buf_push(buffers, buffer);
                currentBuffer = buf_end(buffers) - 1;
            }
            
            int newFile = false;
            
            // Get arg for filename or Prompt for filename if no args provided
            if (argsLength <= 1) {
                printPrompt("Enter the filename: ");
                char filename[MAXLENGTH / 4];
                int filenameLength = 0;
                filenameLength = parsing_getLine(filename, MAXLENGTH / 4, true);
                while (filenameLength == -1) {
                    printPrompt("Enter the filename: ");
                    filenameLength = parsing_getLine(filename, MAXLENGTH / 4, true);
                }
                if (!buffer_openFile(currentBuffer, filename)) {
                    printf("File doesn't exist... Creating it.\n\n");
                    currentBuffer->modified = true;
                    subState = editorState_editor();
                    if (subState == ED_KEEP) subState = subStatePrev;
                    newFile = true;
                }
            } else {
                char *filename = malloc((argsLength + 1) * sizeof(char));
                int i = 0;
                int ii = 0;
                
                while (args[i] != ' ' && args[i] != '\n' && args[i] != '\0')
                {
                    filename[ii] = args[i];
                    ++i;
                    ++ii;
                }
                filename[ii] = '\0';
                if (!buffer_openFile(currentBuffer, filename)) {
                    printf("File doesn't exist... Creating it.\n\n");
                    currentBuffer->modified = true;
                    subState = editorState_editor();
                    if (subState == ED_KEEP) subState = subStatePrev;
                    newFile = true;
                }
                free(filename);
                filename = 0;
            }
            if (!newFile)
                printFileInfo();
            subState = ED_MENU;
        } break;
        case ED_EDITOR:
        {
            subState = editorState_editor();
            if (subState == ED_KEEP) subState = subStatePrev;
        } break;
        case ED_MENU:
        {
            subState = editorState_menu();
            if (subState == ED_KEEP) subState = subStatePrev;
        } break;
        case ED_EXIT: // TODO: Close the current buffer, move buffers down. If no buffers left, goto main menu
        {
            subState = ED_EDITOR;
            
            if (currentBuffer->modified) {
                printError("There are unsaved changes. Use 'E' or 'Q' to close without changes.");
                subState = ED_MENU;
            } else {
                int isLast = false;
                if (currentBuffer == &(buffers[buf_len(buffers) - 1]))
                    isLast = true;
                buffer_close(currentBuffer);
                if (isLast) {
                    buf_pop(buffers);
                    currentBuffer = buf_end(buffers) - 1;
                } else {
                    Buffer *source = currentBuffer + 1;
                    Buffer *destination = currentBuffer;
                    int amtToMove = buf_end(buffers) - source;
                    memmove(destination, source, sizeof(Buffer) * amtToMove);
                    buf_pop(buffers);
                }
                if (buf_len(buffers) <= 0) {
                    initialSet = 0;
                    return MAIN_MENU;
                }
                subState = ED_MENU;
            }
        } break;
        case ED_FORCE_EXIT:
        {
            int isLast = false;
            if (currentBuffer == &(buffers[buf_len(buffers) - 1]))
                isLast = true;
            buffer_close(currentBuffer);
            if (isLast) {
                buf_pop(buffers);
                currentBuffer = buf_end(buffers) - 1;
            } else {
                Buffer *source = currentBuffer + 1;
                Buffer *destination = currentBuffer;
                int amtToMove = buf_end(buffers) - source;
                memmove(destination, source, sizeof(Buffer) * amtToMove);
                buf_pop(buffers);
            }
            if (buf_len(buffers) <= 0) {
                initialSet = 0;
                return MAIN_MENU;
            }
            subState = ED_MENU;
        } break;
        case ED_QUIT:
        {
            int canQuit = true;
            
            for (int i = 0; i < buf_len(buffers); i++) {
                if (buffers[i].modified == true) {
                    canQuit = false;
                    break;
                }
            }
            
            if (!canQuit) {
                printError("There are unsaved changes in at least one of the open buffers. Use 'E' or 'Q' to close without changes.");
                subState = ED_MENU;
            } else return QUIT;
        } break;
        case ED_FORCE_QUIT:
        {
            exit(0);
            //return QUIT;
        } break;
        default:
        printError("Unknown command");
    }
    
    return KEEP;
}

/* Menu for Editor */
EditorState editorState_menu(void) {
    /* Prompt */
    if (buf_len(currentBuffer->openedFilename) > 0) {
        // TODO: This will also print out the directory, so I should get rid of everything before the last slash
        if (currentBuffer->modified) {
            printPrompt("\n<%d: %.*s*|%d> ", currentBuffer - buffers, (int) buf_len(currentBuffer->openedFilename), currentBuffer->openedFilename, currentBuffer->currentLine);
        } else {
            printPrompt("\n<%d: %.*s|%d> ", currentBuffer - buffers, (int) buf_len(currentBuffer->openedFilename), currentBuffer->openedFilename, currentBuffer->currentLine);
        }
        
    } else printPrompt("\n<%d: new file*|%d> ", currentBuffer - buffers, currentBuffer->currentLine);
    
    /* get first character - the menu item */
    char c;
    c = getchar();
    
    if (c == '\n') return KEEP;
    
    /* Store rest of line in rest */
    char rest[MAXLENGTH / 4];
    int restLength = parsing_getLine(rest, MAXLENGTH / 4, true);
    
    printf("\n");
    
    switch (c) {
        case 12: // Ctrl-L
        {
            clrscr();
        } break;
        case '?': // TODO: Add new file and open file.
        {
            if (buf_len(currentBuffer->openedFilename) > 0)
                printf("'%s' is currently open.\n\n", currentBuffer->openedFilename);
            else printf("An unnamed file is currently open.\n\n");
            
            printf("Use Ctrl-D or Ctrl-Z+Enter to denote end of input\n");
            printf("Use Ctrl-X+Enter to cancel the current command/operation\n");
            printf("\n");
            printf(" * Ctrl-L+Enter - Clear the screen\n");
            /* Edit - rewrite a specific line, group of lines, group of characters in a line (given column numbers), and word/group of words */
            //printf(" * 'e' - Edit\n");
            printf(" * '#' - Gives back information on the file, including number of lines, filename, number of characters, filetype, etc.\n");
            printf(" * 'j (line#)' - Set's current line to line number (no output). Use 'j$' to set last line as current line.");
            printf(" * 'a (line#)' - Insert after the line number\n");
            printf(" * 'i (line#)' - Insert before the line number\n");
            printf(" * 'A (line#)' - Appends to a line\n");
            printf(" * 'I (line#)' - Prepends to a line\n");
            printf(" * 'r (line#)' - Replace a line with a new line\n");
            printf(" * 'R (line#) (string)' - Replace the first occurance of the string in the line\n");
            printf(" * 'x (line#)' - Deletes a line\n");
            printf(" * 'm (line#)' - Move the line up by one\n");
            printf(" * 'M (line#)' - Move the line down by one\n");
            printf(" * 'f (string)' - Finds the first occurance of the string in the file and prints the line it's on out\n");
            printf(" * 'F (line#) (string)' - Find the first occurance of the string in the line and print the line out showing you where the occurance is\n");
            printf(" * 'u' - Undo the last operation, cannot undo an undo, cannot undo past 1 operation\n"); // TODO
            printf(" * 'c' - Continue from last line; Append to end of file\n");
            printf(" * 'p (line#:start)' - Preview whole file (optionally starting at given line)\n");
            printf(" * 'P (line#:start) (line#:end)' - Preview a line or set of lines, including the line before and after\n");
            printf(" * 'b' - List all currently open buffers\n");
            printf(" * 'b (buffer#)' - Switch current buffer to buffer #\n");
            printf(" * 'bn' - Switch current buffer to next buffer. Will wrap around when hits end.\n");
            printf(" * 'bp' - Switch current buffer to previous buffer. Will wrap around when hits beginning.\n");
            printf(" * 'o' - Open new buffer\n");
            printf(" * 'n' - Open new file buffer\n");
            printf(" * 's' - Save current buffer\n");
            //printf(" * 'S' - Save all buffers\n"); // TODO
            printf(" * 'e / E' - Exit current buffer / Exit current buffer (without save)\n");
            printf(" * 'q / Q' - Quit, closing all buffers / Quit, closing all buffers (without save)\n");
        } break;
        case 'j':
        {
            char *end;
            
            if (rest[0] == '$' || (rest[0] == ' ' && rest[1] == '$')) {
                currentBuffer->currentLine = buf_len(currentBuffer->lines);
                break;
            }
            
            int line = (int) strtol(rest, &end, 10);
            
            char lineInput[MAXLENGTH / 4];
            int length;
            while (line <= 0 || line > buf_len(currentBuffer->lines) || length == -1) {
                if (rest != end)
                    printError("That line number exceeds the bounds of the file.\n");
                printPrompt("Enter a line number: ");
                length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                line = (int) strtol(lineInput, &end, 10);
            }
            
            currentBuffer->currentLine = line;
        } break;
        case 's':
        {
            if (!currentBuffer->openedFilename && buf_len(currentBuffer->openedFilename) <= 0) {
                printPrompt("Enter a filename: ");
                char *filename = NULL;
                int length = parsing_getLine_dynamic(&filename, true);
                while (length <= 0) {
                    printPrompt("Enter a filename: ");
                    length = parsing_getLine_dynamic(&filename, true);
                }
                
                // Add null character, buffer_saveFile requires it.
                buf_push(filename, '\0');
                
                printf("Saving '%s'\n", filename);
                buffer_saveFile(currentBuffer, filename);
            } else {
                printf("Saving '%s'", currentBuffer->openedFilename);
                buffer_saveFile(currentBuffer, currentBuffer->openedFilename);
            }
        } break;
        case '#':
        {
            printFileInfo();
        } break;
        case 'c':
        {
            editorState_insertAfter(buf_len(currentBuffer->lines));
            recreateOutline();
        } break;
        case 'a':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            // Only ask for line number (again) if it exceeds the bounds of the file and a number was passed in the args to the command. If no line number was passed into command, then use current line number
            if (line != 0) {
                char lineInput[MAXLENGTH / 4];
                int length;
                while (line < 0 || line > buf_len(currentBuffer->lines) || length == -1) {
                    if (rest != end)
                        printError("That line number exceeds the bounds of the file.\n");
                    printPrompt("Enter a line number: ");
                    length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                    line = (int) strtol(lineInput, &end, 10);
                }
            } else line = -1;
            
            if (line == 0)
                editorState_insertBefore(1);
            else editorState_insertAfter(line);
            recreateOutline();
        } break;
        case 'i':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            // Only ask for line number (again) if it exceeds the bounds of the file and a number was passed in the args to the command. If no line number was passed into command, then use current line number
            if (line != 0) {
                char lineInput[MAXLENGTH / 4];
                int length;
                while (line <= 0 || line > buf_len(currentBuffer->lines) + 1 || length == -1) {
                    if (rest != end)
                        printError("That line number exceeds the bounds of the file.\n");
                    printPrompt("Enter a line number: ");
                    length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                    line = (int) strtol(lineInput, &end, 10);
                }
            } else line = -1;
            
            if (line == buf_len(currentBuffer->lines) + 1)
                editorState_insertAfter(buf_len(currentBuffer->lines));
            else editorState_insertBefore(line);
            recreateOutline();
        } break;
        case 'A':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            // Only ask for line number (again) if it exceeds the bounds of the file and a number was passed in the args to the command. If no line number was passed into command, then use current line number
            if (line != 0) {
                char lineInput[MAXLENGTH / 4];
                int length;
                while (line <= 0 || line > buf_len(currentBuffer->lines) || length == -1) {
                    if (rest != end)
                        printError("That line number exceeds the bounds of the file.\n");
                    printPrompt("Enter a line number: ");
                    length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                    line = (int) strtol(lineInput, &end, 10);
                }
            } else line = -1;
            
            editorState_appendTo(line);
            recreateOutline();
        } break;
        case 'I':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            // Only ask for line number (again) if it exceeds the bounds of the file and a number was passed in the args to the command. If no line number was passed into command, then use current line number
            if (line != 0) {
                char lineInput[MAXLENGTH / 4];
                int length;
                while (line <= 0 || line > buf_len(currentBuffer->lines) || length == -1) {
                    if (rest != end)
                        printError("That line number exceeds the bounds of the file.\n");
                    printPrompt("Enter a line number: ");
                    length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                    line = (int) strtol(lineInput, &end, 10);
                }
            } else line = -1;
            
            editorState_prependTo(line);
            recreateOutline();
        } break;
        case 'r':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            // Only ask for line number (again) if it exceeds the bounds of the file and a number was passed in the args to the command. If no line number was passed into command, then use current line number
            if (line != 0) {
                char lineInput[MAXLENGTH / 4];
                int length;
                while (line <= 0 || line > buf_len(currentBuffer->lines) || length == -1) {
                    if (rest != end)
                        printError("That line number exceeds the bounds of the file.\n");
                    printPrompt("Enter a line number: ");
                    length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                    line = (int) strtol(lineInput, &end, 10);
                }
            } else line = -1;
            
            editorState_replaceLine(line);
            recreateOutline();
        } break;
        case 'R':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            ++end; // Don't include the space in between the line number and the string to replace
            
            // Only ask for line number (again) if it exceeds the bounds of the file and a number was passed in the args to the command. If no line number was passed into command, then use current line number
            if (line != 0) {
                char lineInput[MAXLENGTH / 4];
                int length;
                while (line <= 0 || line > buf_len(currentBuffer->lines) || length == -1) {
                    if (rest != end)
                        printError("That line number exceeds the bounds of the file.\n");
                    printPrompt("Enter a line number: ");
                    length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                    line = (int) strtol(lineInput, &end, 10);
                }
            } else line = -1;
            
            char str[MAXLENGTH / 4];
            int strLength = 0;
            
            // If a string was already given with the command
            if (rest + restLength - end - 1 > 0) {
                // Copy into str
                strLength = rest + restLength - end;
                strncpy(str, end, strLength);
                // Use it instead of asking for a string to replace
                editorState_replaceString(line, str, strLength - 1);
                break;
            }
            
            printPrompt("Enter the string to replace: ");
            strLength = parsing_getLine(str, MAXLENGTH / 4, false);
            while (strLength == -1) {
                printPrompt("Enter the string to replace: ");
                strLength = parsing_getLine(str, MAXLENGTH / 4, true);
            }
            
            editorState_replaceString(line, str, strLength);
            recreateOutline();
        } break;
        case 'x':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            // Only ask for line number (again) if it exceeds the bounds of the file and a number was passed in the args to the command. If no line number was passed into command, then use current line number
            if (line != 0) {
                char lineInput[MAXLENGTH / 4];
                int length;
                while (line == 0 || line > buf_len(currentBuffer->lines) || length == -1) {
                    if (rest != end)
                        printError("That line number exceeds the bounds of the file.\n");
                    printPrompt("Enter a line number: ");
                    length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                    line = (int) strtol(lineInput, &end, 10);
                }
            } else line = -1;
            
            editorState_deleteLine(line);
            recreateOutline();
        } break;
        case 'm':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            printf("%d\n", line);
            
            // Only ask for line number (again) if it exceeds the bounds of the file and a number was passed in the args to the command. If no line number was passed into command, then use current line number
            if (line != 0) {
                char lineInput[MAXLENGTH / 4];
                int length;
                while (line <= 0 || line > buf_len(currentBuffer->lines) || length == -1) {
                    if (rest != end)
                        printError("That line number exceeds the bounds of the file.\n");
                    printPrompt("Enter a line number: ");
                    length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                    line = (int) strtol(lineInput, &end, 10);
                }
            } else line = -1;
            
            editorState_moveUp(line);
            recreateOutline();
        } break;
        case 'M':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            // Only ask for line number (again) if it exceeds the bounds of the file and a number was passed in the args to the command. If no line number was passed into command, then use current line number
            if (line != 0) {
                char lineInput[MAXLENGTH / 4];
                int length;
                while (line <= 0 || line > buf_len(currentBuffer->lines) || length == -1) {
                    if (rest != end)
                        printError("That line number exceeds the bounds of the file.\n");
                    printPrompt("Enter a line number: ");
                    length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                    line = (int) strtol(lineInput, &end, 10);
                }
            } else line = -1;
            
            editorState_moveDown(line);
            recreateOutline();
        } break;
        case 'p':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            if (line == 0)
                printText(currentBuffer->currentLine - 1);
            else if (line < 0 || line > buf_len(currentBuffer->lines))
                printText(0);
            else printText(line - 1);
        } break;
        case 'P':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            ++end; // Don't include the space in between the first line number and the second line number
            
            // Only ask for line number (again) if it exceeds the bounds of the file and a number was passed in the args to the command. If no line number was passed into command, then use current line number
            if (line != 0) {
                char lineInput[MAXLENGTH / 4];
                int length;
                while (line == 0 || line > buf_len(currentBuffer->lines) || length == -1) {
                    if (rest != end)
                        printError("That line number exceeds the bounds of the file.\n");
                    printPrompt("Enter a line number: ");
                    length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                    line = (int) strtol(lineInput, &end, 10);
                }
            } else line = currentBuffer->currentLine;
            
            int endLine;
            char *end2;
            char str[MAXLENGTH / 4];
            int strLength = 0;
            
            // If a second line number was already given with the command
            if (rest + restLength - end - 1 > 0) {
                endLine = (int) strtol(end, &end2, 10);
                // If given a valid number
                if (endLine != -1) {
                    // Print the range of lines along with the line before and after
                    if (line - 2 >= 0)
                        printLine(line - 2, 0, true);
                    for (int i = line; i <= endLine; i++) {
                        if (i - 1 >= 0 && i - 1 < buf_len(currentBuffer->lines))
                            printLine(i - 1, 0, true);
                    }
                    if (endLine < buf_len(currentBuffer->lines))
                        printLine(endLine, 0, true);
                    break;
                }
            }
            
            // Otherwise, just print that one line
            if (line - 2 >= 0)
                printLine(line - 2, 0, true);
            printLine(line - 1, 0, true);
            if (line < buf_len(currentBuffer->lines))
                printLine(line, 0, true);
        } break;
        case 'f':
        {
            char str[MAXLENGTH / 4];
            int strLength = 0;
            
            // If a string was already given with the command
            if (restLength - 1 > 0) {
                // Copy into str
                strLength = restLength;
                strncpy(str, rest, strLength);
                // Use it instead of asking for a string to replace
                editorState_findStringInFile(str, strLength - 1);
                break;
            }
            
            printPrompt("Enter the string to find: ");
            strLength = parsing_getLine(str, MAXLENGTH / 4, false);
            while (strLength == -1) {
                printPrompt("Enter the string to find: ");
                strLength = parsing_getLine(str, MAXLENGTH / 4, true);
            }
            
            editorState_findStringInFile(str, strLength);
        } break;
        case 'F':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            ++end; // Don't include the space in between the line number and the string to replace
            
            // Only ask for line number (again) if it exceeds the bounds of the file and a number was passed in the args to the command. If no line number was passed into command, then use current line number
            if (line != 0) {
                char lineInput[MAXLENGTH / 4];
                int length;
                while (line <= 0 || line > buf_len(currentBuffer->lines) || length == -1) {
                    if (rest != end)
                        printError("That line number exceeds the bounds of the file.\n");
                    printPrompt("Enter a line number: ");
                    length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                    line = (int) strtol(lineInput, &end, 10);
                }
            } else line = -1;
            
            char str[MAXLENGTH / 4];
            int strLength = 0;
            
            // If a string was already given with the command
            if (rest + restLength - end - 1 > 0) {
                // Copy into str
                strLength = rest + restLength - end;
                strncpy(str, end, strLength);
                // Use it instead of asking for a string to replace
                //editorState_replaceString(line, str, strLength - 1);
                editorState_findStringInLine(line, str, strLength);
                break;
            }
            
            printPrompt("Enter the string to find: ");
            strLength = parsing_getLine(str, MAXLENGTH / 4, false);
            while (strLength == -1) {
                printPrompt("Enter the string to find: ");
                strLength = parsing_getLine(str, MAXLENGTH / 4, true);
            }
            
            editorState_findStringInLine(line, str, strLength);
        } break;
        case 'b':
        {
            if (restLength > 0) {
                switch (rest[0]) {
                    case 'n':
                    {
                        int current = currentBuffer - buffers;
                        int next = current + 1;
                        if (next >= buf_len(buffers))
                            next = 0;
                        
                        currentBuffer = &(buffers[next]);
                        return ED_KEEP;
                    } break;
                    case 'p':
                    {
                        int current = currentBuffer - buffers;
                        int previous = current - 1;
                        if (previous < 0)
                            previous = buf_len(buffers) - 1;
                        
                        currentBuffer = &(buffers[previous]);
                        return ED_KEEP;
                    } break;
                }
            }
            
            // If a integer was  given with the command
            if (restLength - 1 > 0) {
                char *end;
                int index = (int) strtol(rest, &end, 10);
                
                char lineInput[MAXLENGTH / 4];
                int length;
                while (index < 0 || index > buf_len(buffers) - 1 || length == -1) {
                    printError("That buffer doesn't exist.");
                    printPrompt("Enter a buffer number: ");
                    length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                    index = (int) strtol(lineInput, &end, 10);
                }
                
                currentBuffer = &(buffers[index]);
                
                return ED_KEEP;
            }
            
            for (int i = 0; i < buf_len(buffers); i++) {
                if (buf_len(buffers[i].openedFilename) <= 0) {
                    // We can assume this is the current buffer because you can't switch or close a buffer with unsaved changes
                    if (currentBuffer == &(buffers[i]))
                        printf("*%3d: new file", i);
                    else printf("%4d: new file", i);
                } else {
                    if (currentBuffer == &(buffers[i])) {
                        printf("*%3d: %.*s", i, (int) buf_len(buffers[i].openedFilename), buffers[i].openedFilename);
                    } else {
                        printf("%4d: %.*s", i, (int) buf_len(buffers[i].openedFilename), buffers[i].openedFilename);
                    }
                }
                if (buffers[i].modified)
                    printf("*");
                printf("\n");
            }
        } break;
        case 'l': // TODO: Reload file, prompt if unsaved changes?
        {
            
        } break;
        case 'o':
        {
            return editorState_openAnotherFile(rest, restLength);
        } break;
        case 'n':
        {
            // TODO
        } break;
        case 'e':
        {
            return ED_EXIT;
        } break;
        case 'E':
        {
            return ED_FORCE_EXIT;
        } break;
        case 'q':
        {
            return ED_QUIT;
        } break;
        case 'Q':
        {
            return ED_FORCE_QUIT;
        } break;
        // Hacked in - change filetype to FT_C because of how poor my filetype extension matching is
        case 't':
        {
            currentBuffer->fileType = FT_C;
            recreateOutline();
        } break;
        case 'T':
        {
            printf("FileType: %d\n", currentBuffer->fileType);
        } break;
        default:
        {
            printError("Unknown command");
        } break;
    }
    
    return ED_KEEP;
}

internal EditorState editorState_openAnotherFile(char *rest, int restLength) {
    char str[MAXLENGTH / 4];
    int strLength = 0;
    
    // If a string was already given with the command
    if (restLength - 1 > 0) {
        // Copy into str
        strLength = restLength;
        strncpy(str, rest, strLength);
        
        // Make sure string ends with zero termination
        assert(str[strLength - 1] == '\0');
        
        {
            Buffer buffer;
            buffer_initEmptyBuffer(&buffer);
            buf_push(buffers, buffer);
            currentBuffer = buf_end(buffers) - 1;
        }
        
        if (!buffer_openFile(currentBuffer, str)) {
            currentBuffer->modified = true;
            return editorState_editor();
        } else {
            printFileInfo();
            return ED_MENU;
        }
    }
    
    printPrompt("Enter file to open: ");
    strLength = parsing_getLine(str, MAXLENGTH / 4, false);
    while (strLength == -1) {
        printPrompt("Enter file to open: ");
        strLength = parsing_getLine(str, MAXLENGTH / 4, true);
    }
    
    // Make sure string ends with zero termination
    assert(str[strLength - 1] == '\0');
    
    {
        Buffer buffer;
        buffer_initEmptyBuffer(&buffer);
        buf_push(buffers, buffer);
        currentBuffer = buf_end(buffers) - 1;
    }
    
    if (!buffer_openFile(currentBuffer, str)) {
        printf("File doesn't exist... Creating it.\n\n");
        currentBuffer->modified = true;
        return ED_EDITOR;
    } else printFileInfo();
    
    return ED_KEEP;
}

// Editor - will allow user to type in anything, showing line number at start of new lines. To exit the editor, press Ctrl-D on Linux or Ctrl-Z+Enter on Windows. As each new line is entered, the characters will be added to a char pointer streatchy buffer (dynamic array). Then, this line will be added to the streatchy buffer of lines (called 'lines').
EditorState editorState_editor(void) {
    char c;
    int line = 1;
    
    // If continuing a previously typed-in file,
    // start on last line and overwrite the EOF character
    if (buf_len(currentBuffer->lines) > 0) {
        line = buf_len(currentBuffer->lines) + 1;
    }
    
    // Show the previous line to give context.
    printLine(line - 2, 0, true);
    
    char *chars = NULL;
    
#ifdef _WIN32
    printLineNumber("%5d ", line);
    int canceled = false;
    while ((chars = getInput(&canceled)) != NULL) {
        buf_push(currentBuffer->lines, ((Line) { chars }));
        ++line;
        printLineNumber("%5d ", line);
    }
    
    if (canceled) {
        // TODO: close the buffer?
    }
    printf("\n");
#else
    printLineNumber("%5d ", line);
    while ((c = getchar()) != EOF) {
        buf_push(chars, c);
        if (c == '\n') {
            buf_push(currentBuffer->lines, ((Line) { chars }));
            ++line;
            printLineNumber("%5d ", line);
            chars = NULL; // Create new char stretchy buffer for next line
        }
    }
#endif
    
    // Set cursor to end of file
    currentBuffer->currentLine = buf_len(currentBuffer->lines);
    
    return ED_MENU;
}

// Insert lines after a specific line. Denote end of input by typing Ctrl-D (or Ctrl-Z+Enter on Windows) on new line.
internal void editorState_insertAfter(int line) {
    if (line == -1) line = currentBuffer->currentLine;
    char c;
    if (line - 1 >= 0 && line - 1 < buf_len(currentBuffer->lines))
        printLine(line - 1, 0, true);
    int currentLine = line + 1;
    
    Line *insertLines = NULL;
    char *chars = NULL;
    
#ifdef _WIN32
    printLineNumber("a%4d ", currentLine);
    int canceled = false;
    while ((chars = getInput(&canceled)) != NULL) {
        buf_push(insertLines, ((Line) { chars }));
        ++currentLine;
        printLineNumber("a%4d ", currentLine);
    }
    
    if (canceled) {
        buf_free(chars);
        for (int i = 0; i < buf_len(insertLines); i++) {
            buf_free(insertLines[i].chars);
        }
        buf_free(insertLines);
        // Cancel the operation by returning
        return;
    }
    printf("\n");
#else
    printLineNumber("a%4d ", currentLine);
    while ((c = getchar()) != EOF) {
        if (c == (char) 24) { // Ctrl-X (^X) - Cancel
            // Free the new unused line buffers
            buf_free(chars);
            for (int i = 0; i < buf_len(insertLines); i++) {
                buf_free(insertLines[i].chars);
            }
            buf_free(insertLines);
            // Discard the new line character
            getchar();
            // Cancel the operation by returning
            return;
        }
        buf_push(chars, c);
        if (c == '\n') {
            buf_push(insertLines, ((Line) { chars }));
            ++currentLine;
            printLineNumber("a%4d ", currentLine);
            chars = NULL; // create new char stretchy buffer for next line
        }
    }
#endif
    
    int firstMovedLine = buffer_insertAfterLine(currentBuffer, line, insertLines);
    
    // Free the old lines stretchy buffer
    buf_free(insertLines);
    
    // Show the line that was moved due to inserting before it (and after the line before it)
    if (firstMovedLine <= buf_len(currentBuffer->lines))
        printLine(firstMovedLine - 1, 'v', true);
}

internal void editorState_insertBefore(int line) {
    if (line == -1) line = currentBuffer->currentLine;
    char c;
    if (line - 2 >= 0 && line - 1 < buf_len(currentBuffer->lines))
        printLine(line - 2, 0, true);
    int currentLine = line;
    
    Line *insertLines = NULL;
    char *chars = NULL;
    
#ifdef _WIN32
    printLineNumber("i%4d ", currentLine);
    int canceled = false;
    while ((chars = getInput(&canceled)) != NULL) {
        buf_push(insertLines, ((Line) { chars }));
        ++currentLine;
        printLineNumber("i%4d ", currentLine);
    }
    
    if (canceled) {
        buf_free(chars);
        for (int i = 0; i < buf_len(insertLines); i++) {
            buf_free(insertLines[i].chars);
        }
        buf_free(insertLines);
        // Cancel the operation by returning
        return;
    }
    printf("\n");
#else
    printLineNumber("i%4d ", currentLine);
    while ((c = getchar()) != EOF) {
        if (c == (char) 24) { // Ctrl-X (^X) - Cancel
            // Free the new unused line buffers
            buf_free(chars);
            for (int i = 0; i < buf_len(insertLines); i++) {
                buf_free(insertLines[i].chars);
            }
            buf_free(insertLines);
            // Discard the new line character
            getchar();
            // Cancel the operation by returning
            return;
        }
        buf_push(chars, c);
        if (c == '\n') {
            buf_push(insertLines, ((Line) { chars }));
            ++currentLine;
            printLineNumber("i%4d ", currentLine);
            chars = NULL; // create new char stretchy buffer for next line
        }
    }
#endif
    
    int firstMovedLine = buffer_insertBeforeLine(currentBuffer, line, insertLines);
    
    // Free the old lines stretchy buffer
    buf_free(insertLines);
    
    // Show the line that was moved due to the insertion before it
    if (firstMovedLine <= buf_len(currentBuffer->lines))
        printLine(firstMovedLine - 1, 'v', true);
}

internal void editorState_appendTo(int line) {
    if (line == -1) line = currentBuffer->currentLine;
    char c;
    char *chars = NULL;
    if (line - 2 >= 0 && line - 2 < buf_len(currentBuffer->lines))
        printLine(line - 2, 0, true);
    
    printLine(line - 1, 'A', false);
#ifdef _WIN32
    int canceled = false;
    chars = getInput(&canceled);
    if (canceled) {
        // Delete the chars buffer
        buf_free(chars);
        // Cancel the operation by returning
        return;
    }
#else
    while ((c = getchar()) != EOF) {
        if (c == (char) 24) { // Ctrl-X (^X) - Cancel
            // Delete the chars buffer
            buf_free(chars);
            // Discard the new line character that's typed after Ctrl-X
            getchar();
            // Cancel the operation by returning
            return;
        }
        buf_push(chars, c);
        //count++;
        if (c == '\n') break;
    }
#endif
    
    buffer_appendToLine(currentBuffer, line, chars);
    buf_free(chars);
}

internal void editorState_prependTo(int line) {
    if (line == -1) line = currentBuffer->currentLine;
    char c;
    if (line - 2 >= 0 && line - 2 < buf_len(currentBuffer->lines))
        printLine(line - 2, 0, true);
    
    char *chars = NULL; // The new char stretchy buffer
    
    printLine(line - 1, 'I', true);
    printPrompt("%4s ^- ", "");
#ifdef _WIN32
    int canceled = false;
    chars = getInput(&canceled);
    if (canceled) {
        // Delete the chars buffer
        buf_free(chars);
        // Cancel the operation by returning
        return;
    }
    // Get rid of new line
    if (chars[buf_len(chars) - 1] == '\n') {
        buf_pop(chars);
    }
#else
    while ((c = getchar()) != EOF) {
        if (c == (char) 24) { // Ctrl-X (^X) - Cancel
            // Free the new unused line buffer
            buf_free(chars);
            // Discard the new line character
            getchar();
            // Cancel the operation by returning
            return;
        }
        if (c == '\n') break; // Make sure new line is not pushed onto the buffer
        buf_push(chars, c);
    }
#endif
    
    buffer_prependToLine(currentBuffer, line, chars);
}

internal void editorState_replaceLine(int line) {
    if (line == -1) line = currentBuffer->currentLine;
    char c;
    if (line - 2 >= 0 && line - 2 < buf_len(currentBuffer->lines))
        printLine(line - 2, 0, true);
    
    char *chars = NULL; // The new char stretchy buffer
    
    printLine(line - 1, 'r', true);
    printf("%5s ", "");
#ifdef _WIN32
    int canceled = false;
    chars = getInput(&canceled);
    if (canceled) {
        // Delete the chars buffer
        buf_free(chars);
        // Cancel the operation by returning
        return;
    }
#else
    while ((c = getchar()) != EOF) {
        if (c == (char) 24) { // Ctrl-X (^X) - Cancel
            // Free the new unused line buffer
            buf_free(chars);
            // Discard the new line character
            getchar();
            // Cancel the operation by returning
            return;
        }
        buf_push(chars, c);
        if (c == '\n') break;
    }
#endif
    
    buffer_replaceLine(currentBuffer, line, chars);
}

internal void editorState_replaceString(int line, char *str, int strLength) {
    if (line == -1) line = currentBuffer->currentLine;
    char c;
    
    int index = buffer_findStringInLine(currentBuffer, line, str, strLength);
    
    if (index == -1) {
        printError("No occurance of '%.*s' found\n", strLength, str);
        return;
    }
    
    // Print the previous line to give context
    if (line - 2 >= 0 && line - 2 < buf_len(currentBuffer->lines))
        printLine(line - 2, 0, true);
    
    // Print the string where the replacement is occuring
    printLine(line - 1, 'R', true);
    
    // Create a string (to be printed) with an arrow pointing to the beginning and end of the first occurance of the string being replaced.
    int strPointToMatchLength;
    if (str[strLength - 1] == '\0' || str[strLength - 1] == '\n') {
        strPointToMatchLength = index + strLength - 1;
    } else {
        strPointToMatchLength = index + strLength;
    }
    char *strPointToMatch = alloca(sizeof(char) * (strPointToMatchLength));
    for (int i = 0; i < strPointToMatchLength; i++) {
        if (i == index || i == strPointToMatchLength - 1) {
            strPointToMatch[i] = '^';
            continue;
        }
        if (i > index && i < strPointToMatchLength - 1)
            strPointToMatch[i] = '-';
        else strPointToMatch[i] = ' ';
    }
    
    char *chars = NULL; // What the string will be replaced with
    
    printPrompt("%5s %.*s- ", "", strPointToMatchLength, strPointToMatch); // TODO
#ifdef _WIN32
    int canceled = false;
    chars = getInput(&canceled);
    if (canceled) {
        // Delete the chars buffer
        buf_free(chars);
        // Cancel the operation by returning
        return;
    }
    // Get rid of new line
    if (chars[buf_len(chars) - 1] == '\n') {
        buf_pop(chars);
    }
#else
    while ((c = getchar()) != EOF) {
        if (c == (char) 24) { // Ctrl-X (^X) - Cancel
            // Free the new unused line buffer
            buf_free(chars);
            // Discard the new line character
            getchar();
            // Cancel the operation by returning
            return;
        }
        if (c == '\n') break; // Make sure new line is not pushed onto the buffer
        buf_push(chars, c);
    }
#endif
    
    buffer_replaceInLine(currentBuffer, line, index, index + strLength - 1, chars);
}

// Finds the first occrance of the string in the given line
// Displays the line with an arrow pointing to the occurance
// Will also show the line before it to give context and the column of the start of the occurance
internal void editorState_findStringInLine(int line, char *str, int strLength) {
    if (line == -1) line = currentBuffer->currentLine;
    int index = buffer_findStringInLine(currentBuffer, line, str, strLength);
    
    if (index == -1) {
        printError("No occurance of '%.*s' was found in line %d\n", strLength, str, line);
        return;
    }
    
    // Print the previous line to give context
    if (line - 1 >= 0 && line - 1 < buf_len(currentBuffer->lines))
        printLine(line - 2, 0, true);
    
    // Print the line
    printLine(line - 1, 0, true);
    
    // Create a string (to be printed) with an arrow pointing to the beginning and end of the first occurance of the string being matched.
    int strPointToMatchLength;
    if (str[strLength - 1] == '\0' || str[strLength - 1] == '\n') {
        strPointToMatchLength = index + strLength - 1;
    } else {
        strPointToMatchLength = index + strLength;
    }
    
    char *strPointToMatch = alloca(sizeof(char) * (strPointToMatchLength));
    for (int i = 0; i < strPointToMatchLength; i++) {
        if (i == index || i == strPointToMatchLength - 1) {
            strPointToMatch[i] = '^';
            continue;
        }
        if (i > index && i < strPointToMatchLength - 1)
            strPointToMatch[i] = '-';
        else strPointToMatch[i] = ' ';
    }
    
    // Print out the string that's points to the occurance
    printf("%5s %.*s- \n", "", strPointToMatchLength, strPointToMatch); // TODO: printInfo()
}

// Finds the first occurance of the string in the file
// Displays the line it is on with an arrow pointing to the occurance
// Will also show the line before it to give context
internal void editorState_findStringInFile(char *str, int strLength) {
    int colIndex = -1;
    int foundIndex = buffer_findStringInFile(currentBuffer, str, strLength, &colIndex);
    
    // If no occurance found in file
    if (foundIndex == -1) {
        printError("No occurance of '%.*s' found\n", strLength, str);
        return;
    }
    
    // Print the previous line to give context
    if (foundIndex - 1 >= 0 && foundIndex - 1 < buf_len(currentBuffer->lines))
        printLine(foundIndex - 1, 0, true);
    
    // Print the string where the occurance was found
    printLine(foundIndex, 0, true);
    
    // Create a string (to be printed) with an arrow pointing to the beginning and end of the first occurance of the string being matched.
    int strPointToMatchLength;
    if (str[strLength - 1] == '\0' || str[strLength - 1] == '\n') {
        strPointToMatchLength = colIndex + strLength - 1;
    } else {
        strPointToMatchLength = colIndex + strLength;
    }
    char *strPointToMatch = alloca(sizeof(char) * (strPointToMatchLength));
    for (int i = 0; i < strPointToMatchLength; i++) {
        if (i == colIndex || i == strPointToMatchLength - 1) {
            strPointToMatch[i] = '^';
            continue;
        }
        if (i > colIndex && i < strPointToMatchLength - 1)
            strPointToMatch[i] = '-';
        else strPointToMatch[i] = ' ';
    }
    
    // Print out the string that's points to the occurance
    printf("%5s %.*s- \n", "", strPointToMatchLength, strPointToMatch); // TODO: printInfo()
}

internal void editorState_deleteLine(int line) {
    if (line == -1) line = currentBuffer->currentLine;
    // Show the line before the line that's being deleted
    if (line - 1 > 0)
        printLine(line - 2, 0, true);
    
    if (line == buf_len(currentBuffer->lines)) { // TODO: This isn't working correctly
        printLine(line - 1, 'x', true);
        buffer_deleteLine(currentBuffer, buf_len(currentBuffer->lines));
        
        // Show the first line that was moved - the line # should be the same as the line that was deleted
        if (line <= buf_len(currentBuffer->lines))
            printLine(line - 1, '^', true);
        return;
    }
    
    printLine(line - 1, 'x', true);
    
    buffer_deleteLine(currentBuffer, line);
    
    // Show the first line that was moved - the line # should be the same as the line that was deleted
    if (line <= buf_len(currentBuffer->lines))
        printLine(line - 1, '^', true);
}

internal void editorState_moveUp(int line) {
    if (line == -1) line = currentBuffer->currentLine;
    // Show the line before the line being moved up to
    if (line - 2 > 0)
        printLine(line - 3, 0, true);
    
    buffer_moveLineUp(currentBuffer, line);
    
    // Print the new position of the line that was moved and the old line that was moved down
    printLine(line - 2, '^', true);
    printLine(line - 1, 'v', true);
    
    // Print the next line to give context
    printLine(line, 0, true);
}

internal void editorState_moveDown(int line) {
    if (line == -1) line = currentBuffer->currentLine;
    // Show the line before the line being moved down
    if (line - 1 > 0)
        printLine(line - 2, 0, true);
    
    buffer_moveLineDown(currentBuffer, line);
    
    // Print the new position of the line that was moved and the old line that was moved up
    printLine(line - 1, '^', true);
    printLine(line, 'v', true);
    
    // Print the next line to give context
    printLine(line + 1, 0, true);
}

/* Print the currently stored text with line numbers */
void printText(int startLine) {
    if (buf_len(currentBuffer->lines) <= 0) {
        printLineNumber("%5d ", 1);
        printf("\n");
        return;
    }
    
    int linesAtATime = 15; // TODO: Should have a setting for this.
    int offset = startLine;
    char c;
    
    for (int line = offset; line < linesAtATime + offset + 1 && line <= buf_len(currentBuffer->lines); line++) {
        if (line == buf_len(currentBuffer->lines)) {
            if (line + 1 == currentBuffer->currentLine)
                printLineNumber("%c%4d ", '*', line + 1);
            else printLineNumber("%5d ", line + 1);
            break;
        }
        printLine(line, 0, true);
    }
    offset = linesAtATime + offset + 1;
    if (offset >= buf_len(currentBuffer->lines)) {
        printf("\n");
        return;
    }
    printPrompt("\n<%d: %s | preview> ", currentBuffer - buffers, currentBuffer->openedFilename);
    
#ifdef _WIN32
    while ((c = getch()) != EOF && offset < buf_len(currentBuffer->lines))
#else
        while ((c = getchar()) != EOF && offset < buf_len(currentBuffer->lines))
#endif
    {
        if (c == '?') {
            // Print help info about preview command here
            printf("\nPreviewing '%s'\n", currentBuffer->openedFilename);
            printf(" * 'q' or Ctrl-X to stop previewing\n");
            printf(" * 'Q' to exit the whole program\n");
            printf(" * Enter to show the next lines\n");
            // Discard the enter key
            //getchar();
            printPrompt("\n<%d: %s | preview> ", currentBuffer - buffers, currentBuffer->openedFilename);
            continue;
        } else if (c == 'q' || c == 24) { // 26 is Ctrl-X, aka CANCEL
            // Discard the enter key
            //getchar();
            break;
        } else if (c == 'Q') {
            exit(0);
        }
        
        printf("\r");
        for (int i = 0; i < 45; i++) // TODO: Hacky
            printf(" ");
        printf("\r");
        for (int line = offset; line < linesAtATime + offset + 1 && line <= buf_len(currentBuffer->lines); line++) {
            if (line == buf_len(currentBuffer->lines)) {
                if (line + 1 == currentBuffer->currentLine)
                    printLineNumber("%c%4d ", '*', line + 1);
                else printLineNumber("%5d ", line + 1);
                break;
            }
            printLine(line, 0, true);
        }
        
        offset = linesAtATime + offset + 1;
        if (offset >= buf_len(currentBuffer->lines)) {
            break;
        }
        printPrompt("\n<%d: %s | preview> ", currentBuffer - buffers, currentBuffer->openedFilename);
    }
    
    printf("\n");
}

/*
Prints one line of text given the line number. Note that the line numbers start at 0 (although they are displayed to the user starting at 1).
Pass false into printNewLine so the new line at the end is not printed
*/
void printLine(int line, char operation, int printNewLine) {
    if (line == -1) line = currentBuffer->currentLine;
    // If no lines in buffer and line is 0, show one line.
    if (buf_len(currentBuffer->lines) <= 0 && line == 0) {
        if (operation != 0)
            printLineNumber("%c%4d ", operation, 1);
        else {
            printLineNumber("%5d ", 1);
        }
        printf("\n");
        return;
    }
    
    // If line is last line in file (one above the length of the lines)
    if (line == buf_len(currentBuffer->lines)) {
        if (operation != 0)
            printLineNumber("%c%4d ", operation, line + 1);
        else {
            if (line + 1 == currentBuffer->currentLine)
                printLineNumber("%c%4d ", '*', line + 1);
            else printLineNumber("%5d ", line + 1);
        }
        return;
    } else if (line > buf_len(currentBuffer->lines)) {
        // Error!
        return;
    }
    
    if (operation != 0)
        printLineNumber("%c%4d ", operation, line + 1);
    else {
        if (line + 1 == currentBuffer->currentLine)
            printLineNumber("%c%4d ", '*', line + 1);
        else printLineNumber("%5d ", line + 1);
    }
    
    int length = buf_len(currentBuffer->lines[line].chars);
    // If shouldn't print new line and end of line is a new line, subtract it off from the length
    if (!printNewLine && currentBuffer->lines[line].chars[length - 1] == '\n')
        --length;
    
    //printf("%.*s", length, currentBuffer->lines[line].chars);
    for (int i = 0; i < length; i++) {
        if (currentBuffer->lines[line].chars[i] == '\t')
            printf("    "); // 4 spaces // TODO: Add setting for this
        else putchar(currentBuffer->lines[line].chars[i]);
    }
}

// TODO: number of chars, filetype, syntax highlighting enabled, outline enabled
void printFileInfo(void) {
    printf("File information for '%.*s'\n", (int) buf_len(currentBuffer->openedFilename), currentBuffer->openedFilename);
    //printf("File Information for '%s'\n", currentBuffer->openedFilename);
    
    int numOfLines = buf_len(currentBuffer->lines);
    // If last character of last line ends with a new line, add one to the number of lines
    Line lastLine = currentBuffer->lines[buf_len(currentBuffer->lines) - 1];
    char lastChar = lastLine.chars[buf_len(lastLine.chars) - 1];
    if (lastChar == '\n') {
        numOfLines++;
    }
    printf("Number of Lines: %d\n", numOfLines);
    
    // If markdown file, print outline
    if (currentBuffer->fileType == FT_MARKDOWN) {
        if (buf_len(currentBuffer->outline.markdown_nodes) > 0)
            printf("Outline:\n");
    } else if (currentBuffer->fileType == FT_C) {
        if (buf_len(currentBuffer->outline.c_nodes) > 0)
            printf("Outline:\n");
    }
    showOutline();
}
