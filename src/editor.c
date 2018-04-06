#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "lineeditor.h"

char openedFilename[MAXLENGTH] = { 0 };
int textEndI = 0;
int textEndLine = 0;

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
            printf("Line Editor: New File\n");
            printf("Press Ctrl-D (or Ctrl-Z on Windows) to denote End Of File\n\n");
            // TODO: Reset openedFilename and text
            //openedFilename = { 0 };
            //text = { 0 };
            textEndI = 0;
            textEndLine = 0;
            subState = ED_EDITOR;
            subState = editorState_editor();
            if (subState == ED_KEEP) subState = subStatePrev;
        } break;
        case ED_OPEN:
        {
            // TODO: Get arg for filename or Prompt for filename if no args provided
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
            printf("Opening file %s\n", filename);
            subState = openFile(filename, ii + 1);
            free(filename);
            filename = 0;
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
            textEndI = 0;
            textEndLine = 0;
            
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

EditorState openFile(char *filename, unsigned int filenameLength)
{
    // Open the file, then
    // Take all characters from file and put into 'text'
    //printf("\nOpening File\n");
    char c;
    register int i = 0;
    register int line = 1;
    
    FILE *fp;
    fp = fopen(filename, "r");
    strncpy(openedFilename, filename, filenameLength + 1);
    
    //printf("%3d ", line);
    while ((c = fgetc(fp)) != EOF) {
        text[i] = c;
        //printf("%c", c);
        if (c == '\n') {
            ++line;
            //printf("%3d ", line);
        }
        ++i;
    }
    
    text[i] = '\0';
    textEndI = i;
    textEndLine = line;
    
    printText();
    
    fclose(fp);
    
    return ED_MENU;
}

/* Menu for New File */
EditorState editorState_menu(void) {
    printf("\nNew File: Menu\n");
    
    /* Save - save new file */
    printf(" * 's' - Save\n");
    /* Edit - rewrite a specific line, group of lines, group of characters in a line (given column numbers), and word/group of words */
    printf(" * 'e' - Edit\n");
    /* This will delete the '\0' and continue writing to the text */
    printf(" * 'c' - Continue\n");
    /* Prints out the text */
    printf(" * 'p' - Preview\n");
    /* Saves, then goes back to main menu */
    printf(" * 'd' - Save and Exit\n");
    /* Goes back to main menu */
    printf(" * 'D' - Exit (without save)\n");
    /* Save and Quit */
    printf(" * 'q' - Save and Quit\n");
    /* Quit without save */
    printf(" * 'Q' - Quit (without save)\n");
    
    /* Prompt */
    printf("\neditor: ");
    
    /* get first character - the menu item */
    char c;
    c = getchar();
    
    /* Store rest of line in rest */
    char rest[MAXLENGTH];
    int restLength = parsing_getLine(rest, MAXLENGTH, TRUE);
    /*printf("Rest is: %s\n", rest);
 printf("RestLength is: %d", restLength);*/
    
    switch (c) {
        case 's':
        {
            editorState_save();
        } break;
        case 'c':
        {
            printf("\n");
            return ED_EDITOR;
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
            //running = FALSE;
        } break;
        default:
        printf("Unknown Command!");
    }
    
    return ED_KEEP;
}

EditorState editorState_editor(void) {
    char c;
    register int i = 0;
    register int line = 1;
    
    // If continuing a previously typed-in file,
    //  start on last line and overwrite the EOF character
    if (textEndI > 0) {
        i = textEndI;
        line = textEndLine;
    }
    
    printf("%3d ", line);
    while ((c = getchar()) != EOF) {
        text[i] = c;
        if (c == '\n') {
            ++line;
            printf("%3d ", line);
        }
        ++i;
    }
    
    text[i] = '\0';
    textEndI = i;
    textEndLine = line;
    
    printf("\n");
    return ED_MENU;
}

/* Print text given from input with line numbers */
void printText(void) {
    register int i;
    register int line = 1;
    
    printf("\n\n%3d ", line);
    for (i = 0; i <= textEndI; i++) {
        putchar(text[i]);
        if (text[i] == '\n') {
            ++line;
            printf("%3d ", line);
        }
        if (text[i] == '\0') {
            break;
        }
    }
    printf("\n");
}

/* Save new file */
void editorState_save(void) {
    FILE *fp;
    if (openedFilename[0] != '\0') {
        fp = fopen(openedFilename, "w");
    } else {
        printf("Enter the filename: ");
        char filename[MAXLENGTH];
        int filenameLength = 0;
        filenameLength = parsing_getLine(filename, MAXLENGTH, TRUE);
        while (filenameLength == -1) {
            printf("Enter the filename: ");
            filenameLength = parsing_getLine(filename, MAXLENGTH, TRUE);
        }
        fp = fopen(filename, "w");
        // Copy filename into openedFilename
        for (int i = 0; i < MAXLENGTH; i++) {
            openedFilename[i] = filename[i];
        }
    }
    
    fprintf(fp, "%s", text);
    
    fclose(fp);
}
