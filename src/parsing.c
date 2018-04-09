#include <stdio.h>
#include <assert.h>

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

void getFileTypeExtension(FileType ft, char **ftExt) {
    switch (ft) {
        case FT_TEXT:
        {
            (*ftExt) = malloc(sizeof(char) * 3);
            (*ftExt)[0] = 't';
            (*ftExt)[1] = 'x';
            (*ftExt)[2] = 't';
        } break;
        case FT_MARKDOWN:
        {
            (*ftExt) = malloc(sizeof(char) * 2);
            (*ftExt)[0] = 'm';
            (*ftExt)[1] = 'd';
        } break;
        case FT_C:
        {
            (*ftExt) = malloc(sizeof(char) * 1);
            (*ftExt)[0] = 'c';
        } break;
        case FT_CPP:
        {
            (*ftExt) = malloc(sizeof(char) * 3);
            (*ftExt)[0] = 'c';
            (*ftExt)[1] = 'p';
            (*ftExt)[2] = 'p';
        } break;
        case FT_C_HEADER:
        {
            (*ftExt) = malloc(sizeof(char) * 1);
            (*ftExt)[0] = 'h';
        } break;
    }
}

void createOutline(void) {
    switch (currentBuffer.fileType) {
        case FT_MARKDOWN:
        {
            createMarkdownOutline();
        } break;
    }
}

// Currently, after every operation that modifies the file, the outline is recreated.
// TODO: Perhaps it would be more efficient to only change the line a node points to when
// that line is moved/changed by using some type of hash table for integer keys.
void recreateOutline(void) {
    switch (currentBuffer.fileType) {
        case FT_MARKDOWN:
        {
            // Pop off all of the current nodes
            buf_pop_all(currentBuffer.outline.markdown_nodes);
            assert(buf_len(currentBuffer.outline.markdown_nodes) == 0);
            createMarkdownOutline();
        } break;
    }
}

void showOutline(void) {
    switch (currentBuffer.fileType) {
        case FT_MARKDOWN:
        {
            showMarkdownOutline();
        } break;
    }
}

void createMarkdownOutline(void) {
    assert(currentBuffer.fileType == FT_MARKDOWN);
    // Go through each line
    for (int line = 0; line < buf_len(currentBuffer.lines); line++) {
        // If starts with a hash, then it's a heading
        if (currentBuffer.lines[line].chars[0] == '#') {
            int level = 0;
            // Increment level with each successive '#'
            for (int i = 1; i < buf_len(currentBuffer.lines[line].chars); i++) {
                if (currentBuffer.lines[line].chars[i] == '#') {
                    level++;
                } else break;
            }
            
            // create the node and push it
            MarkdownOutlineNode node;
            node.line = &(currentBuffer.lines[line]);
            node.lineNum = line;
            node.level = level;
            
            buf_push(currentBuffer.outline.markdown_nodes, node);
        }
    }
}

void showMarkdownOutline(void) {
    assert(currentBuffer.fileType == FT_MARKDOWN);
    
    // Go though each node
    for (int node_i = 0; node_i < buf_len(currentBuffer.outline.markdown_nodes); node_i++) {
        int linenum = currentBuffer.outline.markdown_nodes[node_i].line - currentBuffer.lines;
        printLine(linenum, 0);
        // Print out the line
        //printLine(currentBuffer.outline.markdown_nodes[node_i].lineNum, 0);
    }
}
