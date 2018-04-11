#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <stdlib.h>

#include "lineeditor.h"

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
            buffer_initEmptyBuffer(&currentBuffer);
            currentBuffer.modified = true;
            
            if (argsLength > 1) {
                int i = 0;
                while (args[i] != ' ' && args[i] != '\n' && args[i] != '\0') {
                    buf_push(currentBuffer.openedFilename, args[i]);
                    ++i;
                }
            }
            
            printf("Opening a new file.\n");
            printf("Press Ctrl-D (or Ctrl-Z on Windows) on new line to denote End Of Input\n\n");
            
            // Make sure currently stored text has been cleared out.
            assert(buf_len(currentBuffer.lines) == 0);
            
            subState = ED_EDITOR;
            subState = editorState_editor();
            if (subState == ED_KEEP) subState = subStatePrev;
        } break;
        case ED_OPEN:
        {
            buffer_initEmptyBuffer(&currentBuffer);
            
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
                if (!buffer_openFile(&currentBuffer, filename)) {
                    printf("File doesn't exist... Creating it.\n\n");
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
                if (!buffer_openFile(&currentBuffer, filename)) {
                    printf("File doesn't exist... Creating it.\n\n");
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
        case ED_EXIT:
        {
            subState = ED_EDITOR;
            
            // Clear openedFilename and the file information
            buf_free(currentBuffer.openedFilename);
            
            for (int i = 0; i < buf_len(currentBuffer.lines); i++) {
                buf_free(currentBuffer.lines[i].chars);
            }
            
            buf_free(currentBuffer.lines);
            
            initialSet = 0;
            return MAIN_MENU;
        } break;
        case ED_QUIT:
        {
            return QUIT;
        } break;
        default:
        printError("Unknown command");
    }
    
    return KEEP;
}

/* Menu for Editor */
EditorState editorState_menu(void) {
    /* Prompt */
    if (buf_len(currentBuffer.openedFilename) > 0) {
        // TODO: This will also print out the directory, so I should get rid of everything before the last slash
        if (currentBuffer.modified) {
            printPrompt("\n<%d|%.*s*> ", currentBuffer.currentLine, (int) buf_len(currentBuffer.openedFilename), currentBuffer.openedFilename);
        } else {
            printPrompt("\n<%d|%.*s> ", currentBuffer.currentLine, (int) buf_len(currentBuffer.openedFilename), currentBuffer.openedFilename);
        }
        
    } else printPrompt("\n<new file> ");
    
    /* get first character - the menu item */
    char c;
    c = getchar();
    
    if (c == '\n') return KEEP;
    
    /* Store rest of line in rest */
    char rest[MAXLENGTH / 4];
    int restLength = parsing_getLine(rest, MAXLENGTH / 4, true);
    
    printf("\n");
    
    switch (c) {
        case '?': // TODO: Add new file and open file.
        {
            if (buf_len(currentBuffer.openedFilename) > 0)
                printf("'%s' is currently open.\n\n", currentBuffer.openedFilename);
            else printf("An unnamed file is currently open.\n\n");
            
            printf("Use Ctrl-D or Ctrl-Z+Enter to denote end of input\n");
            printf("Use Ctrl-X+Enter to cancel the current command/operation\n");
            printf("\n");
            printf(" * 's' - Save\n");
            /* Edit - rewrite a specific line, group of lines, group of characters in a line (given column numbers), and word/group of words */
            //printf(" * 'e' - Edit\n");
            printf(" * '#' - Gives back information on the file, including number of lines, filename, number of characters, filetype, etc.\n");
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
            printf(" * 'u' - Undo the last operation, cannot undo an undo, cannot undo past 1 operation\n"); // TODO
            printf(" * 'c' - Continue from last line\n");
            printf(" * 'p (line#:start)' - Preview whole file (optionally starting at given line)\n");
            printf(" * 'P (line#:start) (line#:end)' - Preview a line or set of lines, including the line before and after\n");
            printf(" * 'e / E' - Save and Exit / Exit (without save)\n");
            printf(" * 'q / Q' - Save and Quit / Quit (without save)\n");
        } break;
        case 's':
        {
            if (!currentBuffer.openedFilename && buf_len(currentBuffer.openedFilename) <= 0) {
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
                buffer_saveFile(&currentBuffer, filename);
            } else {
                printf("Saving '%s'", currentBuffer.openedFilename);
                buffer_saveFile(&currentBuffer, currentBuffer.openedFilename);
            }
        } break;
        case '#':
        {
            printFileInfo();
        } break;
        case 'c':
        {
            editorState_insertAfter(buf_len(currentBuffer.lines));
            recreateOutline();
        } break;
        case 'a':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            char lineInput[MAXLENGTH / 4];
            int length;
            while (line < 0 || line > buf_len(currentBuffer.lines) || length == -1) {
                if (rest != end)
                    printError("That line number exceeds the bounds of the file.\n");
                printPrompt("Enter a line number: ");
                length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                line = (int) strtol(lineInput, &end, 10);
            }
            
            if (line == 0)
                editorState_insertBefore(1);
            else editorState_insertAfter(line);
            recreateOutline();
        } break;
        case 'i':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            char lineInput[MAXLENGTH / 4];
            int length;
            while (line <= 0 || line > buf_len(currentBuffer.lines) + 1 || length == -1) {
                if (rest != end)
                    printError("That line number exceeds the bounds of the file.\n");
                printPrompt("Enter a line number: ");
                length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                line = (int) strtol(lineInput, &end, 10);
            }
            
            if (line == buf_len(currentBuffer.lines) + 1)
                editorState_insertAfter(buf_len(currentBuffer.lines));
            else editorState_insertBefore(line);
            recreateOutline();
        } break;
        case 'A':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            char lineInput[MAXLENGTH / 4];
            int length;
            while (line <= 0 || line > buf_len(currentBuffer.lines) || length == -1) {
                if (rest != end)
                    printError("That line number exceeds the bounds of the file.\n");
                printPrompt("Enter a line number: ");
                length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                line = (int) strtol(lineInput, &end, 10);
            }
            
            editorState_appendTo(line);
            recreateOutline();
        } break;
        case 'I':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            char lineInput[MAXLENGTH / 4];
            int length;
            while (line <= 0 || line > buf_len(currentBuffer.lines) || length == -1) {
                if (rest != end)
                    printError("That line number exceeds the bounds of the file.\n");
                printPrompt("Enter a line number: ");
                length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                line = (int) strtol(lineInput, &end, 10);
            }
            
            editorState_prependTo(line);
            recreateOutline();
        } break;
        case 'r':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            char lineInput[MAXLENGTH / 4];
            int length;
            while (line <= 0 || line > buf_len(currentBuffer.lines) || length == -1) {
                if (rest != end)
                    printError("That line number exceeds the bounds of the file.\n");
                printPrompt("Enter a line number: ");
                length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                line = (int) strtol(lineInput, &end, 10);
            }
            
            editorState_replaceLine(line);
            recreateOutline();
        } break;
        case 'R':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            ++end; // Don't include the space in between the line number and the string to replace
            
            char lineInput[MAXLENGTH / 4];
            int length;
            while (line <= 0 || line > buf_len(currentBuffer.lines) || length == -1) {
                if (rest != end)
                    printError("That line number exceeds the bounds of the file.\n");
                printPrompt("Enter a line number: ");
                length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                line = (int) strtol(lineInput, &end, 10);
            }
            
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
            
            char lineInput[MAXLENGTH / 4];
            int length;
            while (line == 0 || line > buf_len(currentBuffer.lines) || length == -1) {
                if (rest != end)
                    printError("That line number exceeds the bounds of the file.\n");
                printPrompt("Enter a line number: ");
                length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                line = (int) strtol(lineInput, &end, 10);
            }
            
            editorState_deleteLine(line);
            recreateOutline();
        } break;
        case 'm':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            char lineInput[MAXLENGTH / 4];
            int length;
            while (line == 0 || line > buf_len(currentBuffer.lines) || length == -1) {
                if (rest != end)
                    printError("That line number exceeds the bounds of the file.\n");
                printPrompt("Enter a line number: ");
                length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                line = (int) strtol(lineInput, &end, 10);
            }
            
            editorState_moveUp(line);
            recreateOutline();
        } break;
        case 'M':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            char lineInput[MAXLENGTH / 4];
            int length;
            while (line == 0 || line > buf_len(currentBuffer.lines) || length == -1) {
                if (rest != end)
                    printError("That line number exceeds the bounds of the file.\n");
                printPrompt("Enter a line number: ");
                length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                line = (int) strtol(lineInput, &end, 10);
            }
            
            editorState_moveDown(line);
            recreateOutline();
        } break;
        case 'p':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            if (line <= 0 || line > buf_len(currentBuffer.lines))
                printText(0);
            else printText(line - 1);
        } break;
        case 'P':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            ++end; // Don't include the space in between the first line number and the second line number
            
            char lineInput[MAXLENGTH / 4];
            int length;
            while (line == 0 || line > buf_len(currentBuffer.lines) || length == -1) {
                if (rest != end)
                    printError("That line number exceeds the bounds of the file.\n");
                printPrompt("Enter a line number: ");
                length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                line = (int) strtol(lineInput, &end, 10);
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
                    // Print the range of lines along with the line before and after
                    if (line - 2 >= 0)
                        printLine(line - 2, 0, true);
                    for (int i = line; i <= endLine; i++) {
                        if (i - 1 >= 0 && i - 1 < buf_len(currentBuffer.lines))
                            printLine(i - 1, 0, true);
                    }
                    if (endLine < buf_len(currentBuffer.lines))
                        printLine(endLine, 0, true);
                    break;
                }
            }
            
            // Otherwise, just print that one line
            if (line - 2 >= 0)
                printLine(line - 2, 0, true);
            printLine(line - 1, 0, true);
            if (line < buf_len(currentBuffer.lines))
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
        case 'e':
        {
            if (!currentBuffer.openedFilename && buf_len(currentBuffer.openedFilename) <= 0) {
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
                buffer_saveFile(&currentBuffer, filename);
            } else {
                printf("Saving '%s'", currentBuffer.openedFilename);
                buffer_saveFile(&currentBuffer, currentBuffer.openedFilename);
            }
            
            return ED_EXIT;
        } break;
        case 'E':
        {
            return ED_EXIT;
        } break;
        case 'q':
        {
            
            if (!currentBuffer.openedFilename && buf_len(currentBuffer.openedFilename) <= 0) {
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
                buffer_saveFile(&currentBuffer, filename);
            } else {
                printf("Saving '%s'", currentBuffer.openedFilename);
                buffer_saveFile(&currentBuffer, currentBuffer.openedFilename);
            }
            
            return ED_QUIT;
        } break;
        case 'Q':
        {
            return ED_QUIT;
        } break;
        // Hacked in - change filetype to FT_C because of how poor my filetype extension matching is
        case 't':
        {
            currentBuffer.fileType = FT_C;
            recreateOutline();
        } break;
        case 'T':
        {
            printf("FileType: %d\n", currentBuffer.fileType);
        } break;
        default:
        {
            printError("Unknown command");
        } break;
    }
    
    return ED_KEEP;
}

// Editor - will allow user to type in anything, showing line number at start of new lines. To exit the editor, press Ctrl-D on Linux or Ctrl-Z+Enter on Windows. As each new line is entered, the characters will be added to a char pointer streatchy buffer (dynamic array). Then, this line will be added to the streatchy buffer of lines (called 'lines').
EditorState editorState_editor(void) {
    char c;
    int line = 1;
    
    // If continuing a previously typed-in file,
    // start on last line and overwrite the EOF character
    if (buf_len(currentBuffer.lines) > 0) {
        line = buf_len(currentBuffer.lines) + 1;
    }
    
    // Show the previous line to give context.
    printLine(line - 2, 0, true);
    
    char *chars = NULL;
    
    printLineNumber("%5d ", line);
    while ((c = getchar()) != EOF) {
        buf_push(chars, c);
        if (c == '\n') {
            buf_push(currentBuffer.lines, ((Line) { chars }));
            ++line;
            printLineNumber("%5d ", line);
            chars = NULL; // Create new char stretchy buffer for next line
        }
    }
    
    // Set cursor to end of file
    currentBuffer.currentLine = buf_len(currentBuffer.lines);
    
    return ED_MENU;
}

// Insert lines after a specific line. Denote end of input by typing Ctrl-D (or Ctrl-Z+Enter on Windows) on new line.
internal void editorState_insertAfter(int line) {
    char c;
    if (line - 1 >= 0 && line - 1 < buf_len(currentBuffer.lines))
        printLine(line - 1, 0, true);
    int currentLine = line + 1;
    
    Line *insertLines = NULL;
    char *chars = NULL;
    
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
    
    int firstMovedLine = buffer_insertAfterLine(&currentBuffer, line, insertLines);
    
    // Free the old lines stretchy buffer
    buf_free(insertLines);
    
    // Show the line that was moved due to inserting before it (and after the line before it)
    if (firstMovedLine <= buf_len(currentBuffer.lines))
        printLine(firstMovedLine - 1, 'v', true);
}

internal void editorState_insertBefore(int line) {
    char c;
    if (line - 2 >= 0 && line - 1 < buf_len(currentBuffer.lines))
        printLine(line - 2, 0, true);
    int currentLine = line;
    
    Line *insertLines = NULL;
    char *chars = NULL;
    
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
    
    int firstMovedLine = buffer_insertBeforeLine(&currentBuffer, line, insertLines);
    
    // Free the old lines stretchy buffer
    buf_free(insertLines);
    
    // Show the line that was moved due to the insertion before it
    if (firstMovedLine <= buf_len(currentBuffer.lines))
        printLine(firstMovedLine - 1, 'v', true);
}

internal void editorState_appendTo(int line) {
    char c;
    char *chars = NULL;
    if (line - 2 >= 0 && line - 2 < buf_len(currentBuffer.lines))
        printLine(line - 2, 0, true);
    
    // Remove the new line character from the line
    //buf_pop(currentBuffer.lines[line - 1].chars);
    
    //int count = 0;
    printLine(line - 1, 'A', false);
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
    
    buffer_appendToLine(&currentBuffer, line, chars);
    buf_free(chars);
}

internal void editorState_prependTo(int line) {
    char c;
    if (line - 2 >= 0 && line - 2 < buf_len(currentBuffer.lines))
        printLine(line - 2, 0, true);
    
    char *chars = NULL; // The new char stretchy buffer
    
    printLine(line - 1, 'I', true);
    printPrompt("%4s ^- ", "");
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
    
    buffer_prependToLine(&currentBuffer, line, chars);
}

internal void editorState_replaceLine(int line) {
    char c;
    if (line - 2 >= 0 && line - 2 < buf_len(currentBuffer.lines))
        printLine(line - 2, 0, true);
    
    char *chars = NULL; // The new char stretchy buffer
    
    printLine(line - 1, 'r', true);
    printf("%5s ", "");
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
    
    buffer_replaceLine(&currentBuffer, line, chars);
}

internal void editorState_replaceString(int line, char *str, int strLength) {
    char c;
    
    // TODO: Use buffer_findStringInLine
    // Find the first occurance of the string in the line, -1 for no occurance
    int index = -1;
    int ii = 0;
    for (int i = 0; i < buf_len(currentBuffer.lines[line - 1].chars); i++) {
        if (currentBuffer.lines[line - 1].chars[i] == str[ii]) {
            if (ii == 0)
                index = i;
            ++ii;
        } else {
            ii = 0;
            index = -1;
        }
        
        // If string ends in a new line or 0 termination, subtract one from the string length so we don't match them
        if (str[strLength - 1] == '\0' || str[strLength - 1] == '\n') {
            if (ii == strLength - 1) break;
        } else {
            if (ii == strLength) break;
        }
    }
    
    if (index == -1) {
        printError("No occurance of '%.*s' found\n", strLength, str);
        return;
    }
    
    // Print the previous line to give context
    if (line - 2 >= 0 && line - 2 < buf_len(currentBuffer.lines))
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
    
    buffer_replaceInLine(&currentBuffer, line, index, index + strLength - 1, chars);
}

// Finds the first occrance of the string in the given line
// Displays the line with an arrow pointing to the occurance
// Will also show the line before it to give context and the column of the start of the occurance
internal void editorState_findStringInLine(int line, char *str, int strLength) { // TODO: Low priority - less useful than finding in file
    printError("Unimplemented");
}

// Finds the first occurance of the string in the file
// Displays the line it is on with an arrow pointing to the occurance
// Will also show the line before it to give context
internal void editorState_findStringInFile(char *str, int strLength) {
    // The line number the string match was found on, -1 for no occurance
    int foundIndex = -1;
    // The column index the occurance starts on the line
    int colIndex = -1;
    
    // For each line in file
    for (int line = 0; line < buf_len(currentBuffer.lines); line++) {
        // Find the first occurance of the string in current line, -1 for no occurance
        int index = -1; // Column index
        int ii = 0;
        for (int i = 0; i < buf_len(currentBuffer.lines[line].chars); i++) {
            if (currentBuffer.lines[line].chars[i] == str[ii]) {
                if (ii == 0)
                    index = i;
                ++ii;
            } else {
                ii = 0;
                index = -1;
            }
            
            // If string ends in a new line or 0 termination, subtract one from the string length so we don't match them
            if (str[strLength - 1] == '\0' || str[strLength - 1] == '\n') {
                if (ii == strLength - 1) break;
            } else {
                if (ii == strLength) break;
            }
        }
        
        // If found, break out of for loop, otherwise, keep searching
        if (index != -1) {
            foundIndex = line;
            colIndex = index;
            break;
        }
    }
    
    // If no occurance found in file
    if (foundIndex == -1) {
        printError("No occurance of '%.*s' found\n", strLength, str);
        return;
    }
    
    // Print the previous line to give context
    if (foundIndex - 1 >= 0 && foundIndex - 1 < buf_len(currentBuffer.lines))
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
    // Show the line before the line that's being deleted
    if (line - 1 > 0)
        printLine(line - 2, 0, true);
    
    if (line == buf_len(currentBuffer.lines)) { // TODO: This isn't working correctly
        printLine(line - 1, 'x', true);
        buffer_deleteLine(&currentBuffer, buf_len(currentBuffer.lines));
        
        // Show the first line that was moved - the line # should be the same as the line that was deleted
        if (line <= buf_len(currentBuffer.lines))
            printLine(line - 1, '^', true);
        return;
    }
    
    printLine(line - 1, 'x', true);
    
    buffer_deleteLine(&currentBuffer, line);
    
    // Show the first line that was moved - the line # should be the same as the line that was deleted
    if (line <= buf_len(currentBuffer.lines))
        printLine(line - 1, '^', true);
}

internal void editorState_moveUp(int line) {
    // Show the line before the line being moved up to
    if (line - 2 > 0)
        printLine(line - 3, 0, true);
    
    buffer_moveLineUp(&currentBuffer, line);
    
    // Print the new position of the line that was moved and the old line that was moved down
    printLine(line - 2, '^', true);
    printLine(line - 1, 'v', true);
    
    // Print the next line to give context
    printLine(line, 0, true);
}

internal void editorState_moveDown(int line) {
    // Show the line before the line being moved down
    if (line - 1 > 0)
        printLine(line - 2, 0, true);
    
    buffer_moveLineDown(&currentBuffer, line);
    
    // Print the new position of the line that was moved and the old line that was moved up
    printLine(line - 1, '^', true);
    printLine(line, 'v', true);
    
    // Print the next line to give context
    printLine(line + 1, 0, true);
}

/* Print the currently stored text with line numbers */
void printText(int startLine) {
    if (buf_len(currentBuffer.lines) <= 0) {
        printLineNumber("%5d ", 1);
        printf("\n");
        return;
    }
    
    int linesAtATime = 10; // TODO: Should have a setting for this.
    int offset = startLine;
    char c;
    
    for (int line = offset; line < linesAtATime + offset + 1 && line <= buf_len(currentBuffer.lines); line++) {
        if (line == buf_len(currentBuffer.lines)) {
            if (line + 1 == currentBuffer.currentLine)
                printLineNumber("%c%4d ", '*', line + 1);
            else printLineNumber("%5d ", line + 1);
            break;
        }
        printLine(line, 0, true);
    }
    offset = linesAtATime + offset + 1;
    if (offset >= buf_len(currentBuffer.lines)) {
        printf("\n");
        return;
    }
    printPrompt("\n<%s | preview> ", currentBuffer.openedFilename);
    
    while ((c = getchar()) != EOF && offset < buf_len(currentBuffer.lines)) {
        if (c == '?') {
            // Print help info about preview command here
            printf("\nPreviewing '%s'\n", currentBuffer.openedFilename);
            printf(" * 'q' or Ctrl-X to stop previewing\n");
            printf(" * 'Q' to exit the whole program\n");
            printf(" * Enter to show the next lines\n");
            // Discard the enter key
            getchar();
            printPrompt("\n<%s | preview> ", currentBuffer.openedFilename);
            continue;
        } else if (c == 'q' || c == 24) { // 26 is Ctrl-X, aka CANCEL
            // Discard the enter key
            getchar();
            break;
        } else if (c == 'Q') {
            exit(0);
        }
        
        printf("\n");
        for (int line = offset; line < linesAtATime + offset + 1 && line <= buf_len(currentBuffer.lines); line++) {
            if (line == buf_len(currentBuffer.lines)) {
                if (line + 1 == currentBuffer.currentLine)
                    printLineNumber("%c%4d ", '*', line + 1);
                else printLineNumber("%5d ", line + 1);
                break;
            }
            printLine(line, 0, true);
        }
        
        offset = linesAtATime + offset + 1;
        if (offset >= buf_len(currentBuffer.lines)) {
            break;
        }
        printPrompt("\n<%s | preview> ", currentBuffer.openedFilename);
    }
    
    printf("\n");
}

/*
Prints one line of text given the line number. Note that the line numbers start at 0 (although they are displayed to the user starting at 1).
Pass false into printNewLine so the new line at the end is not printed
*/
void printLine(int line, char operation, int printNewLine) {
    //printf("Current Line: %d\n", currentBuffer.currentLine);
    if (buf_len(currentBuffer.lines) <= 0 && line == 0) {
        if (operation != 0)
            printLineNumber("%c%4d ", operation, 1);
        else {
            if (line + 1 == currentBuffer.currentLine)
                printLineNumber("%c%4d ", '*', 1);
            else printLineNumber("%5d ", 1);
        }
        printf("\n");
        return;
    }
    
    // If line is last line in file (one above the length of the lines)
    if (line == buf_len(currentBuffer.lines)) {
        if (operation != 0)
            printLineNumber("%c%4d ", operation, line + 1);
        else {
            if (line + 1 == currentBuffer.currentLine)
                printLineNumber("%c%4d ", '*', line + 1);
            else printLineNumber("%5d ", line + 1);
        }
        return;
    } else if (line > buf_len(currentBuffer.lines)) {
        // Error!
        return;
    }
    
    if (operation != 0)
        printLineNumber("%c%4d ", operation, line + 1);
    else {
        if (line + 1 == currentBuffer.currentLine)
            printLineNumber("%c%4d ", '*', line + 1);
        else printLineNumber("%5d ", line + 1);
    }
    
    int length = buf_len(currentBuffer.lines[line].chars);
    // If shouldn't print new line and end of line is a new line, subtract it off from the length
    if (!printNewLine && currentBuffer.lines[line].chars[length - 1] == '\n')
        --length;
    printf("%.*s", length, currentBuffer.lines[line].chars);
    /*for (int i = 0; i < buf_len(currentBuffer.lines[line].chars); i++) {
    putchar(currentBuffer.lines[line].chars[i]);
    }*/
}

// TODO: number of chars, filetype, syntax highlighting enabled, outline enabled
void printFileInfo(void) {
    printf("File Information for '%s'\n", currentBuffer.openedFilename);
    
    int numOfLines = buf_len(currentBuffer.lines);
    // If last character of last line ends with a new line, add one to the number of lines
    Line lastLine = currentBuffer.lines[buf_len(currentBuffer.lines) - 1];
    char lastChar = lastLine.chars[buf_len(lastLine.chars) - 1];
    if (lastChar == '\n') {
        numOfLines++;
    }
    printf("Number of Lines: %d\n", numOfLines);
    
    // If markdown file, print outline
    if (currentBuffer.fileType == FT_MARKDOWN) {
        if (buf_len(currentBuffer.outline.markdown_nodes) > 0)
            printf("Outline:\n");
    } else if (currentBuffer.fileType == FT_C) {
        if (buf_len(currentBuffer.outline.c_nodes) > 0)
            printf("Outline:\n");
    }
    showOutline();
}
