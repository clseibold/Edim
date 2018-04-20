#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <stdlib.h>

#include "edimcoder.h"

internal void editorState_openAnotherFile(char *rest, int restLength);
internal void editorState_openNewFile(char *rest, int restLength);

internal int getLineNumber();
internal int checkLineNumber(int original_line);

internal void editorState_insertAfter(char *rest);
internal void editorState_insertBefore(char *rest);
internal void editorState_appendTo(char *rest);
internal void editorState_prependTo(char *rest);
internal void editorState_replaceLine(char *rest);
internal void editorState_replaceString(char *rest, int restLength);

internal void editorState_findStringInLine(char *rest, int restLength);
internal void editorState_findStringInFile(char *rest, int restLength);
internal void editorState_deleteLine(char *rest);
internal void editorState_moveUp(char *rest);
internal void editorState_moveDown(char *rest);

/* Menu for Editor */
State editorState_menu(void) {
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
            currentBuffer->currentLine = buf_len(currentBuffer->lines);
            editorState_insertAfter(rest);
        } break;
        case 'a':
        {
            editorState_insertAfter(rest);
        } break;
        case 'i':
        {
            editorState_insertBefore(rest);
        } break;
        case 'A':
        {
            editorState_appendTo(rest);
        } break;
        case 'I':
        {
            editorState_prependTo(rest);
        } break;
        case 'r':
        {
            editorState_replaceLine(rest);
        } break;
        case 'R':
        {
            editorState_replaceString(rest, restLength);
        } break;
        case 'x':
        {
            editorState_deleteLine(rest);
        } break;
        case 'm':
        {
            editorState_moveUp(rest);
        } break;
        case 'M':
        {
            editorState_moveDown(rest);
        } break;
        case 'p':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            if (line == 0) {
                line = currentBuffer->currentLine;
            } else {
                line = checkLineNumber(line);
            }
            
            if (line < 0 || line > buf_len(currentBuffer->lines))
                printText(0);
            else printText(line - 1);
        } break;
        case 'P':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            if (line == 0) {
                line = currentBuffer->currentLine;
            } else {
                line = checkLineNumber(line);
                // Don't include the space in between the line number and the string to replace
                ++end;
            }
            
            int endLine;
            char *end2;
            char str[MAXLENGTH / 4];
            int strLength = 0;
            
            // If a second line number was already given with the command
            if (rest + restLength - end - 1 > 0) {
                endLine = (int) strtol(end, &end2, 10);
                // If given a valid number
                if (endLine != -1) {
                    // Make sure going from low to high (forwards)
                    if (endLine < line) {
                        int tmp = endLine;
                        endLine = line;
                        line = tmp;
                    }
                    
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
            editorState_findStringInFile(rest, restLength);
        } break;
        case 'F':
        {
            editorState_findStringInLine(rest, restLength);
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
                        return KEEP;
                    } break;
                    case 'p':
                    {
                        int current = currentBuffer - buffers;
                        int previous = current - 1;
                        if (previous < 0)
                            previous = buf_len(buffers) - 1;
                        
                        currentBuffer = &(buffers[previous]);
                        return KEEP;
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
                
                return KEEP;
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
            editorState_openAnotherFile(rest, restLength);
        } break;
        case 'n':
        {
            editorState_openNewFile(rest, restLength);
        } break;
        case 'e':
        {
            return EXIT;
        } break;
        case 'E':
        {
            return FORCE_EXIT;
        } break;
        case 'q':
        {
            return QUIT;
        } break;
        case 'Q':
        {
            return FORCE_QUIT;
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
    
    return KEEP;
}

internal void editorState_openAnotherFile(char *rest, int restLength) {
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
            printf("File doesn't exist... Creating it.\n\n");
            currentBuffer->modified = true;
            editorState_editor();
        } else {
            printFileInfo();
        }
        
        return;
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
        editorState_editor();
    } else printFileInfo();
}

internal void editorState_openNewFile(char *rest, int restLength) {
    {
        Buffer buffer;
        buffer_initEmptyBuffer(&buffer);
        buffer.modified = true;
        buf_push(buffers, buffer);
        currentBuffer = buf_end(buffers) - 1;
    }
    
    if (restLength - 1 > 0) {
        // Trim whitespace from beginning of rest
        while (*rest == ' ' || *rest == '\t') {
            ++rest;
            --restLength;
        }
        
        // Copy into openedFilename
        for (int i = 0; i < restLength; i++) {
            buf_push(currentBuffer->openedFilename, rest[i]);
        }
        
        //buf_push(currentBuffer->openedFilename, '\0');
    }
    
    printf("Opening a new file.\n");
#ifdef _WIN32
    printf("Press Ctrl-Z on new line to denote End Of Input\n\n");
#else
    printf("Press Ctrl-D on new line to denote End Of Input\n\n");
#endif
    
    assert(buf_len(currentBuffer->lines) == 0);
    
    editorState_editor();
}

internal int getLineNumber() {
    char *end;
    char *lineInput = NULL;
    int length = -1;
    int line = -1;
    
    printPrompt("Enter a line number: ");
    length = parsing_getLine_dynamic(&lineInput, true);
    line = (int) strtol(lineInput, &end, 10);
    
    while (line <= 0 || line > buf_len(currentBuffer->lines) || length == -1) {
        printError("That line number exceeds the bounds of the file.\n");
        printPrompt("Enter a line number: ");
        length = parsing_getLine_dynamic(&lineInput, true);
        line = (int) strtol(lineInput, &end, 10);
    }
    
    buf_free(lineInput);
    
    return line;
}

internal int checkLineNumber(int original_line) {
    char *end;
    char *lineInput = NULL;
    int length = 0;
    int line = original_line;
    
    while (line <= 0 || line > buf_len(currentBuffer->lines) || length == -1) {
        printError("That line number exceeds the bounds of the file.\n");
        printPrompt("Enter a line number: ");
        length = parsing_getLine_dynamic(&lineInput, true);
        line = (int) strtol(lineInput, &end, 10);
    }
    
    buf_free(lineInput);
    
    return line;
}

// Editor - will allow user to type in anything, showing line number at start of new lines. To exit the editor, press Ctrl-D on Linux or Ctrl-Z+Enter on Windows. As each new line is entered, the characters will be added to a char pointer streatchy buffer (dynamic array). Then, this line will be added to the streatchy buffer of lines (called 'lines').
void editorState_editor(void) {
    int c;
    int line = 1;
    
    // If continuing a previously typed-in file,
    // start on last line and overwrite the EOF character
    if (buf_len(currentBuffer->lines) > 0) {
        line = buf_len(currentBuffer->lines) + 1;
    }
    
    char *chars = NULL;
    
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
    
    // Set cursor to end of file
    currentBuffer->currentLine = buf_len(currentBuffer->lines);
}

// Insert lines after a specific line. Denote end of input by typing Ctrl-D (or Ctrl-Z+Enter on Windows) on new line.
internal void editorState_insertAfter(char *rest) {
    char *end;
    int line = (int) strtol(rest, &end, 10);
    
    if (line == 0) line = currentBuffer->currentLine;
    else line = checkLineNumber(line);
    
    char c;
    if (line - 1 >= 0 && line - 1 < buf_len(currentBuffer->lines))
        printLine(line - 1, 0, true);
    int currentLine = line + 1;
    
    Line *insertLines = NULL;
    char *chars = NULL;
    
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
    
    int firstMovedLine = buffer_insertAfterLine(currentBuffer, line, insertLines);
    
    // Free the old lines stretchy buffer
    buf_free(insertLines);
    
    // Show the line that was moved due to inserting before it (and after the line before it)
    if (firstMovedLine <= buf_len(currentBuffer->lines))
        printLine(firstMovedLine - 1, 'v', true);
    
    recreateOutline();
}

internal void editorState_insertBefore(char *rest) {
    char *end;
    int line = (int) strtol(rest, &end, 10);
    
    if (line == 0) line = currentBuffer->currentLine;
    else line = checkLineNumber(line);
    
    if (line == 0) { // Current Line is 0
        line = 1;
    }
    
    /*if (line == buf_len(currentBuffer->lines) + 1) {
    currentBuffer->currentLine = buf_len(currentBuffer->lines);
    editorState_insertAfter(rest);
    return;
    }*/
    
    char c;
    if (line - 2 >= 0 && line - 1 < buf_len(currentBuffer->lines))
        printLine(line - 2, 0, true);
    int currentLine = line;
    
    Line *insertLines = NULL;
    char *chars = NULL;
    
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
    
    int firstMovedLine = buffer_insertBeforeLine(currentBuffer, line, insertLines);
    
    // Free the old lines stretchy buffer
    buf_free(insertLines);
    
    // Show the line that was moved due to the insertion before it
    if (firstMovedLine <= buf_len(currentBuffer->lines))
        printLine(firstMovedLine - 1, 'v', true);
    
    recreateOutline();
}

internal void editorState_appendTo(char *rest) {
    char *end;
    int line = (int) strtol(rest, &end, 10);
    
    if (line == 0) line = currentBuffer->currentLine;
    else line = checkLineNumber(line);
    
    char c;
    char *chars = NULL;
    if (line - 2 >= 0 && line - 2 < buf_len(currentBuffer->lines))
        printLine(line - 2, 0, true);
    
    printLine(line - 1, 'A', false);
    int canceled = false;
    chars = getInput(&canceled);
    if (canceled) {
        // Delete the chars buffer
        buf_free(chars);
        // Cancel the operation by returning
        return;
    }
    
    buffer_appendToLine(currentBuffer, line, chars);
    buf_free(chars);
    
    recreateOutline();
}

internal void editorState_prependTo(char *rest) {
    char *end;
    int line = (int) strtol(rest, &end, 10);
    
    if (line == 0) line = currentBuffer->currentLine;
    else line = checkLineNumber(line);
    
    char c;
    if (line - 2 >= 0 && line - 2 < buf_len(currentBuffer->lines))
        printLine(line - 2, 0, true);
    
    char *chars = NULL; // The new char stretchy buffer
    
    printLine(line - 1, 'I', true);
    printPrompt("%4s ^- ", "");
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
    
    buffer_prependToLine(currentBuffer, line, chars);
    
    recreateOutline();
}

internal void editorState_replaceLine(char *rest) {
    char *end;
    int line = (int) strtol(rest, &end, 10);
    
    if (line == 0) line = currentBuffer->currentLine;
    else line = checkLineNumber(line);
    
    char c;
    if (line - 2 >= 0 && line - 2 < buf_len(currentBuffer->lines))
        printLine(line - 2, 0, true);
    
    char *chars = NULL; // The new char stretchy buffer
    
    printLine(line - 1, 'r', true);
    printf("%5s ", "");
    int canceled = false;
    chars = getInput(&canceled);
    if (canceled) {
        // Delete the chars buffer
        buf_free(chars);
        // Cancel the operation by returning
        return;
    }
    
    buffer_replaceLine(currentBuffer, line, chars);
    
    recreateOutline();
}

internal void editorState_replaceString(char *rest, int restLength) {
    char *end;
    int line = (int) strtol(rest, &end, 10);
    
    if (line == 0) {
        line = currentBuffer->currentLine;
    } else {
        line = checkLineNumber(line);
        // Don't include the space in between the line number and the string to replace
        ++end;
    }
    
    char str[MAXLENGTH / 4];
    int strLength = 0;
    int exists = false;
    
    // If a string was already given with the command
    if (rest + restLength - end - 1 > 0) {
        // Copy into str
        strLength = rest + restLength - end;
        strncpy(str, end, strLength);
        // Use it instead of asking for a string to replace
        exists = true;
    }
    
    if (!exists) {
        printPrompt("Enter the string to replace: ");
        strLength = parsing_getLine(str, MAXLENGTH / 4, false);
        while (strLength == -1) {
            printPrompt("Enter the string to replace: ");
            strLength = parsing_getLine(str, MAXLENGTH / 4, true);
        }
    }
    
    char c;
    
    int index = buffer_findStringInLine(currentBuffer, line, str, strLength - 1);
    
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
    
    buffer_replaceInLine(currentBuffer, line, index, index + strLength - 2, chars);
    
    recreateOutline();
}

// Finds the first occrance of the string in the given line
// Displays the line with an arrow pointing to the occurance
// Will also show the line before it to give context and the column of the start of the occurance
internal void editorState_findStringInLine(char *rest, int restLength) {
    char *end;
    int line = (int) strtol(rest, &end, 10);
    
    if (line == 0) {
        line = currentBuffer->currentLine;
    } else {
        line = checkLineNumber(line);
        // Don't include the space in between the line number and the string to replace
        ++end;
    }
    
    
    char str[MAXLENGTH / 4];
    int strLength = 0;
    bool exists = false;
    
    // If a string was already given with the command
    if (rest + restLength - end - 1 > 0) {
        // Copy into str
        strLength = rest + restLength - end;
        strncpy(str, end, strLength);
        // Use it instead of asking for a string to replace
        exists = true;
    }
    
    if (!exists) {
        printPrompt("Enter the string to find: ");
        strLength = parsing_getLine(str, MAXLENGTH / 4, false);
        while (strLength == -1) {
            printPrompt("Enter the string to find: ");
            strLength = parsing_getLine(str, MAXLENGTH / 4, true);
        }
    }
    
    int index = buffer_findStringInLine(currentBuffer, line, str, strLength - 1);
    
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
internal void editorState_findStringInFile(char *rest, int restLength) {
    char str[MAXLENGTH / 4];
    int strLength = 0;
    bool exists = false;
    
    // If a string was already given with the command
    if (restLength - 1 > 0) {
        // Copy into str
        strLength = restLength;
        strncpy(str, rest, strLength);
        exists = true;
    }
    
    if (!exists) {
        printPrompt("Enter the string to find: ");
        strLength = parsing_getLine(str, MAXLENGTH / 4, false);
        while (strLength == -1) {
            printPrompt("Enter the string to find: ");
            strLength = parsing_getLine(str, MAXLENGTH / 4, true);
        }
    }
    
    
    int colIndex = -1;
    int foundIndex = buffer_findStringInFile(currentBuffer, str, strLength - 1, &colIndex);
    
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

internal void editorState_deleteLine(char *rest) {
    char *end;
    int line = (int) strtol(rest, &end, 10);
    
    if (line == 0) line = currentBuffer->currentLine;
    else line = checkLineNumber(line);
    
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
    
    recreateOutline();
}

internal void editorState_moveUp(char *rest) {
    char *end;
    int line = (int) strtol(rest, &end, 10);
    
    if (line == 0) line = currentBuffer->currentLine;
    else line = checkLineNumber(line);
    
    // Show the line before the line being moved up to
    if (line - 2 > 0)
        printLine(line - 3, 0, true);
    
    buffer_moveLineUp(currentBuffer, line);
    
    // Print the new position of the line that was moved and the old line that was moved down
    printLine(line - 2, '^', true);
    printLine(line - 1, 'v', true);
    
    // Print the next line to give context
    printLine(line, 0, true);
    
    recreateOutline();
}

internal void editorState_moveDown(char *rest) {
    char *end;
    int line = (int) strtol(rest, &end, 10);
    
    if (line == 0) line = currentBuffer->currentLine;
    else line = checkLineNumber(line);
    
    // Show the line before the line being moved down
    if (line - 1 > 0)
        printLine(line - 2, 0, true);
    
    buffer_moveLineDown(currentBuffer, line);
    
    // Print the new position of the line that was moved and the old line that was moved up
    printLine(line - 1, '^', true);
    printLine(line, 'v', true);
    
    // Print the next line to give context
    printLine(line + 1, 0, true);
    
    recreateOutline();
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
    printPrompt("\n<%d: %s|preview> ", currentBuffer - buffers, currentBuffer->openedFilename);
    
    bool forward = true;
    while ((c = getch()) != EOF && offset < buf_len(currentBuffer->lines))
    {
        if (c == '?') {
            // Print help info about preview command here
            printf("\nPreviewing '%s'\n", currentBuffer->openedFilename);
            printf(" * 'q' or Ctrl-X to stop previewing\n");
            printf(" * 'Q' to exit the whole program\n");
            printf(" * Enter/'n' to show the next lines\n");
            printf(" * 'p' to show previous lines");
            printf(" * 'j' start showing lines from given line number (TODO)"); // TODO
            printf(" * 'f' find first occurance of string in file and jump there (TODO)"); // TODO
            
            printPrompt("\n<%d: %s|preview> ", currentBuffer - buffers, currentBuffer->openedFilename);
            continue;
        } else if (c == 'p') {
            if (forward) {
                printf("\r");
                for (int i = 0; i < 45; i++) { // TODO: Hacky
                    printf(" ");
                }
                printf("\r");
                printf("--^-^-^-^-^--\n\n");
            }
            offset = offset - (linesAtATime * 2) - 2;
            if (offset < 0) offset = 0;
            forward = false;
        } else if (c == 'q' || c == 24) { // 26 is Ctrl-X, aka CANCEL
            break;
        } else if (c == 'Q') {
            exit(0);
        } else {
            if (!forward) {
                printf("\r");
                for (int i = 0; i < 45; i++) { // TODO: Hacky
                    printf(" ");
                }
                printf("\r");
                printf("--v-v-v-v-v--\n\n");
            }
            forward = true;
        }
        
        printf("\r");
        for (int i = 0; i < 45; i++) { // TODO: Hacky
            printf(" ");
        }
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
        
        offset = offset + linesAtATime + 1;
        if (offset >= buf_len(currentBuffer->lines)) {
            break;
        }
        printPrompt("\n<%d: %s|preview> ", currentBuffer - buffers, currentBuffer->openedFilename);
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
    printf("Filetype: %d\n", currentBuffer->fileType); // TODO: Print actual string of filetype
    
    int numOfLines = buf_len(currentBuffer->lines);
    if (numOfLines != 0) {
        // If last character of last line ends with a new line, add one to the number of lines
        Line lastLine = currentBuffer->lines[buf_len(currentBuffer->lines) - 1];
        char lastChar = lastLine.chars[buf_len(lastLine.chars) - 1];
        if (lastChar == '\n') {
            numOfLines++;
        }
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
