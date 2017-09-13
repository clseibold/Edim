#include <stdio.h>

#define MAXLENGTH 2000000

char text[MAXLENGTH];

void newFile(void);
void printText(void);
void newFile_menu(void);

void newFile(void) {
	//char text[MAXLENGTH];
	char c;
	int i = 0;
	int line = 1;

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

	printf("\n");
	newFile_menu();
}

void printText(void) {
	int line = 1;
  int i;
  
	printf("\n\n%3d ", line);
	for (i = 0; i < MAXLENGTH; i++) {
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
	newFile_menu();
}

void newFile_menu(void) {
	printf("\nNew File: Menu\n");
	printf(" * 's' - Save\n");
	printf(" * 'c' - Continue\n"); // This will delete the '\0' and continue writing to the text
	printf(" * 'd' - Discard\n"); // Goes back to main menu
	printf(" * 'p' - Preview\n"); // Prints out the text
	printf(" * 'q' - Discard and Quit\n"); // Prints out the text
	printf("\n>> ");

	char c;
	c = getchar();
	getchar(); // Get rid of new line character

	switch (c) {
		case 's':
		{

		} break;
		case 'c':
		{

		} break;
		case 'd':
		{

		} break;
		case 'p':
		{
			printText();
		} break;
		case 'q':
		{
			return;
		}
		default:
		printf("Unknown Command!");
	}
}

int main() {
	printf("Line Editor: New File\n");
	printf("Press Ctrl-D on blank line to denote End Of File\n\n");
	newFile();

	return(0);
}
