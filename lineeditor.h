#ifndef LINEEDITOR_H
#define LINEEDITOR_H

#define TRUE 1
#define FALSE 0
#define forever for(;;)

typedef enum State {
	KEEP,
	MAIN_MENU,
	NEW_FILE,
	QUIT
} State;

/* === parsing.c === */
int getLine(char *line, int max, int trimSpace);

/* === editor.c === */
#define MAXLENGTH 2000000

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
State newFile_menu(void);
State newFile_editor(void);
void printText(void);
void newFile_save(void);

#endif