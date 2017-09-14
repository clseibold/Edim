#ifndef LINEEDITOR_H
#define LINEEDITOR_H

#define TRUE 1
#define FALSE 0
#define forever for(;;)

typedef enum State {
	MAIN_MENU,
	NEW_FILE
} State;

/* === parsing.c === */
int getLine(char *line, int max, int trimSpace);

#endif