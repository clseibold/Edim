#ifndef LINEEDITOR_H
#define LINEEDITOR_H

#define TRUE 1
#define FALSE 0
#define forever for(;;)

typedef enum State {
    KEEP,
    MAIN_MENU,
    NEW_FILE,
    OPEN_FILE,
    QUIT
} State;

/* === editor.c === */
#define MAXLENGTH 200000 /* 2000000 was too big on Windows */

char openedFilename[MAXLENGTH];
char text[MAXLENGTH];
int textEndI;
int textEndLine;

typedef enum EditorState {
    ED_KEEP,
    ED_OPEN,
    ED_NEW,
    ED_EDITOR,
    ED_MENU,
    ED_EXIT, // Exit to main menu
    ED_QUIT, // Quit program
} EditorState;

State editorState(EditorState state, char args[MAXLENGTH], int argsLength);
EditorState openFile(char *filename, unsigned int filenameLength);
EditorState editorState_menu(void);
EditorState editorState_editor(void);
void editorState_save(void);
void printText(void);

/* === parsing.c === */
int parsing_getLine(char *line, int max, int trimSpace);

#endif
