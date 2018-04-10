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
internal void editorState_save(void);
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
                buffer_openFile(&currentBuffer, filename);
                //openFile(filename);
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
                buffer_openFile(&currentBuffer, filename);
                //openFile(filename);
                free(filename);
                filename = 0;
            }
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

void openFile(char *filename)
{
    // Open the file, then
    // Take all characters from file and put into lines streatchy buffer each Line with a chars streatchy buffer.
    char c;
    int line = 1;
    
    FILE *fp;
    fp = fopen(filename, "r");
    
    // Find last occurance of '.' in filename
    int dotIndex;
    for (int i = 0; i < strlen(filename); i++) {
        if (filename[i] == '.') {
            dotIndex = i;
        }
    }
    
    // Determine filetype based on extension
    // TODO: Doesn't handle CPP files yet.
    FileType ft;
    switch (filename[dotIndex + 1])
    {
        case 't':
        ft = FT_TEXT;
        break;
        case 'm':
        ft = FT_MARKDOWN;
        break;
        case 'c':
        ft = FT_C;
        break;
        case 'h':
        ft = FT_C_HEADER;
        break;
        default:
        ft = FT_UNKNOWN;
        break;
    }
    if (ft != FT_UNKNOWN) {
        int isType = true;
        char *ftExt;
        getFileTypeExtension(ft, &ftExt);
        for (int i = dotIndex + 1; i < strlen(filename); i++) {
            if (filename[i] != ftExt[i - dotIndex - 1]) {
                isType = false;
                break;
            }
        }
        
        if (isType) {
            currentBuffer.fileType = ft;
        } else {
            currentBuffer.fileType = FT_UNKNOWN;
        }
        free(ftExt);
    }
    
    printf("Opening file '%s'.\n", filename);
    
    for (int i = 0; i < strlen(filename) + 1; i++) {
        buf_push(currentBuffer.openedFilename, filename[i]);
    }
    
    // Make sure the filename ends with '\0'
    assert(currentBuffer.openedFilename[buf_len(currentBuffer.openedFilename) - 1] == '\0');
    char *chars = NULL;
    
    while ((c = fgetc(fp)) != EOF) {
        buf_push(chars, c);
        if (c == '\n') {
            buf_push(currentBuffer.lines, ((Line) { chars }));
            ++line;
            chars = NULL; // Create new char Stretchy Buffer for next line
        }
    }
    
    // Create the outline
    createOutline();
    
    //printText();
    printFileInfo();
    
    fclose(fp);
}

/* Menu for Editor */
EditorState editorState_menu(void) {
    /* Prompt */
    if (buf_len(currentBuffer.openedFilename) > 0) {
        // TODO: This will also print out the directory, so I should get rid of everything before the last slash
        printPrompt("\n<%.*s> ", (int) buf_len(currentBuffer.openedFilename), currentBuffer.openedFilename);
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
            printf(" * '#' - Gives back information on the file, including number of lines, filename, number of characters, filetype, etc.");
            printf(" * 'a (line#)' - Insert after the line number\n");
            printf(" * 'i (line#)' - Insert before the line number\n");
            printf(" * 'A (line#)' - Appends to a line\n");
            printf(" * 'I (line#)' - Prepends to a line\n");
            printf(" * 'r (line#)' - Replace a line with a new line\n");
            printf(" * 'R (line#) (string)' - Replace the first occurance of the string in the line\n");
            printf(" * 'x (line#)' - Deletes a line\n");
            printf(" * 'm (line#)' - Move the line up by one");
            printf(" * 'M (line#)' - Move the line down by one");
            printf(" * 'f (string)' - Finds the first occurance of the string in the file and prints the line it's on out\n");
            printf(" * 'u' - Undo the last operation, cannot undo an undo, cannot undo past 1 operation"); // TODO
            printf(" * 'c' - Continue from last line\n");
            printf(" * 'p (line#:start)' - Preview whole file (optionally starting at given line)\n");
            printf(" * 'P (line#:start) (line#:end)' - Preview a line or set of lines, including the line before and after\n");
            printf(" * 'e / E' - Save and Exit / Exit (without save)\n");
            printf(" * 'q / Q' - Save and Quit / Quit (without save)\n");
        } break;
        case 's':
        {
            editorState_save();
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
                        printLine(line - 2, 0);
                    for (int i = line; i <= endLine; i++) {
                        if (i - 1 >= 0 && i - 1 < buf_len(currentBuffer.lines))
                            printLine(i - 1, 0);
                    }
                    if (endLine < buf_len(currentBuffer.lines))
                        printLine(endLine, 0);
                    break;
                }
            }
            
            // Otherwise, just print that one line
            if (line - 2 >= 0)
                printLine(line - 2, 0);
            printLine(line - 1, 0);
            if (line < buf_len(currentBuffer.lines))
                printLine(line, 0);
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
            editorState_save();
            return ED_EXIT;
        } break;
        case 'E':
        {
            return ED_EXIT;
        } break;
        case 'q':
        {
            editorState_save();
            return ED_QUIT;
        } break;
        case 'Q':
        {
            return ED_QUIT;
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
    printLine(line - 2, 0);
    
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
    
    return ED_MENU;
}

// Insert lines after a specific line. Denote end of input by typing Ctrl-D (or Ctrl-Z+Enter on Windows) on new line.
internal void editorState_insertAfter(int line) {
    char c;
    if (line - 1 >= 0 && line - 1 < buf_len(currentBuffer.lines))
        printLine(line - 1, 0);
    int currentLine = line + 1;
    int lineStart = currentLine;
    
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
    
    int linesAddedAmt = buf_len(insertLines);
    
    // Insert the new lines into the lines buffer
    buf_add(currentBuffer.lines, linesAddedAmt);
    
    int linesLeft = buf_len(currentBuffer.lines) - linesAddedAmt - (lineStart - 1);
    // Move lines up by how many lines have been inserted
    for (int i = 0; i < linesLeft; i++) {
        currentBuffer.lines[buf_len(currentBuffer.lines) - i - 1] = currentBuffer.lines[lineStart - 2 + (linesLeft - i)];
        //    V
        // 0 1|2 3 4
        // 1 2|3 4
        //    -V
        // 1 2 3 4 5
        
        // 4 <- 3
        // 3 <- 2
    }
    
    // Copy over new lines
    for (int i = 0; i < linesAddedAmt; i++) {
        currentBuffer.lines[lineStart - 1 + i] = insertLines[i];
    }
    
    // Free the old lines stretchy buffer
    buf_free(insertLines);
    
    // Show the line that was moved due to inserting before it (and after the line before it)
    int movedLine = lineStart + linesAddedAmt;
    if (movedLine <= buf_len(currentBuffer.lines))
        printLine(movedLine - 1, 'v');
}

internal void editorState_insertBefore(int line) {
    char c;
    if (line - 2 >= 0 && line - 1 < buf_len(currentBuffer.lines))
        printLine(line - 2, 0);
    int currentLine = line;
    int lineStart = currentLine;
    
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
    
    int linesAddedAmt = buf_len(insertLines);
    
    // Insert the new lines into the lines buffer
    buf_add(currentBuffer.lines, linesAddedAmt);
    
    int linesLeft = buf_len(currentBuffer.lines) - linesAddedAmt - (lineStart - 1);
    // Moves lines up by how many lines have been inserted
    for (int i = 0; i < linesLeft; i++) {
        currentBuffer.lines[buf_len(currentBuffer.lines) - i - 1] = currentBuffer.lines[lineStart - 2 + (linesLeft - i)];
    }
    
    // Copy over new lines
    for (int i = 0; i < linesAddedAmt; i++) {
        currentBuffer.lines[lineStart - 1 + i] = insertLines[i];
    }
    
    // Free the old lines stretchy buffer
    buf_free(insertLines);
    
    // Show the line that was moved due to the insertion before it
    int movedLine = lineStart + linesAddedAmt;
    if (movedLine <= buf_len(currentBuffer.lines))
        printLine(movedLine - 1, 'v');
}

internal void editorState_appendTo(int line) {
    char c;
    if (line - 2 >= 0 && line - 2 < buf_len(currentBuffer.lines))
        printLine(line - 2, 0);
    
    // Remove the new line character from the line
    buf_pop(currentBuffer.lines[line - 1].chars);
    
    int count = 0;
    printLine(line - 1, 'A');
    while ((c = getchar()) != EOF) {
        if (c == (char) 24) { // Ctrl-X (^X) - Cancel
            // Pop off all of the characters that have been added thus far
            for (int i = 0; i < count; i++) {
                buf_pop(currentBuffer.lines[line - 1].chars);
            }
            // Push back on the new line character the was previously removed
            buf_push(currentBuffer.lines[line - 1].chars, '\n');
            // Discard the new line character that's typed after Ctrl-X
            getchar();
            // Cancel the operation by returning
            return;
        }
        buf_push(currentBuffer.lines[line - 1].chars, c);
        count++;
        if (c == '\n') break;
    }
}

internal void editorState_prependTo(int line) {
    char c;
    if (line - 2 >= 0 && line - 2 < buf_len(currentBuffer.lines))
        printLine(line - 2, 0);
    
    char *chars = NULL; // The new char stretchy buffer
    
    printLine(line - 1, 'I');
    printPrompt("%4s ^- ", ""); // TODO
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
    
    // Move all the characters from the original line buffer to the new one
    for (int i = 0; i < buf_len(currentBuffer.lines[line - 1].chars); i++) {
        buf_push(chars, currentBuffer.lines[line - 1].chars[i]);
    }
    
    // Free the original line buffer and set the new line buffer to the current line
    buf_free(currentBuffer.lines[line - 1].chars);
    currentBuffer.lines[line - 1].chars = chars;
}

internal void editorState_replaceLine(int line) {
    char c;
    if (line - 2 >= 0 && line - 2 < buf_len(currentBuffer.lines))
        printLine(line - 2, 0);
    char *chars = NULL; // The new char stretchy buffer
    
    printLine(line - 1, 'r');
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
    
    // Free the original line buffer and set the new line buffer to the current line
    buf_free(currentBuffer.lines[line - 1].chars);
    currentBuffer.lines[line - 1].chars = chars;
}

// TODO: Problem with replacing only one character
internal void editorState_replaceString(int line, char *str, int strLength) {
    char c;
    
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
        printLine(line - 2, 0);
    
    // Print the string where the replacement is occuring
    printLine(line - 1, 'R');
    
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
    
    // Move the characters before the replacement from the original line buffer to the new one
    for (int i = 0; i < index; i++) {
        buf_push(chars, currentBuffer.lines[line - 1].chars[i]);
    }
    
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
    
    // Move the characters after the replacement from the original line buffer to the new one
    int afterIndex; // Index of the next character after the last character of the string being replaced
    if (str[strLength - 1] == '\0' || str[strLength - 1] == '\n') {
        afterIndex = index + strLength - 1;
    } else {
        afterIndex = index + strLength;
    }
    for (int i = afterIndex; i < buf_len(currentBuffer.lines[line - 1].chars); i++) {
        buf_push(chars, currentBuffer.lines[line - 1].chars[i]);
    }
    
    // Free the original line buffer and set the new line buffer to the current line
    buf_free(currentBuffer.lines[line - 1].chars);
    currentBuffer.lines[line - 1].chars = chars;
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
        printLine(foundIndex - 1, 0);
    
    // Print the string where the occurance was found
    printLine(foundIndex, 0);
    
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
        printLine(line - 2, 0);
    
    if (line == buf_len(currentBuffer.lines)) { // TODO: This isn't working correctly
        printLine(line - 1, 'x');
        buf_free(currentBuffer.lines[buf_len(currentBuffer.lines) - 1].chars);
        buf_pop(currentBuffer.lines);
        
        // Show the first line that was moved - the line # should be the same as the line that was deleted
        if (line <= buf_len(currentBuffer.lines))
            printLine(line - 1, '^');
        return;
    }
    
    printLine(line - 1, 'x');
    
    // Delete char stretchy buffer of line that's being deleted
    buf_free(currentBuffer.lines[line - 1].chars);
    
    // Move all lines down one
    for (int i = line - 1; i < buf_len(currentBuffer.lines) - 1; i++) {
        currentBuffer.lines[i] = currentBuffer.lines[i + 1];
    }
    
    // Decrease the length of the buffer, keeping the char stretchy buffer of this last line because it was moved down one.
    buf_pop(currentBuffer.lines);
    
    // Show the first line that was moved - the line # should be the same as the line that was deleted
    if (line <= buf_len(currentBuffer.lines))
        printLine(line - 1, '^');
}

internal void editorState_moveUp(int line) {
    // Show the line before the line being moved up to
    if (line - 2 > 0)
        printLine(line - 3, 0);
    
    // Store the line where the given line is being moved up to
    Line tmp = currentBuffer.lines[line - 2];
    
    // Set the new position of the line being moved
    currentBuffer.lines[line - 2] = currentBuffer.lines[line - 1];
    
    // Set the old position to the line stored in tmp (the line being moved down)
    currentBuffer.lines[line - 1] = tmp;
    
    // Print the new position of the line that was moved and the old line that was moved down
    printLine(line - 2, '^');
    printLine(line - 1, 'v');
    
    // Print the next line to give context
    printLine(line, 0);
}

internal void editorState_moveDown(int line) {
    // Show the line before the line being moved down
    if (line - 1 > 0)
        printLine(line - 2, 0);
    
    // Store the line where the given line is being moved down to
    Line tmp = currentBuffer.lines[line];
    
    // Set the new position of the line being moved
    currentBuffer.lines[line] = currentBuffer.lines[line - 1];
    
    // Set the old position to the line stored in tmp (the line being moved up)
    currentBuffer.lines[line - 1] = tmp;
    
    // Print the new position of the line that was moved and the old line that was moved up
    printLine(line - 1, '^');
    printLine(line, 'v');
    
    // Print the next line to give context
    printLine(line + 1, 0);
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
            printLineNumber("%5d ", line + 1);
            break;
        }
        printLine(line, 0);
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
                printLineNumber("%5d ", line + 1);
                break;
            }
            printLine(line, 0);
        }
        
        offset = linesAtATime + offset + 1;
        if (offset >= buf_len(currentBuffer.lines)) {
            break;
        }
        printPrompt("\n<%s | preview> ", currentBuffer.openedFilename);
    }
    
    printf("\n");
}

/* Prints one line of text given the line number. Note that the line numbers start at 0 (although they are displayed to the user starting at 1). */
void printLine(int line, char operation) {
    if (buf_len(currentBuffer.lines) <= 0 && line == 0) {
        if (operation != 0)
            printLineNumber("%c%4d ", operation, 1);
        else printLineNumber("%5d ", 1);
        printf("\n");
        return;
    }
    
    if (line > buf_len(currentBuffer.lines)) {
        // Error!
        return;
    }
    
    if (operation != 0)
        printLineNumber("%c%4d ", operation, line + 1);
    else printLineNumber("%5d ", line + 1);
    for (int i = 0; i < buf_len(currentBuffer.lines[line].chars); i++) {
        putchar(currentBuffer.lines[line].chars[i]);
    }
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

/* Save the currently stored text in a new file (or the file that was opened or saved to previously) */
void editorState_save(void) {
    FILE *fp;
    
    if (currentBuffer.openedFilename && buf_len(currentBuffer.openedFilename) > 0) {
        fp = fopen(currentBuffer.openedFilename, "w");
    } else {
        printPrompt("Enter the filename: ");
        char filename[MAXLENGTH / 4];
        int filenameLength = 0;
        filenameLength = parsing_getLine(filename, MAXLENGTH / 4, true);
        while (filenameLength == -1) {
            printPrompt("Enter the filename: ");
            filenameLength = parsing_getLine(filename, MAXLENGTH / 4, true);
        }
        fp = fopen(filename, "w");
        // Copy filename into openedFilename
        for (int i = 0; i < filenameLength; i++) {
            buf_push(currentBuffer.openedFilename, filename[i]);
        }
        
        
        // Find last occurance of '.' in filename
        int dotIndex;
        for (int i = 0; i < strlen(filename); i++) {
            if (filename[i] == '.') {
                dotIndex = i;
            }
        }
        
        // Determine filetype based on extension
        // TODO: Doesn't handle CPP files yet.
        FileType ft;
        switch (filename[dotIndex + 1])
        {
            case 't':
            ft = FT_TEXT;
            break;
            case 'm':
            ft = FT_MARKDOWN;
            break;
            case 'c':
            ft = FT_C;
            break;
            case 'h':
            ft = FT_C_HEADER;
            break;
            default:
            ft = FT_UNKNOWN;
            break;
        }
        if (ft != FT_UNKNOWN) {
            int isType = true;
            char *ftExt;
            getFileTypeExtension(ft, &ftExt);
            for (int i = dotIndex + 1; i < strlen(filename); i++) {
                if (filename[i] != ftExt[i - dotIndex - 1]) {
                    isType = false;
                    break;
                }
            }
            
            if (isType) {
                currentBuffer.fileType = ft;
            } else {
                currentBuffer.fileType = FT_UNKNOWN;
            }
            free(ftExt);
        }
    }
    
    printf("Saving '%s'.\n", currentBuffer.openedFilename);
    for (int line = 0; line < buf_len(currentBuffer.lines); line++) {
        for (int i = 0; i < buf_len(currentBuffer.lines[line].chars); i++) {
            fprintf(fp, "%c", currentBuffer.lines[line].chars[i]);
        }
    }
    
    fclose(fp);
}
