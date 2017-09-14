#include <stdio.h>

/* Gets input, trims leading space, and puts into line char array
	as long as it doesn't go over the max.

	@line - array of characters to fill up
	@max - maximum amount of characters allowed in line
	@trimSpace - Whether to trim leading space (1) or not (0)
	@return - the length of the line
 */
int getLine(char *line, int max, int trimSpace) {
	char c;
	register int i = 0;

	/* Trim whitespace */
	while (trimSpace && (c = getchar()) == ' ' || c == '\t')
		;

	/* If there's nothing left, return */
	if (c == '\n') {
		line[i] = '\0';
		return 1;
	}

	/* Transfer input characters into line string */
	while (c != EOF && c != '\n' && i < max)
	{
		line[i] = c;
		++i;
		c = getchar();
	}

	/* End of string */
	line[i] = '\0';

	return i;
}