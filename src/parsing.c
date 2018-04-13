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

// Gets input, trims leading space, and puts into a char stretchy buffer (dynamic array). Does not add null character at end.
// Returns the length of the buffer.
int parsing_getLine_dynamic(char **chars, int trimSpace) {
    int c;
    
    /* Trim whitespace */
    while (trimSpace && ((c = getchar()) == ' ' || c == '\t'))
        ;
    
    if (!trimSpace) c = getchar();
    
    /* If there's nothing left, return */
    if (c == '\n') {
        return 0;
    }
    
    /* Push input characters onto buffer */
    while (c != EOF && c != '\n') {
        buf_push(*chars, (char) c);
        c = getchar();
    }
    
    return buf_len(*chars);
}

// TODO: Test on other platforms!
// Returns NULL when canceled and a buffer of length 0 when Ctrl-Z with no input
// TODO: Add a pointer to a bool (int) that will be set to true when Ctrl-Z was typed.
char *getInput(void) {
    char *inputBuffer = NULL;
    
    int c;
    int currentIndex = 0;
    while ((c = getch()) != 26) { // Ctrl-Z
        if (c == 0 || c == 224) {
            int special = getch();
            if (special == 75) { // Left arrow
                if (currentIndex != 0) {
                    if (inputBuffer[currentIndex - 1] == '\t')
                        fputs("\b\b\b\b", stdout);
                    else putchar('\b');
                    --currentIndex;
                }
            } else if (special == 77) { // Right arrow
                if (currentIndex < buf_len(inputBuffer)) {
                    if (inputBuffer[currentIndex] == '\t')
                        fputs("    ", stdout);
                    else putchar(inputBuffer[currentIndex]);
                    ++currentIndex;
                }
            } else if (special == 83) { // Delete
                if (currentIndex < buf_len(inputBuffer)) {
                    // Move all the characters down one
                    char *source = &(inputBuffer[currentIndex + 1]);
                    char *destination = &(inputBuffer[currentIndex]);
                    int amtToMove = buf_end(inputBuffer) - source;
                    memmove(destination, source, sizeof(char) * amtToMove);
                    buf_pop(inputBuffer);
                    // Print all of the characters that have been moved
                    for (int i = currentIndex; i < buf_len(inputBuffer); i++) {
                        if (inputBuffer[i] == '\t')
                            fputs("    ", stdout);
                        else putchar(inputBuffer[i]);
                    }
                    // Print spaces where the last character(s) used to be
                    fputs("    \b\b\b\b", stdout);
                    // Go back to where the cursor is
                    for (int i = 0; i < buf_len(inputBuffer) - currentIndex; i++) {
                        if (inputBuffer[buf_len(inputBuffer) - 1 - i] == '\t')
                            fputs("\b\b\b\b", stdout);
                        else putchar('\b');
                    }
                }
            } else if (special == 79) { // End key
                for (int i = currentIndex; i < buf_len(inputBuffer); i++) {
                    if (inputBuffer[i] == '\t')
                        fputs("    ", stdout);
                    else putchar(inputBuffer[i]);
                }
                currentIndex = buf_len(inputBuffer);
            } else if (special == 71) { // Home key
                for (int i = 0; i < currentIndex; i++) {
                    if (inputBuffer[i] == '\t')
                        fputs("\b\b\b\b", stdout);
                    else putchar('\b');
                }
                currentIndex = 0;
            } else if (special == 115) { // Ctrl+Left // TODO
            } else if (special == 116) { // Ctrl+Right // TODO
            } else {
                //printf("%d", special); // For debugging
            }
            continue;
        }
        /*if (c == 127) { // Delete // TODO: Detect Tab character
        
            continue;
        }*/
        if (c == 24) { // Cancel
            //buf_pop_all(inputBuffer);
            buf_free(inputBuffer);
            inputBuffer = NULL;
            currentIndex = 0;
            return NULL;
        }
        if (c == '\n' || c == '\r') { // Enter
#ifdef _WIN32
            putchar('\r');
#endif
            putchar('\n');
            buf_push(inputBuffer, '\n');
            ++currentIndex;
            break;
        }
        if (c == '\t') { // Tab
            if (currentIndex == buf_len(inputBuffer)) {
                fputs("    ", stdout);
                buf_push(inputBuffer, '\t');
            } else {
                fputs("    ", stdout);
                buf_add(inputBuffer, 1);
                // Move all characters up one
                char *source = &(inputBuffer[currentIndex]);
                char *destination = &(inputBuffer[currentIndex + 1]);
                int amtToMove = &(inputBuffer[buf_len(inputBuffer) - 1]) - source;
                memmove(destination, source, sizeof(char) * amtToMove);
                // Change the character in the inputBuffer
                inputBuffer[currentIndex] = c;
                // Print the characters
                for (int i = currentIndex + 1; i < buf_len(inputBuffer); i++) {
                    if (inputBuffer[i] == '\t')
                        fputs("    ", stdout);
                    else putchar(inputBuffer[i]);
                }
                // Move back to where the cursor is
                for (int i = 0; i < buf_len(inputBuffer) - currentIndex - 1; i++) {
                    if (inputBuffer[buf_len(inputBuffer) - 1 - i] == '\t')
                        fputs("\b\b\b\b", stdout);
                    else putchar('\b');
                }
            }
            
            ++currentIndex;
            continue;
        }
        if (c == '\b') { // Backspace
            if (currentIndex > 0) {
                if (currentIndex == buf_len(inputBuffer)) {
                    if (inputBuffer[currentIndex - 1] == '\t')
                        fputs("\b\b\b\b", stdout);
                    else fputs("\b \b", stdout);
                    buf_pop(inputBuffer);
                    --currentIndex;
                    continue;
                }
                
                if (inputBuffer[currentIndex - 1] == '\t')
                    fputs("\b\b\b\b", stdout);
                else fputs("\b \b", stdout);
                
                // Move all characters down one
                char *source = &(inputBuffer[currentIndex]);
                char *destination = &(inputBuffer[currentIndex - 1]);
                int amtToMove = buf_end(inputBuffer) - source;
                memmove(destination, source, sizeof(char) * amtToMove);
                
                buf_pop(inputBuffer);
                
                // Print all of the characters again
                for (int i = currentIndex - 1; i < buf_len(inputBuffer); i++) {
                    if (inputBuffer[i] == '\t')
                        fputs("    ", stdout);
                    else if (inputBuffer[i] == '\n' || inputBuffer[i] == '\r')
                        ; // Do Nothing
                    else putchar(inputBuffer[i]);
                }
                
                // Print spaces where the last character(s) used to be
                fputs("    \b\b\b\b", stdout);
                
                // Move back to where the cursor is
                for (int i = 0; i <= buf_len(inputBuffer) - currentIndex; i++) {
                    if (inputBuffer[buf_len(inputBuffer) - 1 - i] == '\t')
                        fputs("\b\b\b\b", stdout);
                    else putchar('\b');
                }
                
                --currentIndex;
            }
            continue;
        }
        
        if (currentIndex == buf_len(inputBuffer)) {
            putchar(c);
            buf_push(inputBuffer, c);
        } else {
            putchar(c);
            buf_add(inputBuffer, 1);
            // Move all characters up one
            char *source = &(inputBuffer[currentIndex]);
            char *destination = &(inputBuffer[currentIndex + 1]);
            int amtToMove = &(inputBuffer[buf_len(inputBuffer) - 1]) - source;
            memmove(destination, source, sizeof(char) * amtToMove);
            // Change the character in the inputBuffer
            inputBuffer[currentIndex] = c;
            // Print the characters
            for (int i = currentIndex + 1; i < buf_len(inputBuffer); i++) {
                if (inputBuffer[i] == '\t')
                    fputs("    ", stdout);
                else putchar(inputBuffer[i]);
            }
            // Move back to where the cursor is
            for (int i = 0; i < buf_len(inputBuffer) - currentIndex - 1; i++) {
                if (inputBuffer[buf_len(inputBuffer) - 1 - i] == '\t')
                    fputs("\b\b\b\b", stdout);
                else putchar('\b');
            }
        }
        ++currentIndex;
    }
    
    //printf("Result: %.*s\n", (int) buf_len(inputBuffer), inputBuffer);
    
    return inputBuffer;
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
    switch (currentBuffer->fileType) {
        case FT_MARKDOWN:
        {
            createMarkdownOutline();
        } break;
        case FT_C:
        {
            createCOutline();
        } break;
    }
}

// Currently, after every operation that modifies the file, the outline is recreated.
// TODO: Perhaps it would be more efficient to only change the line a node points to when
// that line is moved/changed by using some type of hash table for integer keys.
void recreateOutline(void) {
    switch (currentBuffer->fileType) {
        case FT_MARKDOWN:
        {
            // Pop off all of the current nodes
            if (buf_len(currentBuffer->outline.markdown_nodes) > 0)
                buf_pop_all(currentBuffer->outline.markdown_nodes);
            assert(buf_len(currentBuffer->outline.markdown_nodes) == 0);
            createMarkdownOutline();
        } break;
        case FT_C:
        {
            // Pop off all of the current nodes
            if (buf_len(currentBuffer->outline.c_nodes))
                buf_pop_all(currentBuffer->outline.c_nodes);
            assert(buf_len(currentBuffer->outline.c_nodes) == 0);
            createCOutline();
        } break;
    }
}

void showOutline(void) {
    switch (currentBuffer->fileType) {
        case FT_MARKDOWN:
        {
            showMarkdownOutline();
        } break;
        case FT_C:
        {
            showCOutline();
        } break;
    }
}

void createMarkdownOutline(void) {
    assert(currentBuffer->fileType == FT_MARKDOWN);
    
    // Go through each line
    for (int line = 0; line < buf_len(currentBuffer->lines); line++) {
        // If starts with a hash, then it's a heading
        if (currentBuffer->lines[line].chars[0] == '#') {
            int level = 0;
            // Increment level with each successive '#'
            for (int i = 1; i < buf_len(currentBuffer->lines[line].chars); i++) {
                if (currentBuffer->lines[line].chars[i] == '#') {
                    level++;
                } else break;
            }
            
            // create the node and push it
            MarkdownOutlineNode node;
            node.line = &(currentBuffer->lines[line]);
            node.lineNum = line;
            node.level = level;
            
            buf_push(currentBuffer->outline.markdown_nodes, node);
        }
    }
}

// TODO: This will be greatly improved once I have a lexer
// and some general parser utils
void createCOutline(void) {
    assert(currentBuffer->fileType == FT_C);
    
    // Go through each line
    for (int line = 0; line < buf_len(currentBuffer->lines); line++) {
        char *start = &(currentBuffer->lines[line].chars[0]);
        char *current = start;
        int lineLength = buf_len(currentBuffer->lines[line].chars);
        
        // Skip whitespace
        while ((current - start < lineLength) && *current == ' ' || *current == '\t') {
            ++current;
        }
        
        // Support optional 'internal' or 'static' before function declaration
        // TODO: This is hacky
        if (*current == 'i' && *(current + 1) == 'n' && *(current + 2) == 't' && *(current + 3) == 'e' && *(current + 4) == 'r' && *(current + 5) == 'n' && *(current + 6) == 'a' && *(current + 7) == 'l' && *(current + 8) == ' ')
        {
            current += 8;
            
            // Skip whitespace
            while ((current - start < lineLength) &&*current == ' ' || *current == '\t') ++current;
        } else if (*current == 's') {
            char str[7] = "static ";
            int i = 0;
            int startsWithStatic = true;
            while (i < 7 && (current + i) - start < lineLength) {
                if (*(current + i) != str[i]) {
                    startsWithStatic = false;
                    break;
                }
                ++i;
            }
            if (startsWithStatic) {
                current += 7;
            }
            
            // Skip whitespace
            while ((current - start < lineLength) &&*current == ' ' || *current == '\t') ++current;
        }
        
        int isDeclaration = true;
        switch(*current) {
            case 'v':
            {
                char str[5] = "void ";
                int i = 0;
                while (i < 5 && current - start < lineLength) {
                    if (*current != str[i]) {
                        isDeclaration = false;
                        break;
                    }
                    ++current;
                    ++i;
                }
            } break;
            case 'i':
            {
                char str[4] = "int ";
                int i = 0;
                while (i < 4 && current - start < lineLength) {
                    if (*current != str[i]) {
                        isDeclaration = false;
                        break;
                    }
                    ++current;
                    ++i;
                }
            } break;
            case 'f':
            {
                char str[6] = "float ";
                int i = 0;
                while (i < 6 && current - start < lineLength) {
                    if (*current != str[i]) {
                        isDeclaration = false;
                        break;
                    }
                    ++current;
                    ++i;
                }
            } break;
            case 'd':
            {
                char str[7] = "double ";
                int i = 0;
                while (i < 7 && current - start < lineLength) {
                    if (*current != str[i]) {
                        isDeclaration = false;
                        break;
                    }
                    ++current;
                    ++i;
                }
            } break;
            case 'b':
            {
                char str[5] = "bool ";
                int i = 0;
                while (i < 5 && current - start < lineLength) {
                    if (*current != str[i]) {
                        isDeclaration = false;
                        break;
                    }
                    ++current;
                    ++i;
                }
            } break;
            case 'c':
            {
                char str[5] = "char ";
                int i = 0;
                while (i < 5 && current - start < lineLength) {
                    if (*current != str[i]) {
                        isDeclaration = false;
                        break;
                    }
                    ++current;
                    ++i;
                }
            } break;
            default:
            {
                // Commented this because currently checking for parentheses to only allow function declarations, and this will allow us to show all functions that return types that aren't primitive
                //isDeclaration = false;
                
                // Check that there's a space between the type and the function name
                // Skip all characters except for space
                while (current - start < lineLength) {
                    if (*current == ' ' || *current == '\t') {
                        break;
                    }
                    ++current;
                }
                // Make sure not at end of line
                if (current - start >= lineLength)
                    isDeclaration = false;
                // Make sure there's at least one space
                if (*current != ' ' && *current != '\t') isDeclaration = false;
            } break;
        }
        
        if (isDeclaration == true) {
            int isFunctionDeclaration = false;
            
            // Skip whitespace
            while (*current == ' ' || *current == '\t') ++current;
            
            // Make sure there's at least one character for the function name
            if (*current != '(' && *current != ')' && *current != '=' && *current != '"' && *current != '\'' && *current != ',' && current - start < lineLength) {
                ++current;
                
                // Skip all characters except for left parentheses and equals
                // Don't skip whitespace, function names can't have whitespace
                // TODO: There can be a whitespace between the function name and the left parentheses - this isn't checking for that yet.
                while (current - start < lineLength) {
                    if (*current == '(') {
                        isFunctionDeclaration = true;
                        break;
                    } else if (*current == '=' || *current == ',' || *current == '"' || *current == '\'' || *current == ' ' || *current == '\t') { // Don't allow certain characters in function names
                        isFunctionDeclaration = false;
                        break;
                    }
                    ++current;
                }
                int foundRightParen = false;
                // Skip all characters except for right parentheses
                while (current - start < lineLength) {
                    if(*current == ')') {
                        foundRightParen = true;
                        ++current;
                        break;
                    }
                    if (*current == '=') {
                        isFunctionDeclaration = false;
                        break;
                    }
                    ++current;
                }
                
                // If reached end without finding the right parentheses, it's (perhaps) not a function declaration
                if (!foundRightParen) {
                    isFunctionDeclaration = false;
                } else {
                    // Skip whitespace
                    while (current - start < lineLength && (*current == ' ' || *current == '\t')) ++current;
                    
                    // Check if next character is '{', if not, check next line
                    if (*current == '{' && current - start < lineLength) {
                        isFunctionDeclaration = true;
                    } else {
                        // Check next line
                        char *startNextLine = &(currentBuffer->lines[line + 1].chars[0]);
                        char *currentNextLine = startNextLine;
                        
                        // Skip whitespace
                        while (*currentNextLine == ' ' || *currentNextLine == '\t') ++currentNextLine;
                        // Check that first non-whitespace character of next line is '{'
                        if (*currentNextLine == '{') {
                            isFunctionDeclaration = true;
                        } else {
                            isFunctionDeclaration = false;
                        }
                    }
                }
            }
            
            // Only add Function declarations
            if (isFunctionDeclaration) {
                COutlineNode node;
                node.line = &(currentBuffer->lines[line]);
                node.lineNum = line;
                
                buf_push(currentBuffer->outline.c_nodes, node);
            }
        }
    }
}

void showMarkdownOutline(void) {
    assert(currentBuffer->fileType == FT_MARKDOWN);
    
    // Go though each node
    for (int node_i = 0; node_i < buf_len(currentBuffer->outline.markdown_nodes); node_i++) {
        int linenum = currentBuffer->outline.markdown_nodes[node_i].line - currentBuffer->lines;
        printLine(linenum, 0, true);
        // Print out the line
        //printLine(currentBuffer->outline.markdown_nodes[node_i].lineNum, 0, true);
    }
}

void showCOutline(void) {
    assert(currentBuffer->fileType == FT_C);
    
    // Go through each node
    for (int node_i = 0; node_i < buf_len(currentBuffer->outline.c_nodes); node_i++) {
        int linenum = currentBuffer->outline.c_nodes[node_i].line - currentBuffer->lines;
        printLine(linenum, 0, true);
    }
}
