#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <stdlib.h>

#include "lineeditor.h"

char *openedFilename = NULL;

/* Get input for new file */
State editorState(EditorState state, char args[MAXLENGTH], int argsLength) {
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
            if (argsLength > 1) {
                int i = 0;
                while (args[i] != ' ' && args[i] != '\n' && args[i] != '\0') {
                    buf_push(openedFilename, args[i]);
                    ++i;
                }
            }
            
            printf("Opening a new file.\n");
            printf("Press Ctrl-D (or Ctrl-Z on Windows) on new line to denote End Of Input\n\n");
            
            // Make sure currently stored text has been cleared out.
            assert(buf_len(lines) == 0);
            
            subState = ED_EDITOR;
            subState = editorState_editor();
            if (subState == ED_KEEP) subState = subStatePrev;
        } break;
        case ED_OPEN:
        {
            // TODO: Get arg for filename or Prompt for filename if no args provided
            if (argsLength <= 1) {
                printf("Enter the filename: ");
                char filename[MAXLENGTH];
                int filenameLength = 0;
                filenameLength = parsing_getLine(filename, MAXLENGTH, true);
                while (filenameLength == -1) {
                    printf("Enter the filename: ");
                    filenameLength = parsing_getLine(filename, MAXLENGTH, true);
                }
                subState = openFile(filename);
            } else {
                char *filename = alloca((argsLength + 1) * sizeof(char));
                int i = 0;
                int ii = 0;
                
                while (args[i] != ' ' && args[i] != '\n' && args[i] != '\0')
                {
                    filename[ii] = args[i];
                    ++i;
                    ++ii;
                }
                filename[ii] = '\0';
                subState = openFile(filename);
                //free(filename);
                //filename = 0;
            }
            if (subState == ED_KEEP) subState = subStatePrev;
        } break;
        case ED_EDITOR:
        {
            /*printf("Line Editor: New File\n");
   printf("Press Ctrl-D (or Ctrl-Z on Windows) to denote End Of File\n\n");*/
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
            printf("Exiting");
            
            // Clear openedFilename and the file information
            buf_free(openedFilename);
            
            for (int i = 0; i < buf_len(lines); i++) {
                buf_free(lines[i].chars);
            }
            
            buf_free(lines);
            
            initialSet = 0;
            return MAIN_MENU;
        } break;
        case ED_QUIT:
        {
            return QUIT;
        } break;
        default:
        printf("Unknown Command!");
    }
    
    return KEEP;
}

EditorState openFile(char *filename)
{
    // Open the file, then
    // Take all characters from file and put into lines streatchy buffer each Line with a chars streatchy buffer.
    char c;
    int line = 1;
    
    FILE *fp;
    fp = fopen(filename, "r");
    printf("Opening file '%s'.\n", filename);
    for (int i = 0; i < strlen(filename) + 1; i++) {
        buf_push(openedFilename, filename[i]);
    }
    // Make sure the filename ends with '\0'
    assert(openedFilename[buf_len(openedFilename) - 1] == '\0');
    char *chars = NULL;
    
    while ((c = fgetc(fp)) != EOF) {
        buf_push(chars, c);
        if (c == '\n') {
            buf_push(lines, ((Line) { chars, line }));
            ++line;
            chars = NULL; // Create new char Stretchy Buffer for next line
        }
    }
    
    printText();
    
    fclose(fp);
    
    
    return ED_MENU;
}

/* Menu for Editor */
EditorState editorState_menu(void) {
    /* Prompt */
    printf("\neditor> ");
    
    /* get first character - the menu item */
    char c;
    c = getchar();
    
    if (c == '\n') return KEEP;
    
    /* Store rest of line in rest */
    char rest[MAXLENGTH];
    int restLength = parsing_getLine(rest, MAXLENGTH, true);
    //printf("Rest is: %s\n", rest);
    //printf("RestLength is: %d", restLength);
    
    printf("\n");
    
    switch (c) {
        case '?': // TODO: Add new file and open file.
        {
            if (buf_len(openedFilename) > 0)
                printf("'%s' is currently open.\n\n", openedFilename);
            else printf("An unnamed file is currently open.\n\n");
            
            printf(" * 's' - Save\n");
            /* Edit - rewrite a specific line, group of lines, group of characters in a line (given column numbers), and word/group of words */
            printf(" * 'e' - Edit\n");
            printf(" * 'a [line#]' - Insert after the line number\n");
            printf(" * 'i [line#]' - Insert before the line number\n");
            printf(" * 'x [line#]' - Deletes a line\n");
            // Continue writing from last line of file.
            printf(" * 'c' - Continue\n");
            printf(" * 'p' - Preview\n");
            printf(" * 'd' - Save and Exit\n");
            printf(" * 'D' - Exit (without save)\n");
            printf(" * 'q' - Save and Quit\n");
            printf(" * 'Q' - Quit (without save)\n");
        } break;
        case 's':
        {
            editorState_save();
        } break;
        case 'c':
        {
            return ED_EDITOR;
        } break;
        case 'a':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            // TODO: if line == 0, then insert before line 1
            
            char lineInput[MAXLENGTH];
            int length;
            while (line == 0 || line > buf_len(lines) || length == -1) {
                if (rest != end)
                    printf("That line number exceeds the bounds of the file.\n");
                printf("Enter a line number: ");
                length = parsing_getLine(lineInput, MAXLENGTH, true);
                line = (int) strtol(lineInput, &end, 10);
            }
            
            editorState_insertAfter(line);
        } break;
        case 'i':
        {
            printf("Unimplemented!\n");
        } break;
        case 'x':
        {
            char *end;
            int line = (int) strtol(rest, &end, 10);
            
            char lineInput[MAXLENGTH];
            int length;
            while (line == 0 || line > buf_len(lines) || length == -1) {
                if (rest != end)
                    printf("That line number exceeds the bounds of the file.\n");
                printf("Enter a line number: ");
                length = parsing_getLine(lineInput, MAXLENGTH, true);
                line = (int) strtol(lineInput, &end, 10);
            }
            
            editorState_deleteLine(line);
        } break;
        case 'p':
        {
            printText();
        } break;
        case 'd':
        {
            editorState_save();
            return ED_EXIT;
        } break;
        case 'D':
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
        printf("Unknown Command!");
    }
    
    return ED_KEEP;
}

// Editor - will allow user to type in anything, showing line number at start of new lines. To exit the editor, press Ctrl-D on Linux or Ctrl-Z+Enter on Windows. As each new line is entered, the characters will be added to a char pointer streatchy buffer (dynamic array). Then, this line will be added to the streatchy buffer of lines (called 'lines').
EditorState editorState_editor(void) {
    char c;
    int line = 1;
    
    // If continuing a previously typed-in file,
    //  start on last line and overwrite the EOF character
    if (buf_len(lines) > 0) {
        line = buf_len(lines) + 1;
    }
    
    // Show the previous line to give context.
    printLine(line - 2);
    
    char *chars = NULL;
    
    printf("%3d ", line);
    while ((c = getchar()) != EOF) {
        buf_push(chars, c);
        if (c == '\n') {
            buf_push(lines, ((Line) { chars, line }));
            ++line;
            printf("%3d ", line);
            chars = NULL; // Create new char stretchy buffer for next line
        }
    }
    
    return ED_MENU;
}

EditorState editorState_insertAfter(int line) {
    char c;
    printLine(line - 1);
    int currentLine = line + 1;
    int lineStart = currentLine;
    
    Line *insertLines = NULL;
    char *chars = NULL;
    
    printf("%3d ", currentLine);
    while ((c = getchar()) != EOF) {
        buf_push(chars, c);
        if (c == '\n') {
            buf_push(insertLines, ((Line) { chars, currentLine }));
            ++currentLine;
            printf("%3d ", currentLine);
            chars = NULL; // create new char stretchy buffer for next line
        }
    }
    
    int linesAddedAmt = buf_len(insertLines);
    
    // Insert the new lines into the lines buffer
    buf_add(lines, linesAddedAmt);
    
    int linesLeft = buf_len(lines) - linesAddedAmt - (lineStart - 1);
    // Move lines up by how many lines have been inserted
    for (int i = 0; i < linesLeft; i++) {
        lines[buf_len(lines) - i - 1] = lines[lineStart - 2 + (linesLeft - i)];
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
        lines[lineStart - 1 + i] = insertLines[i];
    }
    
    // Free the old line stretchy buffer
    buf_free(insertLines);
    
    return ED_MENU;
}

void editorState_deleteLine(int line) {
    if (line == buf_len(lines)) {
        buf_free(buf_pop(lines)->chars);
        return;
    }
    
    printf("Deleting line '%d'\n", line - 1);
    
    // Delete char stretchy buffer of line that's being deleted
    buf_free(lines[line - 1].chars);
    
    // Move all lines down one
    for (int i = line - 1; i < buf_len(lines) - 1; i++) {
        lines[i] = lines[i + 1];
    }
    
    // Decrease the length of the buffer, keeping the char stretchy buffer of this last line because it was moved down one.
    buf_pop(lines)->chars;
}

/* Print the currently stored text with line numbers */
void printText(void) {
    if (buf_len(lines) <= 0) {
        printf("%3d ", 1);
        printf("\n");
        return;
    }
    
    for (int line = 0; line < buf_len(lines); line++) {
        printf("%3d ", line + 1);
        for (int i = 0; i < buf_len(lines[line].chars); i++) {
            putchar(lines[line].chars[i]);
        }
    }
    
    int last_line = buf_len(lines) - 1;
    int last_char = buf_len(lines[last_line].chars) - 1;
    if (lines[last_line].chars[last_char] == '\n') {
        printf("%3d ", last_line + 2);
    }
    printf("\n");
}

/* Prints one line of text given the line number. Note that the line numbers start at 0 (although they are displayed to the user starting at 1). */
void printLine(int line) {
    if (buf_len(lines) <= 0 && line == 0) {
        printf("%3d ", 1);
        printf("\n");
        return;
    }
    
    if (line > buf_len(lines)) {
        // Error!
        return;
    }
    
    printf("%3d ", line + 1);
    for (int i = 0; i < buf_len(lines[line].chars); i++) {
        putchar(lines[line].chars[i]);
    }
}

/* Save the currently stored text in a new file (or the file that was opened or saved to previously) */
void editorState_save(void) {
    FILE *fp;
    if (buf_len(openedFilename) > 0) {
        fp = fopen(openedFilename, "w");
    } else {
        printf("Enter the filename: ");
        char filename[MAXLENGTH];
        int filenameLength = 0;
        filenameLength = parsing_getLine(filename, MAXLENGTH, true);
        while (filenameLength == -1) {
            printf("Enter the filename: ");
            filenameLength = parsing_getLine(filename, MAXLENGTH, true);
        }
        fp = fopen(filename, "w");
        // Copy filename into openedFilename
        for (int i = 0; i < filenameLength; i++) {
            buf_push(openedFilename, filename[i]);
        }
    }
    
    for (int line = 0; line < buf_len(lines); line++) {
        for (int i = 0; i < buf_len(lines[line].chars); i++) {
            fprintf(fp, "%c", lines[line].chars[i]);
        }
    }
    
    fclose(fp);
}
