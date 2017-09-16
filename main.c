#include <stdio.h>

#include "lineeditor.h"

State mainMenu() {
	printf("Line Editor: Main Menu\n");
	printf("Copyright (c) Christian Seibold. All Rights Reserved.\n\n");

	/* Save - save new file */
	printf(" * 'n' - New File\n");
	/* Edit - rewrite a specific line, group of lines, group of characters in a line (given column numbers), and word/group of words */
	printf(" * 'o' - Open File\n");
	/* Quit */
	printf(" * 'Q' - Quit\n");

	/* Prompt */
	printf("\n>> ");

	/* get first character - the menu item */
	char c;
	c = getchar();
	
	/* Store rest of line in rest */
	char rest[MAXLENGTH];
	int restLength = getLine(rest, MAXLENGTH, TRUE);
	/*printf("Rest is: %s\n", rest);
	printf("RestLength is: %d", restLength);*/

	switch (c) {
		case 'n':
		{
			return NEW_FILE;
		} break;
		case 'o':
		{
			printf("\nUnimplemented!\n");
		} break;
		case 'Q':
		{
			return QUIT;
		} break;
		default:
		printf("Unknown Command!");
	}

	return KEEP;
}

int main() {
	int running = TRUE;
	State state = MAIN_MENU;
	State statePrev = state;

	while (running) {
		switch (state) {
			case MAIN_MENU:
			{
				state = mainMenu();
				if (state == KEEP) state = statePrev;
			} break;
			case NEW_FILE:
			{
				printf("\n");
				state = newFile();
				if (state == KEEP) state = statePrev;
			} break;
			case QUIT:
			{
				running = FALSE;
			}
		}

		statePrev = state;
	}

	return(0);
}
