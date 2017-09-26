#include <stdio.h>

#include "lineeditor.h"

int textEndI = 0;
int textEndLine = 0;

/* Get input for new file */
State newFile(void) {
	static NewFileState subState = NF_EDITOR;
	static NewFileState subStatePrev;
	
	subStatePrev = subState;

	switch (subState) {
		case NF_EDITOR:
		{
			printf("Line Editor: New File\n");
			printf("Press Ctrl-D on blank line to denote End Of File\n\n");
			subState = newFile_editor();
			if (subState == NF_KEEP) subState = subStatePrev;
		} break;
		case NF_MENU:
		{
			subState = newFile_menu();
			if (subState == NF_KEEP) subState = subStatePrev;
		} break;
		case NF_EXIT:
		{
			subState = NF_EDITOR;
			textEndI = 0;
			textEndLine = 0;

			return MAIN_MENU;
		} break;
		case NF_QUIT:
		{
			return QUIT;
		} break;
		default:
		printf("Unknown Command!");
	}

	return KEEP;
}

State openFile(char *filename)
{
	// Open the file, then
	// Take all characters from file and put into 'text'

	// Show the menu
	

	return KEEP;
}

/* Menu for New File */
State newFile_menu(void) {
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
			newFile_save();
		} break;
		case 'c':
		{
			printf("\n");
			return NF_EDITOR;
		} break;
		case 'p':
		{
			printText();
		} break;
		case 'd':
		{
			newFile_save();
			return NF_EXIT;
		} break;
		case 'D':
		{
			return NF_EXIT;
		} break;
		case 'q':
		{
			newFile_save();
			return NF_QUIT;
		} break;
		case 'Q':
		{
			return NF_QUIT;
			//running = FALSE;
		} break;
		default:
		printf("Unknown Command!");
	}

	return NF_KEEP;
}

State newFile_editor(void) {
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
	return NF_MENU;
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
void newFile_save(void) {
	FILE *fp;
	fp = fopen("output.txt", "w");

	fprintf(fp, "%s", text);

	fclose(fp);
}
