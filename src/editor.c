#include "edimcoder.h"

internal void editorState_openAnotherFile(char *rest, int restLength);
internal void editorState_openNewFile(char *rest, int restLength);

internal int getLineNumber();
internal int checkLineNumber(int original_line);

internal void editorState_printHelpScreen();

internal void editorState_insertAfter(lineRange line_range);
internal void editorState_insertBefore(lineRange line_range);
internal void editorState_appendTo(lineRange line_range);
internal void editorState_prependTo(lineRange line_range);
internal void editorState_replaceLine(lineRange line_range);
internal void editorState_replaceString(lineRange line_range, char *rest, int restLength);

internal void editorState_findStringInLine(char *rest, int restLength);
internal void editorState_findStringInFile(char *rest, int restLength);
internal void editorState_deleteLine(lineRange line_range);
internal void editorState_moveUp(lineRange line_range);
internal void editorState_moveDown(lineRange line_range);

internal bool commandInputCallback(char c, bool isSpecial, char **inputBuffer, int *currentIndex) {
    bool bufferEmpty = false;
    if (*inputBuffer == NULL || buf_len(*inputBuffer) == 0)
        bufferEmpty = true;
    if (!isSpecial && c == INPUT_CTRL_L) {
        clrscr();
        
        // Reprint prompt
        if (buf_len(currentBuffer->openedFilename) > 0) {
            // TODO: This will also print out the directory, so I should get rid of everything before the last slash
            if (currentBuffer->modified) {
                printPrompt("\n<%d: %.*s*|%d> ", currentBuffer - buffers, (int) buf_len(currentBuffer->openedFilename), currentBuffer->openedFilename, currentBuffer->currentLine);
            } else {
                printPrompt("\n<%d: %.*s|%d> ", currentBuffer - buffers, (int) buf_len(currentBuffer->openedFilename), currentBuffer->openedFilename, currentBuffer->currentLine);
            }
            
        } else printPrompt("\n<%d: new file*|%d> ", currentBuffer - buffers, currentBuffer->currentLine);
        return false;
    } else if (!isSpecial && c == INPUT_CTRL_O && bufferEmpty) {
        char *str = "o ";
        for (int i = 0; i < strlen(str); i++)
            buf_push(*inputBuffer, str[i]);
        printf("%s", str);
        (*currentIndex) += strlen(str);
        return false;
    } else if (!isSpecial && c == '\t') { // TODO: Command Autocompletion
        // Look at all characters
        // Find a command that starts with those characters
        // and has the least amount of *aditional* characters.
        return false;
    }
    
    /*else if (!isSpecial && c == 'i' && bufferEmpty) {
        char *str = "i ";
        for (int i = 0; i < strlen(str); i++)
            buf_push(*inputBuffer, str[i]);
        printf("%s", str);
        (*currentIndex) += strlen(str);
        return false;
    }*/
    
    if (!isSpecial && bufferEmpty) {
        char *str = malloc(3 * sizeof(char));
        str[0] = 'h';
        str[1] = ' ';
        str[2] = '\0';
        switch (c) {
            case 'i':
            str[0] = 'i'; break;
            case 'a':
            str[0] = 'a'; break;
            case 'I':
            str[0] = 'I'; break;
            case 'A':
            str[0] = 'A'; break;
            case 'r':
            str[0] = 'r'; break;
            case 'R':
            str[0] = 'R'; break;
            case 'j':
            str[0] = 'j'; break;
            case 'p':
            str[0] = 'p'; break;
            case 'P':
            str[0] = 'P'; break;
            case 'x':
            str[0] = 'x'; break;
            case 'm':
            str[0] = 'm'; break;
            case 'M':
            str[0] = 'M'; break;
            /*case 'f':
            str[0] = 'f'; break;
            case 'F':
            str[0] = 'F'; break;*/
            case 'u':
            str[0] = 'u'; break;
            case 'n':
            str[0] = 'n'; break;
            case 'b':
            str[0] = 'b'; break;
            case 'o':
            str[0] = 'o'; break;
            case 's':
            str[0] = 's'; break;
            case 'e':
            str[0] = 'e'; break;
            case 'E':
            str[0] = 'E'; break;
            case 'q':
            str[0] = 'q'; break;
            case 'Q':
            str[0] = 'Q'; break;
            // --
            case 'h':
            {
                char *str2 = "help ";
                for (int i = 0; i < strlen(str2); i++)
                    buf_push(*inputBuffer, str2[i]);
                printf("%s", str2);
                (*currentIndex) += strlen(str2);
                free(str); str = NULL;
            } return false;
            case '#':
            {
                char *str2 = "info ";
                for (int i = 0; i < strlen(str2); i++)
                    buf_push(*inputBuffer, str2[i]);
                printf("%s", str2);
                (*currentIndex) += strlen(str2);
                free(str); str = NULL;
            } return false;
            // --
            default:
            free(str); str = NULL;
            return true;
        }
        printf("%s", str);
        for (int i = 0; i < strlen(str); i++)
            buf_push(*inputBuffer, str[i]);
        (*currentIndex) += strlen(str);
        free(str); str = NULL;
        return false;
    } else if (!isSpecial) {
        char *str = malloc(3 * sizeof(char));
        str[0] = '$';
        str[1] = ' ';
        str[2] = '\0';
        //char *str = "$ ";
        switch (c) {
            case '$':
            str[0] = '$'; break;
            case '^':
            str[0] = '^'; break;
            /*case '0': // Check that no other numbers before (aside from space)
            str[0] = '0'; break;*/
            default:
            free(str); str = NULL;
            return true;
        }
        for (int i = 0; i < strlen(str); i++)
            buf_push(*inputBuffer, str[i]);
        printf("%s", str);
        (*currentIndex) += strlen(str);
        free(str); str = NULL;
        return false;
    }
    
    return true;
}

/* Menu for Editor */
State editorState_menu(void) {
    /* Prompt */
    if (buf_len(currentBuffer->openedFilename) > 0) {
        // TODO: This will also print out the directory, so I should get rid of everything before the last slash
        if (currentBuffer->modified) {
            printPrompt("\n<%d: %.*s*|%d> ", currentBuffer - buffers, (int) buf_len(currentBuffer->openedFilename), currentBuffer->openedFilename, currentBuffer->currentLine);
        } else {
            printPrompt("\n<%d: %.*s|%d> ", currentBuffer - buffers, (int) buf_len(currentBuffer->openedFilename), currentBuffer->openedFilename, currentBuffer->currentLine);
        }
        
    } else printPrompt("\n<%d: new file*|%d> ", currentBuffer - buffers, currentBuffer->currentLine);
    
    char *input = NULL;
    bool canceled = false;
    input = getInput(&canceled, input, commandInputCallback);
    if (canceled || input == NULL || buf_len(input) == 0 || (buf_len(input) == 1 && input[0] == '\n')) {
        buf_free(input);
        return KEEP;
    }
    
    char *current = input;
    
    // Parse first word for command
    // TODO: While loop should start here for looping through multiple commands (separated by semicolon)
    current = skipWhitespace(current, buf_end(input));
    
    pString command;
    command.start = current;
    current = skipWord(current, buf_end(input), false, false);
    command.end = current;
    if (command.start == command.end) {
        // TODO: Read symbol?
    }
    
    // Skip Whitespace
    current = skipWhitespace(current, buf_end(input));
    
    printf("\n");
    
    // String Comparisons - I believe this is inefficient
    // TODO: Look into string interning
    size_t maxChars = (command.end - command.start);
    //printf("'%.*s'", maxChars, command.start);
    if (strncmp(command.start, "clear", 5) == 0) {
        clrscr();
        buf_free(input);
        return KEEP;
    } else if (strncmp(command.start, "help", 4) == 0) {
        editorState_printHelpScreen();
        buf_free(input);
        return KEEP;
    } else if (strncmp(command.start, "info", 4) == 0) {
        printFileInfo();
        buf_free(input);
        return KEEP;
    }

    // TODO: Interpret variable for line range
    int line_start = (int) parseLineNumber(currentBuffer, current, buf_end(input));
    current = skipLineNumber(current, buf_end(input));
    current = skipWhitespace(current, buf_end(input));

    // The line range will start and end at one line if only one integer/line give (and line_range_length is 0).
    // If two given (a range is give), then starts and ends based on those two
    // line numbers given (and line_range_length > 0).
    lineRange line_range;
    line_range.start = line_start;
    line_range.end = line_start;
    int line_range_length = line_range.end - line_range.start;

    if (*current == ':') {
        current++;
        current = skipWhitespace(current, buf_end(input));
        int line_end = (int) parseLineNumber(currentBuffer, current, buf_end(input));
        current = skipLineNumber(current, buf_end(input));
        line_range.end = line_end;
        line_range_length = line_range.end - line_range.start;
        current = skipWhitespace(current, buf_end(input));
    }

    // If no line number/range given, then check for a bookmark name (denoted
    // by a # at the start)
    if (line_range.start == 0 && line_range.end == 0) {
        if (*current == '#') {
            ++current;
            
            pString bookmarkName;
            bookmarkName.start = NULL;
            bookmarkName.end = NULL;
            int bookmarkName_length = 0;

            bookmarkName.start = current;
            current = skipWord(current, buf_end(input), true, false);
            bookmarkName.end = current;
            bookmarkName_length = bookmarkName.end - bookmarkName.start;
            current = skipWhitespace(current, buf_end(input));

            // Bookmarks for pages of the current file
            if (bookmarkName.start[0] == 'p') {
                char *current_in_bookmarkName = bookmarkName.start + 1;
                pString pageText;
                pageText.start = current_in_bookmarkName;
                current_in_bookmarkName = skipNumbers(current_in_bookmarkName, bookmarkName.end);
                pageText.end = current_in_bookmarkName;

                int pageNum = 0;
                char *current_in_pageText = pageText.start;
                while (*current_in_pageText >= '0' && *current_in_pageText <= '9') {
                    pageNum *= 10;
                    pageNum += *current_in_pageText - '0';
                    ++current_in_pageText;
                    if (current_in_pageText == pageText.end + 1) break;
                }

                int lineNum = (pageNum - 1) * 15;
                int lineNum_end = lineNum + 15;
                if (lineNum < 1) {
                    lineNum = 1;
                    lineNum_end = 15;
                }
                if (lineNum > buf_len(currentBuffer->lines)) lineNum = buf_len(currentBuffer->lines) - 15;
                line_range.start = lineNum;
                line_range.end = lineNum_end;
                line_range_length = line_range.end - line_range.start;
            } else {
                Bookmark *result_bookmark;
                bool found = get_bookmark(currentBuffer, bookmarkName, &result_bookmark);

                if (found) {
                    line_range.start = result_bookmark->range.start;
                    line_range.end = result_bookmark->range.end;
                    line_range_length = line_range.end - line_range.start;
                }
            }
        }
    }

    char *rest = current;
    int boundSize = buf_len(input) - (current - input);
    int restLength = boundSize;
    
    switch (command.start[0]) {
        case 'j':
        {
            int line = line_range.start;
            
            if (!(line == 0 && buf_len(currentBuffer->lines) == 0))
                line = checkLineNumber(line);

            if (line_range_length == 0) {
                if (line - 2 >= 0)
                    printLine(line - 2, 0, true);
                printLine(line - 1, '*', true);
                if (line < buf_len(currentBuffer->lines))
                    printLine(line, 0, true);
            } else {
                int endLine = line_range.end;
                if (endLine == 0 && buf_len(currentBuffer->lines) != 0) {
                    if (endLine == 0) line = currentBuffer->currentLine;
                }
                
                // Make sure going from low to high (forwards)
                if (endLine < line) {
                    int tmp = endLine;
                    endLine = line;
                    line = tmp;
                }
                // Print the line before
                if (line - 2 >= 0)
                    printLine(line - 2, 0, true);
                // Print the range of lines
                for (int i = line; i <= endLine; i++) {
                    if (i - 1 >= 0 && i - 1 < buf_len(currentBuffer->lines)) {
                        char operation = 0;
                        if (i == line)
                            operation = '*';
                        printLine(i - 1, operation, true);
                    }
                }
                // Print the line after
                if (endLine < buf_len(currentBuffer->lines))
                    printLine(endLine, 0, true);
            }
            
            currentBuffer->currentLine = line;
        } break;
        case 'd': // Define bookmark - currently O(n) // TODO: Switch to using hashmap for storing bookmarks
        {
            pString name;
            name.start = current;
            current = skipWord(current, buf_end(input), true, false);
            name.end = current;
            int name_length = name.end - name.start;
            
            current = skipWhitespace(current, buf_end(input));
            restLength = buf_len(input) - (current - input);

            if (name_length == 0) {
                // TODO: Prompt for name
                printf("Unimplemented prompting for name. Please add a name after the line range.");
                break;
            }

            printf("Set bookmark '%.*s' to %d:%d\n", name_length, name.start, line_range.start, line_range.end);

            bool updated = add_bookmark(currentBuffer, name, line_range);
            //map_put(bookmarks, (void *) name, (void *) line_range);
        } break;
        case 'w': // TODO: For Testing
        {
            pString name;
            name.start = current;
            current = skipWord(current, buf_end(input), true, false);
            name.end = current;
            int name_length = name.end - name.start;

            current = skipWhitespace(current, buf_end(input));
            restLength = buf_len(input) - (current - input);

            Bookmark *result_bookmark;
            bool found = get_bookmark(currentBuffer, name, &result_bookmark);

            if (found) {
                printf("%d:%d\n", result_bookmark->range.start, result_bookmark->range.end);
            }
        } break;
        case 'g':
        {
            // Show list of bookmarks

            for (int i = 0; i < buf_len(currentBuffer->bookmarks); i++) {
                printf("%4d: %.*s %d:%d", i, (int) buf_len(currentBuffer->bookmarks[i].name), currentBuffer->bookmarks[i].name, currentBuffer->bookmarks[i].range.start, currentBuffer->bookmarks[i].range.end);
                printf("\n");
            }
        } break;
        case 's':
        {
            pString filename;
            filename.start = current;
            current = skipWord(current, buf_end(input), true, true);
            filename.end = current;
            int filename_length = filename.end - filename.start;

            current = skipWhitespace(current, buf_end(input)); // TODO
            restLength = buf_len(input) - (current - input);

            if (!currentBuffer->openedFilename && buf_len(currentBuffer->openedFilename) <= 0 && filename_length == 0) {
                printPrompt("Enter a filename: ");
                char *filename = NULL;
                int length = parsing_getLine_dynamic(&filename, true);
                while (length <= 0) {
                    printPrompt("Enter a filename: ");
                    length = parsing_getLine_dynamic(&filename, true);
                }
                
                // Add null character, buffer_saveFile requires it.
                buf_push(filename, '\0');
                
                printf("Saving '%s'\n", filename);
                buffer_saveFile(currentBuffer, filename);
            } else if (filename_length > 0) { // TODO: Should be checked first - default
                // Put filename into a buffer with \0 at end
                char *filename_buf = NULL;
                char *c = filename.start;
                buf_add(filename_buf, (long unsigned int) (filename.end - filename.start + 1));

                for (int i = 0; i < filename.end - filename.start + 1; i++) {
                    if (i == filename.end - filename.start) {
                        filename_buf[i] = '\0';
                    } else {
                        filename_buf[i] = filename.start[i];
                    }
                }

                printf("Saving '%s'\n", filename_buf);
                buffer_saveFile(currentBuffer, filename_buf);
            } else {
                printf("Saving '%s'", currentBuffer->openedFilename);
                buffer_saveFile(currentBuffer, currentBuffer->openedFilename);
            }
        } break;
        case '#':
        {
            printFileInfo();
        } break;
        case 'c':
        {
            currentBuffer->currentLine = buf_len(currentBuffer->lines);
            editorState_insertAfter(line_range);
        } break;
        case 'a':
        {
            editorState_insertAfter(line_range);
        } break;
        case 'i':
        {
            editorState_insertBefore(line_range);
        } break;
        case 'A':
        {
            editorState_appendTo(line_range);
        } break;
        case 'I':
        {
            editorState_prependTo(line_range);
        } break;
        case 'r':
        {
            editorState_replaceLine(line_range);
        } break;
        case 'R':
        {
            editorState_replaceString(line_range, rest, restLength);
        } break;
        case 'x':
        {
            editorState_deleteLine(line_range);
        } break;
        case 'm':
        {
            editorState_moveUp(line_range);
        } break;
        case 'M':
        {
            editorState_moveDown(line_range);
        } break;
        case 'p':
        {
            int line = line_range.start;
            
            if (!(line == 0 && buf_len(currentBuffer->lines) == 0)) {
                if (line == 0) line = currentBuffer->currentLine;
                line = checkLineNumber(line);
            }
            
            if (line < 0 || line > buf_len(currentBuffer->lines))
                printText(0);
            else printText(line - 1);
        } break;
        case 'P':
        {
            int line = line_range.start;
            
            if (!(line == 0 && buf_len(currentBuffer->lines) == 0)) {
                if (line == 0) line = currentBuffer->currentLine;
                line = checkLineNumber(line);
            }
            
            /*current = skipWhitespace(current, buf_end(input));
            if (*current == ':') { // NOTE: Small hack for supporting ranges using ':'
                current++;
                current = skipWhitespace(current, buf_end(input));
            }*/
            
            // If there was only one line number given
            if (line_range_length == 0) {
                if (line - 2 >= 0)
                    printLine(line - 2, 0, true);
                printLine(line - 1, 0, true);
                if (line < buf_len(currentBuffer->lines))
                    printLine(line, 0, true);
            } else {
                int endLine = line_range.end;
                //int endLine = (int) parseLineNumber(currentBuffer, current, buf_end(input));
                //current = skipWhitespace(current, buf_end(input));
                
                if (endLine == 0 && buf_len(currentBuffer->lines) != 0) {
                    if (endLine == 0) line = currentBuffer->currentLine;
                }
                
                // Make sure going from low to high (forwards)
                if (endLine < line) {
                    int tmp = endLine;
                    endLine = line;
                    line = tmp;
                }
                
                // Print the line before
                if (line - 2 >= 0)
                    printLine(line - 2, 0, true);
                
                // Print the range of lines
                for (int i = line; i <= endLine; i++) {
                    if (i - 1 >= 0 && i - 1 < buf_len(currentBuffer->lines))
                        printLine(i - 1, 0, true);
                }
                
                // Print the line after
                if (endLine < buf_len(currentBuffer->lines))
                    printLine(endLine, 0, true);
            }
        } break;
        case 'f':
        {
            editorState_findStringInFile(rest, restLength);
        } break;
        case 'F':
        {
            editorState_findStringInLine(rest, restLength);
        } break;
        case 'b':
        {
            char *restOrig = rest;
            rest = skipWhitespace(rest, buf_end(input));
            restLength = restLength - (rest - restOrig);

            // If p/n given without space between
            if (command.end - command.start == 2) {
                switch (command.start[1]) {
                    case 'n':
                    {
                        int current = currentBuffer - buffers;
                        int next = current + 1;
                        if (next >= buf_len(buffers))
                            next = 0;
                        
                        currentBuffer = &(buffers[next]);
                        
                        buf_free(input);
                        return KEEP;
                    } break;
                    case 'p':
                    {
                        int current = currentBuffer - buffers;
                        int previous = current - 1;
                        if (previous < 0)
                            previous = buf_len(buffers) - 1;
                        
                        currentBuffer = &(buffers[previous]);
                        
                        buf_free(input);
                        return KEEP;
                    } break;
                }
            } else if (restLength > 0 && line_range.start == 0) { // If space between p and b *and* a range wasn't given
                switch (rest[0]) {
                    case 'n':
                    {
                        int current = currentBuffer - buffers;
                        int next = current + 1;
                        if (next >= buf_len(buffers))
                            next = 0;
                        
                        currentBuffer = &(buffers[next]);
                        
                        buf_free(input);
                        return KEEP;
                    } break;
                    case 'p':
                    {
                        int current = currentBuffer - buffers;
                        int previous = current - 1;
                        if (previous < 0)
                            previous = buf_len(buffers) - 1;
                        
                        currentBuffer = &(buffers[previous]);
                        
                        buf_free(input);
                        return KEEP;
                    } break;
                }
            }
            
            // If a integer was given with the command
            if (line_range.start != 0) {
                char *end;
                /*char *end;
                int index = (int) strtol(rest, &end, 10);*/
                int index = line_range.start;
                
                char lineInput[MAXLENGTH / 4];
                int length;
                while (index < 0 || index > buf_len(buffers) - 1 || length == -1) {
                    printError("That buffer doesn't exist.");
                    printPrompt("Enter a buffer number: ");
                    length = parsing_getLine(lineInput, MAXLENGTH / 4, true);
                    index = (int) strtol(lineInput, &end, 10);
                }
                
                currentBuffer = &(buffers[index]);
                
                buf_free(input);
                return KEEP;
            }
            
            // Otherwise, list out the buffers
            for (int i = 0; i < buf_len(buffers); i++) {
                if (buf_len(buffers[i].openedFilename) <= 0) {
                    // We can assume this is the current buffer because you can't switch or close a buffer with unsaved changes
                    if (currentBuffer == &(buffers[i]))
                        printf("*%3d: new file", i);
                    else printf("%4d: new file", i);
                } else {
                    if (currentBuffer == &(buffers[i])) {
                        printf("*%3d: %.*s", i, (int) buf_len(buffers[i].openedFilename), buffers[i].openedFilename);
                    } else {
                        printf("%4d: %.*s", i, (int) buf_len(buffers[i].openedFilename), buffers[i].openedFilename);
                    }
                }
                if (buffers[i].modified)
                    printf("*");
                printf("\n");
            }
        } break;
        case 'l': // TODO: Reload file, prompt if unsaved changes?
        {
            
        } break;
        case 'o':
        {
            editorState_openAnotherFile(rest, restLength);
        } break;
        case 'n':
        {
            editorState_openNewFile(rest, restLength);
        } break;
        case 'e':
        {
            buf_free(input);
            return EXIT;
        } break;
        case 'E':
        {
            buf_free(input);
            return FORCE_EXIT;
        } break;
        case 'q':
        {
            buf_free(input);
            return QUIT;
        } break;
        case 'Q':
        {
            buf_free(input);
            return FORCE_QUIT;
        } break;
        // Hacked in - change filetype to FT_C because of how poor my filetype extension matching is
        case 't':
        {
            currentBuffer->fileType = FT_C;
            recreateOutline();
        } break;
        case 'T':
        {
            printf("FileType: %d\n", currentBuffer->fileType);
        } break;
        default:
        {
            printError("Unknown command");
        } break;
    }
    
    buf_free(input);
    return KEEP;
}

internal void editorState_openAnotherFile(char *rest, int restLength) {
    char str[MAXLENGTH / 4];
    int strLength = 0;
    
    // If a string was already given with the command
    if (restLength - 1 > 0) {
        if (rest[restLength - 1] == '\n')
            --restLength;
        
        // Copy into str
        strLength = restLength;
        strncpy(str, rest, strLength);
        
        if (str[strLength - 1] != '\0')
            str[strLength] = '\0';
        // Make sure string ends with zero termination
        assert(str[strLength] == '\0');
        
        {
            Buffer buffer;
            buffer_initEmptyBuffer(&buffer);
            buf_push(buffers, buffer);
            currentBuffer = buf_end(buffers) - 1;
        }
        
        if (!buffer_openFile(currentBuffer, str)) {
            printf("File doesn't exist... Creating it.\n\n");
            currentBuffer->modified = true;
            editorState_editor();
        } else {
            printFileInfo();
        }
        
        return;
    }
    
    printPrompt("Enter file to open: ");
    strLength = parsing_getLine(str, MAXLENGTH / 4, false);
    while (strLength == -1) {
        printPrompt("Enter file to open: ");
        strLength = parsing_getLine(str, MAXLENGTH / 4, true);
    }
    
    // Make sure string ends with zero termination
    assert(str[strLength - 1] == '\0');
    
    {
        Buffer buffer;
        buffer_initEmptyBuffer(&buffer);
        buf_push(buffers, buffer);
        currentBuffer = buf_end(buffers) - 1;
    }
    
    if (!buffer_openFile(currentBuffer, str)) {
        printf("File doesn't exist... Creating it.\n\n");
        currentBuffer->modified = true;
        editorState_editor();
    } else printFileInfo();
}

internal void editorState_openNewFile(char *rest, int restLength) {
    {
        Buffer buffer;
        buffer_initEmptyBuffer(&buffer);
        buffer.modified = true;
        buf_push(buffers, buffer);
        currentBuffer = buf_end(buffers) - 1;
    }
    
    if (restLength - 1 > 0) {
        // Trim whitespace from beginning of rest
        while (*rest == ' ' || *rest == '\t') {
            ++rest;
            --restLength;
        }
        
        // Copy into openedFilename
        for (int i = 0; i < restLength; i++) {
            buf_push(currentBuffer->openedFilename, rest[i]);
        }
        
        //buf_push(currentBuffer->openedFilename, '\0');
    }
    
    printf("Opening a new file.\n");
#ifdef _WIN32
    printf("Press Ctrl-Z on new line to denote End Of Input\n\n");
#else
    printf("Press Ctrl-D on new line to denote End Of Input\n\n");
#endif
    
    assert(buf_len(currentBuffer->lines) == 0);
    
    editorState_editor();
}

internal int getLineNumber() {
    char *end;
    char *lineInput = NULL;
    int length = -1;
    int line = -1;
    
    printPrompt("Enter a line number: ");
    length = parsing_getLine_dynamic(&lineInput, true);
    line = (int) strtol(lineInput, &end, 10);
    
    while (line <= 0 || line > buf_len(currentBuffer->lines) || length == -1) {
        if (lineInput != NULL) {
            buf_free(lineInput);
            lineInput = NULL;
        }
        printError("That line number exceeds the bounds of the file.\n");
        printPrompt("Enter a line number: ");
        length = parsing_getLine_dynamic(&lineInput, true);
        line = (int) strtol(lineInput, &end, 10);
    }
    
    buf_free(lineInput);
    
    return line;
}

internal int checkLineNumber(int original_line) {
    char *end;
    char *lineInput = NULL;
    int length = 0;
    int line = original_line;
    
    if ((currentBuffer->lines == NULL || buf_len(currentBuffer->lines) == 0) && (line == 0 || line == 1)) return 0;
    
    while (line <= 0 || line > buf_len(currentBuffer->lines) || length == -1) {
        if (lineInput != NULL) {
            buf_free(lineInput);
            lineInput = NULL;
        }
        printError("That line number exceeds the bounds of the file.\n");
        printPrompt("Enter a line number: ");
        length = parsing_getLine_dynamic(&lineInput, true);
        line = (int) strtol(lineInput, &end, 10);
        
        if ((currentBuffer->lines == NULL || buf_len(currentBuffer->lines) == 0) && (line == 0 || line == 1)) return 0;
    }
    
    buf_free(lineInput);
    
    return line;
}

internal Line *multiLineEditor(int previousLine, Line *insertLines, bool *canceled, OperationKind kind) {
    char *chars = NULL;
    if (previousLine - 1 > 0) { // AutoIndentation
        int whitespaceCount = 0;
        char *start = currentBuffer->lines[previousLine - 1].chars;
        while (*start == '\t') {
            ++whitespaceCount;
            ++start;
            if (start - chars > buf_len(chars)) break;
        }
        
        while (start - chars <= buf_len(chars)) {
            if (*start == '{') {
                ++whitespaceCount;
            } else if (*start == '}')
                --whitespaceCount;
            ++start;
        }
        
        chars = NULL;
        
        // If C, C++, or C_HEADER files, autoindent
        if (currentBuffer->fileType == FT_C || currentBuffer->fileType == FT_CPP || currentBuffer->fileType == FT_C_HEADER || currentBuffer->fileType == FT_UNKNOWN) {
            for (int i = 0; i < whitespaceCount; i++) {
                buf_push(chars, '\t');
            }
        }
    }
    
    char operation = ' ';
    switch (kind) {
        case 0:
        operation = ' '; break;
        case InsertAfter:
        operation = 'a'; break;
        case InsertBefore:
        operation = 'i'; break;
        default:
        operation = ' '; break;
    }
    
    int currentLine = previousLine + 1;
    printLineNumber("%c%4d ", operation, currentLine);
    
    bool inputCanceled = false;
    while ((chars = getInput(&inputCanceled, chars, NULL)) != NULL && buf_len(chars) != 0) {
        buf_push(insertLines, ((Line) { chars }));
        ++currentLine;
        
        // Count number of tabs at start of line for autoindentation of next line
        // TODO: Make work with spaces.
        int whitespaceCount = 0;
        char *start = chars;
        while (*start == '\t') {
            ++whitespaceCount;
            ++start;
            if (start - chars > buf_len(chars)) break;
        }
        
        int scopeOpeningCount = 0;
        while (start - chars <= buf_len(chars)) {
            if (*start == '{') {
                ++scopeOpeningCount;
            } else if (*start == '}')
                --scopeOpeningCount;
            ++start;
        }
        if (scopeOpeningCount < 0) scopeOpeningCount = 0;
        whitespaceCount += scopeOpeningCount;
        
        chars = NULL;
        
        // If C, C++, or C_HEADER files, autoindent
        if (currentBuffer->fileType == FT_C || currentBuffer->fileType == FT_CPP || currentBuffer->fileType == FT_C_HEADER || currentBuffer->fileType == FT_UNKNOWN) {
            for (int i = 0; i < whitespaceCount; i++) {
                buf_push(chars, '\t');
            }
        }
        
        printLineNumber("%c%4d ", operation, currentLine);
    }
    
    if (inputCanceled) {
        buf_free(chars);
        // Cancel the operation by returning
        (*canceled) = true;
        return NULL;
    }
    
    return insertLines;
}

internal void editorState_printHelpScreen() {
    if (buf_len(currentBuffer->openedFilename) > 0)
        printf("'%s' is currently open.\n\n", currentBuffer->openedFilename);
    else printf("An unnamed file is currently open.\n\n");
    
#ifdef _WIN32
    printf("Use Ctrl-Z+Enter to denote end of input\n");
#else
    printf("Use Ctrl-D to denote end of input\n");
#endif
    printf("Use Ctrl-X+Enter to cancel the current command/operation\n");
    printf("\n");
    printf(" * Ctrl-L or 'clear' - Clear the screen\n");
    /* Edit - rewrite a specific line, group of lines, group of characters in a line (given column numbers), and word/group of words */
    //printf(" * 'e' - Edit\n");
    printf(" * '#' - Gives back information on the file, including number of lines, filename, number of characters, filetype, etc.\n");
    printf(" * 'j (line#)' - Set's current line to line number (no output). Use 'j$' to set last line as current line.\n");
    printf(" * 'a (line#)' - Insert after the line number\n");
    printf(" * 'i (line#)' - Insert before the line number\n");
    printf(" * 'A (line#)' - Appends to a line\n");
    printf(" * 'I (line#)' - Prepends to a line\n");
    printf(" * 'r (line#)' - Replace a line with a new line\n");
    printf(" * 'R (line#) (string)' - Replace the first occurance of the string in the line\n");
    printf(" * 'x (line#)' - Deletes a line\n");
    printf(" * 'm (line#)' - Move the line up by one\n");
    printf(" * 'M (line#)' - Move the line down by one\n");
    printf(" * 'f (string)' - Finds the first occurance of the string in the file and prints the line it's on out\n");
    printf(" * 'F (line#) (string)' - Find the first occurance of the string in the line and print the line out showing you where the occurance is\n");
    printf(" * 'u' - Undo the last operation, cannot undo an undo, cannot undo past 1 operation - UNIMPLEMENTED\n"); // TODO
    printf(" * 'c' - Continue from last line; Append to end of file\n");
    printf(" * 'p (line#:start)' - Preview whole file (optionally starting at given line)\n");
    printf(" * 'P (line#:start):(line#:end)' - Preview a line or set of lines, including the line before and after\n");
    printf(" * 'b' - List all currently open buffers\n");
    printf(" * 'b (buffer#)' - Switch current buffer to buffer #\n");
    printf(" * 'bn' - Switch current buffer to next buffer. Will wrap around when hits end.\n");
    printf(" * 'bp' - Switch current buffer to previous buffer. Will wrap around when hits beginning.\n");
    printf(" * 'd(line#:start):(line#:end) (string)' - Create bookmark with line range start:end and name string\n");
    printf(" * 'w (string)' - Print out line range of bookmark with name string\n");
    printf(" * 'g' - List out all bookmarks\n");
    printf(" * 'o' - Open file in new buffer\n");
    printf(" * 'n' - Create new file in new buffer\n");
    printf(" * 's' - Save current buffer\n");
    //printf(" * 'S' - Save all buffers\n"); // TODO
    printf(" * 'e / E' - Exit current buffer / Exit current buffer (without save)\n");
    printf(" * 'q / Q' - Quit, closing all buffers / Quit, closing all buffers (without save)\n");
    printf("\nAny command that accepts a line number or line range - denoted by '(line#:start):(line#:end)' - can also accept a bookmark. Bookmarks are prefixed with '#'. Example: 'P #test'.\n");
}

// Editor - will allow user to type in anything, showing line number at start of new lines. To exit the editor, press Ctrl-D on Linux or Ctrl-Z+Enter on Windows. As each new line is entered, the characters will be added to a char pointer streatchy buffer (dynamic array). Then, this line will be added to the streatchy buffer of lines (called 'lines').
void editorState_editor(void) {
    int c;
    int line = 1;
    
    // If continuing a previously typed-in file,
    // start on last line and overwrite the EOF character
    //if (buf_len(currentBuffer->lines) > 0) {
    //line = buf_len(currentBuffer->lines) + 1;
    //}
    
    bool canceled = false;
    currentBuffer->lines = multiLineEditor(0, NULL, &canceled, 0);
    if (canceled) {
        // TODO: close the buffer?
    }
    printf("\n");
    
    // Set cursor to end of file
    currentBuffer->currentLine = buf_len(currentBuffer->lines);
}

// Insert lines after a specific line. Denote end of input by typing Ctrl-D (or Ctrl-Z+Enter on Windows) on new line.
internal void editorState_insertAfter(lineRange line_range) {
    int line = line_range.end;
    
    if (line == 0) line = currentBuffer->currentLine;
    else line = checkLineNumber(line);
    
    char c;
    if (line - 1 >= 0 && line - 1 < buf_len(currentBuffer->lines))
        printLine(line - 1, 0, true);
    int currentLine = line + 1;
    
    bool canceled = false;
    Line *insertLines = multiLineEditor(currentLine - 1, NULL, &canceled, InsertAfter);
    if (canceled) {
        for (int i = 0; i < buf_len(insertLines); i++) {
            buf_free(insertLines[i].chars);
        }
        buf_free(insertLines);
        return;
    }
    
    printf("\n");
    
    int firstMovedLine = buffer_insertAfterLine(currentBuffer, line, insertLines);
    
    // Free the old lines stretchy buffer
    buf_free(insertLines);
    
    // Show the line that was moved due to inserting before it (and after the line before it)
    if (firstMovedLine <= buf_len(currentBuffer->lines))
        printLine(firstMovedLine - 1, 'v', true);
    
    recreateOutline();
}

internal void editorState_insertBefore(lineRange line_range) {
    int line = line_range.start;
    
    if (line == 0) line = currentBuffer->currentLine;
    else line = checkLineNumber(line);
    
    if (line == 0) { // Current Line is 0
        line = 1;
    }
    
    /*if (line == buf_len(currentBuffer->lines) + 1) {
    currentBuffer->currentLine = buf_len(currentBuffer->lines);
    editorState_insertAfter(rest);
    return;
    }*/
    
    char c;
    if (line - 2 >= 0 && line - 1 < buf_len(currentBuffer->lines))
        printLine(line - 2, 0, true);
    int currentLine = line;
    
    bool canceled = false;
    Line *insertLines = multiLineEditor(currentLine - 1, NULL, &canceled, InsertBefore);
    if (canceled) {
        for (int i = 0; i < buf_len(insertLines); i++) {
            buf_free(insertLines[i].chars);
        }
        buf_free(insertLines);
        return;
    }
    
    printf("\n");
    
    int firstMovedLine = buffer_insertBeforeLine(currentBuffer, line, insertLines);
    
    // Free the old lines stretchy buffer
    buf_free(insertLines);
    
    // Show the line that was moved due to the insertion before it
    if (firstMovedLine <= buf_len(currentBuffer->lines))
        printLine(firstMovedLine - 1, 'v', true);
    
    recreateOutline();
}

internal void editorState_appendTo(lineRange line_range) {
    int line = line_range.start;
    
    if (line == 0) line = currentBuffer->currentLine;
    else line = checkLineNumber(line);
    
    char c;
    char *chars = NULL;
    if (line - 2 >= 0 && line - 2 < buf_len(currentBuffer->lines))
        printLine(line - 2, 0, true);
    
    printLine(line - 1, 'A', false);
    bool canceled = false;
    chars = getInput(&canceled, chars, NULL);
    if (canceled) {
        // Delete the chars buffer
        buf_free(chars);
        // Cancel the operation by returning
        return;
    }
    
    buffer_appendToLine(currentBuffer, line, chars);
    buf_free(chars);
    
    recreateOutline();
}

internal void editorState_prependTo(lineRange line_range) {
    int line = line_range.start;
    
    if (line == 0) line = currentBuffer->currentLine;
    else line = checkLineNumber(line);
    
    char c;
    if (line - 2 >= 0 && line - 2 < buf_len(currentBuffer->lines))
        printLine(line - 2, 0, true);
    
    char *chars = NULL; // The new char stretchy buffer
    
    printLine(line - 1, 'I', true);
    printPrompt("%4s ^- ", "");
    bool canceled = false;
    chars = getInput(&canceled, chars, NULL);
    if (canceled) {
        // Delete the chars buffer
        buf_free(chars);
        // Cancel the operation by returning
        return;
    }
    // Get rid of new line
    if (chars[buf_len(chars) - 1] == '\n') {
        buf_pop(chars);
    }
    
    buffer_prependToLine(currentBuffer, line, chars);
    
    recreateOutline();
}

internal void editorState_replaceLine(lineRange line_range) {
    int line = line_range.start;
    
    if (line == 0) line = currentBuffer->currentLine;
    else line = checkLineNumber(line);
    
    char c;
    if (line - 2 >= 0 && line - 2 < buf_len(currentBuffer->lines))
        printLine(line - 2, 0, true);
    
    char *chars = NULL; // The new char stretchy buffer
    
    printLine(line - 1, 'r', true);
    printf("%5s ", "");
    bool canceled = false;
    chars = getInput(&canceled, chars, NULL);
    if (canceled) {
        // Delete the chars buffer
        buf_free(chars);
        // Cancel the operation by returning
        return;
    }
    
    buffer_replaceLine(currentBuffer, line, chars);
    
    recreateOutline();
}

internal void editorState_replaceString(lineRange line_range, char *rest, int restLength) {
    int line = line_range.start;
    char *end = rest;
    
    if (line == 0) {
        line = currentBuffer->currentLine;
    } else {
        line = checkLineNumber(line);
    }
    
    char str[MAXLENGTH / 4];
    int strLength = 0;
    int exists = false;
    
    // If a string was already given with the command
    if (rest + restLength - end - 1 > 0) {
        // Copy into str
        strLength = rest + restLength - end;
        strncpy(str, end, strLength);
        // Use it instead of asking for a string to replace
        exists = true;
    }
    
    if (!exists) {
        printPrompt("Enter the string to replace: ");
        strLength = parsing_getLine(str, MAXLENGTH / 4, false);
        while (strLength == -1) {
            printPrompt("Enter the string to replace: ");
            strLength = parsing_getLine(str, MAXLENGTH / 4, true);
        }
    }
    
    char c;
    
    int index = buffer_findStringInLine(currentBuffer, line, str, strLength - 1);
    
    if (index == -1) {
        printError("No occurance of '%.*s' found\n", strLength, str);
        return;
    }
    
    // Print the previous line to give context
    if (line - 2 >= 0 && line - 2 < buf_len(currentBuffer->lines))
        printLine(line - 2, 0, true);
    
    // Print the string where the replacement is occuring
    printLine(line - 1, 'R', true);
    
    // Create a string (to be printed) with an arrow pointing to the beginning and end of the first occurance of the string being replaced.
    int strPointToMatchLength;
    if (str[strLength - 1] == '\0' || str[strLength - 1] == '\n') {
        strPointToMatchLength = index + strLength - 1;
    } else {
        strPointToMatchLength = index + strLength;
    }
    char *strPointToMatch = alloca(sizeof(char) * (strPointToMatchLength));
    for (int i = 0; i < strPointToMatchLength; i++) {
        if (i == index || i == strPointToMatchLength - 1) {
            strPointToMatch[i] = '^';
            continue;
        }
        if (i > index && i < strPointToMatchLength - 1)
            strPointToMatch[i] = '-';
        else strPointToMatch[i] = ' ';
    }
    
    char *chars = NULL; // What the string will be replaced with
    
    printPrompt("%5s %.*s- ", "", strPointToMatchLength, strPointToMatch); // TODO
    bool canceled = false;
    chars = getInput(&canceled, chars, NULL);
    if (canceled) {
        // Delete the chars buffer
        buf_free(chars);
        // Cancel the operation by returning
        return;
    }
    // Get rid of new line
    if (chars[buf_len(chars) - 1] == '\n') {
        buf_pop(chars);
    }
    
    buffer_replaceInLine(currentBuffer, line, index, index + strLength - 2, chars);
    
    recreateOutline();
}

// Finds the first occrance of the string in the given line
// Displays the line with an arrow pointing to the occurance
// Will also show the line before it to give context and the column of the start of the occurance
internal void editorState_findStringInLine(char *rest, int restLength) {
    char *end;
    int line = (int) strtol(rest, &end, 10);
    
    if (line == 0) {
        line = currentBuffer->currentLine;
    } else {
        line = checkLineNumber(line);
        // Don't include the space in between the line number and the string to replace
        ++end;
    }
    
    
    char str[MAXLENGTH / 4];
    int strLength = 0;
    bool exists = false;
    
    // If a string was already given with the command
    if (rest + restLength - end - 1 > 0) {
        // Copy into str
        strLength = rest + restLength - end;
        strncpy(str, end, strLength);
        // Use it instead of asking for a string to replace
        exists = true;
    }
    
    if (!exists) {
        printPrompt("Enter the string to find: ");
        strLength = parsing_getLine(str, MAXLENGTH / 4, false);
        while (strLength == -1) {
            printPrompt("Enter the string to find: ");
            strLength = parsing_getLine(str, MAXLENGTH / 4, true);
        }
    }
    
    int index = buffer_findStringInLine(currentBuffer, line, str, strLength - 1);
    
    if (index == -1) {
        printError("No occurance of '%.*s' was found in line %d\n", strLength, str, line);
        return;
    }
    
    // Print the previous line to give context
    if (line - 1 >= 0 && line - 1 < buf_len(currentBuffer->lines))
        printLine(line - 2, 0, true);
    
    // Print the line
    printLine(line - 1, 0, true);
    
    // Create a string (to be printed) with an arrow pointing to the beginning and end of the first occurance of the string being matched.
    int strPointToMatchLength;
    if (str[strLength - 1] == '\0' || str[strLength - 1] == '\n') {
        strPointToMatchLength = index + strLength - 1;
    } else {
        strPointToMatchLength = index + strLength;
    }
    
    char *strPointToMatch = alloca(sizeof(char) * (strPointToMatchLength));
    for (int i = 0; i < strPointToMatchLength; i++) {
        if (i == index || i == strPointToMatchLength - 1) {
            strPointToMatch[i] = '^';
            continue;
        }
        if (i > index && i < strPointToMatchLength - 1)
            strPointToMatch[i] = '-';
        else strPointToMatch[i] = ' ';
    }
    
    // Print out the string that's points to the occurance
    printf("%5s %.*s- \n", "", strPointToMatchLength, strPointToMatch); // TODO: printInfo()
}

// Finds the first occurance of the string in the file
// Displays the line it is on with an arrow pointing to the occurance
// Will also show the line before it to give context
internal void editorState_findStringInFile(char *rest, int restLength) {
    char str[MAXLENGTH / 4];
    int strLength = 0;
    bool exists = false;
    
    // If a string was already given with the command
    if (restLength - 1 > 0) {
        // Copy into str
        strLength = restLength;
        strncpy(str, rest, strLength);
        exists = true;
    }
    
    if (!exists) {
        printPrompt("Enter the string to find: ");
        strLength = parsing_getLine(str, MAXLENGTH / 4, false);
        while (strLength == -1) {
            printPrompt("Enter the string to find: ");
            strLength = parsing_getLine(str, MAXLENGTH / 4, true);
        }
    }
    
    
    int colIndex = -1;
    int foundIndex = buffer_findStringInFile(currentBuffer, str, strLength - 1, &colIndex);
    
    // If no occurance found in file
    if (foundIndex == -1) {
        printError("No occurance of '%.*s' found\n", strLength, str);
        return;
    }
    
    // Print the previous line to give context
    if (foundIndex - 1 >= 0 && foundIndex - 1 < buf_len(currentBuffer->lines))
        printLine(foundIndex - 1, 0, true);
    
    // Print the string where the occurance was found
    printLine(foundIndex, 0, true);
    
    // Create a string (to be printed) with an arrow pointing to the beginning and end of the first occurance of the string being matched.
    int strPointToMatchLength;
    if (str[strLength - 1] == '\0' || str[strLength - 1] == '\n') {
        strPointToMatchLength = colIndex + strLength - 1;
    } else {
        strPointToMatchLength = colIndex + strLength;
    }
    char *strPointToMatch = alloca(sizeof(char) * (strPointToMatchLength));
    for (int i = 0; i < strPointToMatchLength; i++) {
        if (i == colIndex || i == strPointToMatchLength - 1) {
            strPointToMatch[i] = '^';
            continue;
        }
        if (i > colIndex && i < strPointToMatchLength - 1)
            strPointToMatch[i] = '-';
        else strPointToMatch[i] = ' ';
    }
    
    // Print out the string that's points to the occurance
    printf("%5s %.*s- \n", "", strPointToMatchLength, strPointToMatch); // TODO: printInfo()
}

internal void editorState_deleteLine(lineRange line_range) {
    int line = line_range.start;
    
    if (line == 0) line = currentBuffer->currentLine;
    else line = checkLineNumber(line);
    
    // Show the line before the line that's being deleted
    if (line - 1 > 0)
        printLine(line - 2, 0, true);
    
    if (line == buf_len(currentBuffer->lines)) { // TODO: This isn't working correctly
        printLine(line - 1, 'x', true);
        buffer_deleteLine(currentBuffer, buf_len(currentBuffer->lines));
        
        // Show the first line that was moved - the line # should be the same as the line that was deleted
        if (line <= buf_len(currentBuffer->lines))
            printLine(line - 1, '^', true);
        return;
    }
    
    printLine(line - 1, 'x', true);
    
    buffer_deleteLine(currentBuffer, line);
    
    // Show the first line that was moved - the line # should be the same as the line that was deleted
    if (line <= buf_len(currentBuffer->lines))
        printLine(line - 1, '^', true);
    
    recreateOutline();
}

internal void editorState_moveUp(lineRange line_range) {
    int line = line_range.start;
    
    if (line == 0) line = currentBuffer->currentLine;
    else line = checkLineNumber(line);
    
    // Show the line before the line being moved up to
    if (line - 2 > 0)
        printLine(line - 3, 0, true);
    
    buffer_moveLineUp(currentBuffer, line);
    
    // Print the new position of the line that was moved and the old line that was moved down
    printLine(line - 2, '^', true);
    printLine(line - 1, 'v', true);
    
    // Print the next line to give context
    printLine(line, 0, true);
    
    recreateOutline();
}

internal void editorState_moveDown(lineRange line_range) {
    int line = line_range.start;
    
    if (line == 0) line = currentBuffer->currentLine;
    else line = checkLineNumber(line);
    
    // Show the line before the line being moved down
    if (line - 1 > 0)
        printLine(line - 2, 0, true);
    
    buffer_moveLineDown(currentBuffer, line);
    
    // Print the new position of the line that was moved and the old line that was moved up
    printLine(line - 1, '^', true);
    printLine(line, 'v', true);
    
    // Print the next line to give context
    printLine(line + 1, 0, true);
    
    recreateOutline();
}

/* Print the currently stored text with line numbers */
void printText(int startLine) {
    if (buf_len(currentBuffer->lines) <= 0) {
        printLineNumber("%5d ", 1);
        printf("\n");
        return;
    }
    
    int linesAtATime = 15; // TODO: Should have a setting for this (or based on terminal/console height)
    int offset = startLine;
    char c;
    
    for (int line = offset; line < linesAtATime + offset + 1 && line <= buf_len(currentBuffer->lines); line++) {
        if (line == buf_len(currentBuffer->lines)) {
            if (line + 1 == currentBuffer->currentLine)
                printLineNumber("%c%4d ", '*', line + 1);
            else printLineNumber("%5d ", line + 1);
            break;
        }
        printLine(line, 0, true);
    }
    offset = linesAtATime + offset + 1;
    if (offset >= buf_len(currentBuffer->lines)) {
        printf("\n");
        return;
    }
    printPrompt("\n<%d: %s|preview> ", currentBuffer - buffers, currentBuffer->openedFilename);
    
    bool forward = true;
    while ((c = getch()) != EOF && offset < buf_len(currentBuffer->lines))
    {
        if (c == '?') {
            // Print help info about preview command here
            printf("\nPreviewing '%s'\n", currentBuffer->openedFilename);
            printf(" * 'q' or Ctrl-X to stop previewing\n");
            printf(" * 'Q' to exit the whole program\n");
            printf(" * Enter/'n' to show the next lines\n");
            printf(" * 'p' to show previous lines");
            printf(" * 'j' start showing lines from given line number (TODO)"); // TODO
            printf(" * 'f' find first occurance of string in file and jump there (TODO)"); // TODO
            
            printPrompt("\n<%d: %s|preview> ", currentBuffer - buffers, currentBuffer->openedFilename);
            continue;
        } else if (c == 'p') {
            if (forward) {
                printf("\r");
                for (int i = 0; i < 45; i++) { // TODO: Hacky
                    printf(" ");
                }
                printf("\r");
                printf("--^-^-^-^-^--\n\n");
            }
            offset = offset - (linesAtATime * 2) - 2;
            if (offset < 0) offset = 0;
            forward = false;
        } else if (c == 'q' || c == 24) { // 26 is Ctrl-X, aka CANCEL
            break;
        } else if (c == 'Q') {
            exit(0);
        } else {
            if (!forward) {
                printf("\r");
                for (int i = 0; i < 45; i++) { // TODO: Hacky
                    printf(" ");
                }
                printf("\r");
                printf("--v-v-v-v-v--\n\n");
            }
            forward = true;
        }
        
        printf("\r");
        for (int i = 0; i < 45; i++) { // TODO: Hacky
            printf(" ");
        }
        printf("\r");
        for (int line = offset; line < linesAtATime + offset + 1 && line <= buf_len(currentBuffer->lines); line++) {
            if (line == buf_len(currentBuffer->lines)) {
                if (line + 1 == currentBuffer->currentLine)
                    printLineNumber("%c%4d ", '*', line + 1);
                else printLineNumber("%5d ", line + 1);
                break;
            }
            printLine(line, 0, true);
        }
        
        offset = offset + linesAtATime + 1;
        if (offset >= buf_len(currentBuffer->lines)) {
            break;
        }
        printPrompt("\n<%d: %s|preview> ", currentBuffer - buffers, currentBuffer->openedFilename);
    }
    
    printf("\n");
}

/*
Prints one line of text given the line number. Note that the line numbers start at 0 (although they are displayed to the user starting at 1).
Pass false into printNewLine so the new line at the end is not printed
*/
void printLine(int line, char operation, int printNewLine) {
    if (line == -1) line = currentBuffer->currentLine;
    // If no lines in buffer and line is 0, show one line.
    if (buf_len(currentBuffer->lines) <= 0 && line == 0) {
        if (operation != 0)
            printLineNumber("%c%4d ", operation, 1);
        else {
            printLineNumber("%5d ", 1);
        }
        printf("\n");
        return;
    }
    
    // If line is last line in file (one above the length of the lines)
    if (line == buf_len(currentBuffer->lines)) {
        if (operation != 0)
            printLineNumber("%c%4d ", operation, line + 1);
        else {
            if (line + 1 == currentBuffer->currentLine)
                printLineNumber("%c%4d ", '*', line + 1);
            else printLineNumber("%5d ", line + 1);
        }
        return;
    } else if (line > buf_len(currentBuffer->lines)) {
        // Error!
        return;
    }
    
    if (operation != 0)
        printLineNumber("%c%4d ", operation, line + 1);
    else {
        if (line + 1 == currentBuffer->currentLine)
            printLineNumber("%c%4d ", '*', line + 1);
        else printLineNumber("%5d ", line + 1);
    }
    
    int length = buf_len(currentBuffer->lines[line].chars);
    // It shouldn't print new line and end of line is a new line, subtract it off from the length
    if (!printNewLine && currentBuffer->lines[line].chars[length - 1] == '\n')
        --length;
    
    //printf("%.*s", length, currentBuffer->lines[line].chars);
    for (int i = 0; i < length; i++) {
        if (currentBuffer->lines[line].chars[i] == '\t')
            printf("    "); // 4 spaces // TODO: Add setting for this
        else if (currentBuffer->lines[line].chars[i] == INPUT_ESC)
            colors_printf(COLOR_RED, "$");
        else putchar(currentBuffer->lines[line].chars[i]);
    }
}

// TODO: number of chars, filetype, syntax highlighting enabled, outline enabled
void printFileInfo(void) {
    printf("File information for '%.*s'\n", (int) buf_len(currentBuffer->openedFilename), currentBuffer->openedFilename);
    printf("Filetype: %d\n", currentBuffer->fileType); // TODO: Print actual string of filetype
    
    int numOfLines = buf_len(currentBuffer->lines);
    if (numOfLines != 0) {
        // If last character of last line ends with a new line, add one to the number of lines
        Line lastLine = currentBuffer->lines[buf_len(currentBuffer->lines) - 1];
        char lastChar = lastLine.chars[buf_len(lastLine.chars) - 1];
        if (lastChar == '\n') {
            numOfLines++;
        }
    }
    printf("Number of Lines: %d\n", numOfLines);
    
    // If markdown file, print outline
    if (currentBuffer->fileType == FT_MARKDOWN) {
        if (buf_len(currentBuffer->outline.markdown_nodes) > 0)
            printf("Outline:\n");
    } else if (currentBuffer->fileType == FT_C) {
        if (buf_len(currentBuffer->outline.c_nodes) > 0)
            printf("Outline:\n");
    }
    showOutline();
}
