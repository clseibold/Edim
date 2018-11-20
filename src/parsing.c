#include "edimcoder.h"

// Gives back pointer starting at next non-whitespace character that appears after start
char *skipWhitespace(char *start, char *endBound) {
    char *current = start;
    while (*current == ' ' || *current == '\t' || *current == '\n' || *current == '\r') {
        ++current;
        if (current > endBound) break;
    }
    return current;
}

// Will give back a pointer just after the word skipped.
char *skipWord(char *start, char *endBound, bool includeNumbers, bool includeSymbols) {
    char *current = start;
    while ((*current >= 'A' && *current <= 'Z') || (*current >= 'a' && *current <= 'z')
           || (includeNumbers && *current >= '0' && *current <= '9')
           || (includeNumbers && *current == '-') // Note: Technically this will also match numbers that have '-' in the middle and at end, but whatever TODO: Fix this
           || (includeSymbols && *current >= '!' && *current <= '/')
           || (includeSymbols && *current >= ':' && *current <= '@')
           || (includeSymbols && *current >= '[' && *current <= '`')
           || (includeSymbols && *current >= '{' && *current <= '~')) {
        ++current;
        if (current > endBound) break;
    }
    return current;
}

// TODO
char *skipText() {
    return NULL;
}

// TODO
char *skipSymbols() {
    return NULL;
}

char *skipNumbers(char *start, char *endBound) {
    char *current = start;
    if (*current == '-') ++current; // Note: Not doing '+' here
    while (*current >= '0' && *current <= '9') {
        ++current;
        if (current > endBound) break;
    }
    return current;
}

// Specific to Edim Command Language
char *skipLineNumber(char *start, char *endBound) {
    char *current = start;
    if (*current == '\'') {
        ++current;
        current = skipWord(current, endBound, true, false);
    } else {
        // If symbol
        if ((*current >= '!' && *current <= '/')
            || (*current >= ':' && *current <= '@')
            || (*current >= '[' && *current <= '`')
            || (*current >= '{' && *current <= '~')) {
            //++current;
            if (*current == '.' || *current == '$') // Hacky, TODO
                ++current;
        } else {
            current = skipNumbers(current, endBound);
        }
    }
    return current;
}

// Returns '0' if number could not be parsed
long parseLineNumber(Buffer *buffer, char *start, char *endBound) {
    char *current = start;
    current = skipWhitespace(current, endBound);
    pString lineNumber;
    lineNumber.start = current;
    current = skipLineNumber(current, endBound);
    if (current == lineNumber.start) return 0; // No number found
    lineNumber.end = current;
    
    // Special Line Number Symbols
    if (lineNumber.end - lineNumber.start == 1) {
        switch (lineNumber.start[0]) {
            case '0':
            if (buf_len(buffer->lines) == 0)
                return 0;
            else return 1;
            case '1':
            return 1;
            case '-':
            return 0;
            case '$':
            return buf_len(buffer->lines);
            case '.':
            return buffer->currentLine;
        }
    }
    
    // Turn number string into long integer
    int result = 0;
    bool negative = false;
    current = lineNumber.start;
    if (*current == '-') {
        current++;
        negative = true;
    }
    while (*current >= '0' && *current <= '9') {
        result *= 10;
        result += *current - '0';
        ++current;
        if (current == lineNumber.end + 1) break;
    }
    
    if (negative && result > 0) {
        result *= -1;
    }
    
    return result;
}

/*int parseLineNumber() {

}*/

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

// TODO: Test on macOS and BSD!
// TODO: bool printNewLine
// TODO: Placeholder/Ghost text
// TODO: Autocomplete?
char *getInput(bool *canceled, char *inputBuffer, inputKeyCallback callback) {
    (*canceled) = false;
    int currentIndex = 0;
    int defaultLength = buf_len(inputBuffer);
    
    if (inputBuffer != NULL && buf_len(inputBuffer) > 0) {
        for (int i = 0; i < buf_len(inputBuffer); i++) {
            if (inputBuffer[i] == '\t')
                fputs("    ", stdout);
            else if (inputBuffer[i] == INPUT_ESC)
                colors_printf(COLOR_RED, "$");
            else putchar(inputBuffer[i]);
            ++currentIndex;
        }
    }
    
    char c;
    while ((c = getch()) != INPUT_ENDINPUT) { // Ctrl-Z for Windows, Ctrl-D for Linux
#ifdef _WIN32
        if (c == INPUT_SPECIAL1 || c == INPUT_SPECIAL2)
#else
        if (c == INPUT_SPECIAL1)
#endif
        {
#ifndef _WIN32
            if (getch_nonblocking() == INPUT_SPECIAL2) {
#endif
                if (callback != NULL) {
                    if (!callback(c, true, &inputBuffer, &currentIndex))
                        continue;
                }
                
                int specialkey = getch();
                if (specialkey == INPUT_LEFT) { // Left arrow
                    if (currentIndex != 0) {
                        if (inputBuffer[currentIndex - 1] == '\t')
                            fputs("\b\b\b\b", stdout);
                        else putchar('\b');
                        --currentIndex;
                    }
                } else if (specialkey == INPUT_RIGHT) { // Right arrow
                    if (currentIndex < buf_len(inputBuffer)) {
                        if (inputBuffer[currentIndex] == '\t')
                            fputs("    ", stdout);
                        else if (inputBuffer[currentIndex] == INPUT_ESC)
                            colors_printf(COLOR_RED, "$");
                        else putchar(inputBuffer[currentIndex]);
                        ++currentIndex;
                    }
#ifdef _WIN32
                } else if (specialkey == 83) { // Delete
#else
                } else if (specialkey == INPUT_DELETE1) {
                    int special2 = getch();
                    if (special2 == INPUT_DELETE2) {
#endif
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
                                if (inputBuffer[i] == INPUT_ESC)
                                    colors_printf(COLOR_RED, "$");
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
#ifndef _WIN32
                    }
#endif
                } else if (specialkey == INPUT_END) { // End key
                    for (int i = currentIndex; i < buf_len(inputBuffer); i++) {
                        if (inputBuffer[i] == '\t')
                            fputs("    ", stdout);
                        else if (inputBuffer[i] == INPUT_ESC)
                            colors_printf(COLOR_RED, "$");
                        else putchar(inputBuffer[i]);
                    }
                    currentIndex = buf_len(inputBuffer);
                } else if (specialkey == INPUT_HOME) { // Home key
                    for (int i = 0; i < currentIndex; i++) {
                        if (inputBuffer[i] == '\t')
                            fputs("\b\b\b\b", stdout);
                        else putchar('\b');
                    }
                    currentIndex = 0;
                    //} else if (special == 115) { // Ctrl+Left // TODO
                    //} else if (special == 116) { // Ctrl+Right // TODO
                } else {
                    //printf("%d", special); // For debugging
                }
                continue;
#ifndef _WIN32
            } else { // TODO: Add to the location of the cursor
                // If escape key was hit and wasn't a special key
                // (most likely ESC hit by itself)
                // then print $ with next character after, and add
                // to buffer ESC + next character
                colors_printf(COLOR_RED, "$");
                //printf("%c", c);
                buf_push(inputBuffer, 27);
                ++currentIndex;
                //buf_push(inputBuffer, c);
                continue;
            }
#endif
        }
        
        if (callback != NULL) {
            if (!callback(c, false, &inputBuffer, &currentIndex))
                continue;
        }
        
        // ASCII Control Keys
        if (c == INPUT_CTRL_X) {
            //buf_pop_all(inputBuffer);
            fputs("^X", stdout);
            buf_free(inputBuffer);
            inputBuffer = NULL;
            currentIndex = 0;
            (*canceled) = true;
            return NULL;
        } else if (c == INPUT_CTRL_C) {
            exit(0); // TODO
        } else if (c > 0 && c <= 26 && c != 13 && c != 8 && c != 9 && c != 10) { // Capture all other Control Keys (aside from Enter, backspace, and tab)
            continue;
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
                    else if (inputBuffer[i] == INPUT_ESC)
                            colors_printf(COLOR_RED, "$");
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
        if (c == INPUT_BACKSPACE)
        {
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
                    else if (inputBuffer[i] == INPUT_ESC)
                            colors_printf(COLOR_RED, "$");
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

#ifdef _WIN32
        if (c == INPUT_ESC) {
            colors_printf(COLOR_RED, "$");
            buf_push(inputBuffer, c);
            continue;
        }
#endif
        
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
                else if (inputBuffer[i] == INPUT_ESC)
                    colors_printf(COLOR_RED, "$");
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
    
    // If no input was made, free the buffer and return;
    if (buf_len(inputBuffer) - defaultLength <= 0) {
        if (inputBuffer != NULL)
            buf_free(inputBuffer);
    }
    
    // If there's no new line - which there shouldn't be -
    // then add it.
    if (inputBuffer != NULL && *(buf_end(inputBuffer) - 1) != '\n') {
        buf_add(inputBuffer, '\n');
    }
    
    if (buf_len(inputBuffer) == 0) {
#ifdef _WIN32
        printf("^Z");
#else
        printf("^D");
#endif
    }
    
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
            while (current - start < lineLength && (*current == ' ' || *current == '\t')) ++current;
        } else if (*current == 'i' && *(current + 1) == 'n' && *(current + 2) == 'l' && *(current + 3) == 'i' && *(current + 4) == 'n' && *(current + 5) == 'e' && *(current + 6) == ' ') {
            current += 7;
            
            // Skip whitespace
            while (current - start < lineLength && (*current == ' ' || *current == '\t'))
                ++current;
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
            while (current - start < lineLength && (*current == ' ' || *current == '\t')) ++current;
        } else if (*current == 'c') {
            char str[6] = "const ";
            int i = 0;
            int startsWithConst = true;
            while (i < 6 && (current + i) - start < lineLength) {
                if (*(current + i) != str[i]) {
                    startsWithConst = false;
                    break;
                }
                ++i;
            }
            if (startsWithConst) {
                current += 6;
            }
            
            // Skip whitespace
            if (current - start < lineLength && (*current == ' ' || *current == '\t'))
                ++current;
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

/* === Bookmarks === */

bool get_bookmark(Buffer *buffer, pString name, Bookmark **result_bookmark) {
    // Go through all bookmarks in array to see matching name
    //Bookmark *result_bookmark;
    int name_length = name.end - name.start;
    bool found = false;
    for (int i = 0; i < buf_len(buffer->bookmarks); i++) {
        Bookmark bookmark = buffer->bookmarks[i];
        if (strncmp(bookmark.name, name.start, MAX(buf_len(bookmark.name), name_length)) == 0) { // strncmp(command.start, "clear", maxChars) == 0
            (*result_bookmark) = &buffer->bookmarks[i];
            found = true;
            break;
        }
    }

    return found;
}

// TODO: Accept the buffer to get bookmarks array
bool add_bookmark(Buffer *buffer, pString name, lineRange range) {
    int name_length = name.end - name.start;

    // Check if bookmark already exists
    Bookmark *result_bookmark;
    bool found = get_bookmark(buffer, name, &result_bookmark);

    if (found) {
        // If already exists, update the range
        result_bookmark->range.start = range.start;
        result_bookmark->range.end = range.end;
    } else {
        // Otherwise, create and add the bookmark
        Bookmark bookmark;
        bookmark.name = NULL;
        buf_add(bookmark.name, name_length);
        for (int i = 0; i < name_length; i++) {
            bookmark.name[i] = name.start[i];
            //buf_push(bookmark.name, name.start[i]);
        }

        bookmark.range.start = range.start;
        bookmark.range.end = range.end;

        buf_push(buffer->bookmarks, bookmark);
    }

    return found;
}
