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

char text[MAXLENGTH];
int textEndI;
int textEndLine;

typedef enum NewFileState {
	NF_KEEP,
	NF_EDITOR,
	NF_MENU,
	NF_EXIT, // Exit to main menu
	NF_QUIT, // Quit program
} NewFileState;

State newFile(void);
State openFile(char *filename);
State newFile_menu(void);
State newFile_editor(void);
void printText(void);
void newFile_save(void);

/* === parsing.c === */
int parsing_getLine(char *line, int max, int trimSpace);

#endif
