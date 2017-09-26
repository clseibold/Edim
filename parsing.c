#include <stdio.h>

#include "lineeditor.h"

/* Gets input, trims leading space, and puts into line char array
	as long as it doesn't go over the max.

	@line - array of characters to fill up
	@max - maximum amount of characters allowed in line
	@trimSpace - Whether to trim leading space (1) or not (0)
	@return - the length of the line
 */
int parsing_getLine(char *line, int max, int trimSpace) {
	int c;
	int i = 0;

	/* Trim whitespace */
	while (trimSpace && ((c = getchar()) == ' ' || c == '\t'))
		;

	if (!trimSpace) c = getchar();

	/* If there's nothing left, return */
	if (c == '\n') {
		line[i] = '\0';
		return 1; /* Including \0 */
	}

	/* Transfer input characters into line string */
	while (c != EOF && c != '\n' && i < max)
	{
		line[i] = (char) c;
		++i;
		c = getchar();
	}

	/* End of string */
	line[i] = '\0';
	++i; /* Includes '\0' in the length */

	return i;
}
