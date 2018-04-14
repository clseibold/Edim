#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <stdlib.h>

#include "lineeditor.h"

void buffer_initEmptyBuffer(Buffer *buffer) {
    Line emptyLine;
    emptyLine.chars = NULL;
    
    Operation emptyOperation;
    emptyOperation.kind = Undo;
    emptyOperation.lines = NULL;
    emptyOperation.original = emptyLine;
    
    buffer->openedFilename = NULL;
    buffer->fileType = FT_UNKNOWN;
    buffer->lines = NULL;
    buffer->lastOperation = emptyOperation;
    buffer->modified = false;
    buffer->outline.nodes = NULL;
}

// filename should be zero-terminated
// Returns 0 (false) if couldn't open file - however, the filetype is still set to the buffer.
int buffer_openFile(Buffer *buffer, char *filename) {
    FILE *fp;
    fp = fopen(filename, "r");
    
    // Find last occurance of '.' in filename
    int dotIndex;
    /*for (int i = strlen(filename) - 1; i >= 0; i++) {
        if (filename[i] == '.') {
            dotIndex = i;
            break;
        }
    }*/
    for (int i = 0; i < strlen(filename); i++) {
        if (filename[i] == '.') {
            dotIndex = i;
        }
    }
    
    // Determine filetype based on extension
    // TODO: Doesn't handle CPP files yet.
    FileType ft;
    switch (filename[dotIndex + 1]) {
        case 't':
        ft = FT_TEXT;
        break;
        case 'm':
        ft = FT_MARKDOWN;
        break;
        case 'c':
        ft = FT_C;
        break;
        case 'h':
        ft = FT_C_HEADER;
        break;
        default:
        ft = FT_UNKNOWN;
        break;
    }
    if (ft != FT_UNKNOWN) {
        int isType = true;
        char *ftExt;
        getFileTypeExtension(ft, &ftExt);
        for (int i = dotIndex + 1; i < strlen(filename); i++) {
            if (filename[i] != ftExt[i - dotIndex - 1]) {
                isType = false;
                break;
            }
        }
        
        if (isType)
            buffer->fileType = ft;
        else buffer->fileType = FT_UNKNOWN;
        
        free(ftExt);
    }
    
    buffer->openedFilename = NULL;
    for (int i = 0; i < strlen(filename) + 1; i++) {
        buf_push(buffer->openedFilename, filename[i]);
    }
    
    // Make sure the filename ends with '\0'
    assert(buffer->openedFilename[buf_len(buffer->openedFilename) - 1] == '\0');
    char *chars = NULL;
    int line = 1;
    char c;
    
    // If fp is NULL, file doesn't exist. Return false after having set the fileType.
    if (fp == NULL) {
        buffer->modified = true;
        fclose(fp);
        return false;
    }
    
    while ((c = fgetc(fp)) != EOF) {
        buf_push(chars, c);
        if (c == '\n') {
            buf_push(buffer->lines, ((Line) { chars }));
            ++line;
            chars = NULL; // Create new char Stretchy Buffer for next line.
        }
    }
    
    fclose(fp);
    
    // Set modified to false and current line to last line in file.
    buffer->modified = false;
    buffer->currentLine = buf_len(buffer->lines);
    
    printf("Filetype: %d\n", buffer->fileType);
    
    // Create the outline
    createOutline();
    
    return true;
}

void buffer_close(Buffer *buffer) {
    // Clear openedFilename and the file information
    buf_free(buffer->openedFilename);
    
    for (int i = 0; i < buf_len(buffer->lines); i++) {
        buf_free(buffer->lines[i].chars);
    }
    
    // Clear the outline
    switch (buffer->fileType) {
        case FT_MARKDOWN:
        {
            buf_free(buffer->outline.markdown_nodes);
        } break;
        case FT_C:
        {
            buf_free(buffer->outline.c_nodes);
        } break;;
    }
    
    // Free the buffer
    buf_free(buffer->lines);
}

// If openedFilename is not set in the buffer, then filename is used.
void buffer_saveFile(Buffer *buffer, char *filename) {
    FILE *fp;
    if (buffer->openedFilename && buf_len(buffer->openedFilename) > 0) {
        fp = fopen(buffer->openedFilename, "w");
    } else {
        fp = fopen(filename, "w");
        // Copy filename into openedFilename
        for (int i = 0; i < strlen(filename) + 1; i++) {
            buf_push(buffer->openedFilename, filename[i]);
        }
        
        // Find last occurance of '.' in filename
        int dotIndex;
        for (int i = 0; i < strlen(filename); i++) {
            if (filename[i] == '.') {
                dotIndex = i;
            }
        }
        
        // Determine filetype based on extension
        // TODO: Doesn't handle CPP files yet.
        FileType ft;
        switch (filename[dotIndex + 1])
        {
            case 't':
            ft = FT_TEXT;
            break;
            case 'm':
            ft = FT_MARKDOWN;
            break;
            case 'c':
            ft = FT_C;
            break;
            case 'h':
            ft = FT_C_HEADER;
            break;
            default:
            ft = FT_UNKNOWN;
            break;
        }
        if (ft != FT_UNKNOWN) {
            int isType = true;
            char *ftExt;
            getFileTypeExtension(ft, &ftExt);
            for (int i = dotIndex + 1; i < strlen(filename); i++) {
                if (filename[i] != ftExt[i - dotIndex - 1]) {
                    isType = false;
                    break;
                }
            }
            
            if (isType) {
                buffer->fileType = ft;
            } else {
                buffer->fileType = FT_UNKNOWN;
            }
            free(ftExt);
        }
    }
    
    // Write the characters out to the file
    for (int line = 0; line < buf_len(buffer->lines); line++) {
        for (int i = 0; i < buf_len(buffer->lines[line].chars); i++) {
            fprintf(fp, "%c", buffer->lines[line].chars[i]);
        }
    }
    
    buffer->modified = false;
    
    fclose(fp);
}

// The lines buffer isn't freed.
// Returns the line after the lines that were added.
int buffer_insertAfterLine(Buffer *buffer, int line, Line *lines) {
    int lineToInsertAfter = line;
    if (line == -1) {
        lineToInsertAfter = buffer->currentLine;
    }
    
    int linesAddedAmt = buf_len(lines);
    int amtToMove = buf_len(buffer->lines) - line;
    
    // Add space for the new lines into the buffer
    buf_add(buffer->lines, linesAddedAmt);
    
    // Move the lines after the line inserting after up by how many lines were inserted // TODO: Make this message clearer
    Line *moveSource = &(buffer->lines[lineToInsertAfter - 1 + 1]);
    Line *moveDestination = moveSource + linesAddedAmt;
    memmove(moveDestination, moveSource, amtToMove * sizeof(Line));
    
    // Copy the lines that were inserted to directly after the line inserting after
    Line *copySource = lines;
    Line *copyDestination = moveSource;
    size_t copyAmt = linesAddedAmt * sizeof(Line);
    memcpy(copyDestination, copySource, copyAmt);
    
    // Set cursor to the last line that was inserted
    buffer->modified = true;
    buffer->currentLine = lineToInsertAfter + linesAddedAmt;
    
    // Return the first line that was moved to make room for the inserted lines
    return buffer->currentLine + 1;
}

// The lines buffer isn't freed.
// Returns the line after the lines that were added.
int buffer_insertBeforeLine(Buffer *buffer, int line, Line *lines) {
    int lineToInsertBefore = line;
    if (line == -1) {
        lineToInsertBefore = buffer->currentLine;
    }
    
    int linesAddedAmt = buf_len(lines);
    int amtToMove = buf_len(buffer->lines) - (line - 1);
    
    // Add space for the new lines into the buffer
    buf_add(buffer->lines, linesAddedAmt);
    
    // Move the lines after the passed in line (and including it) up by how many lines are being inserted before
    Line *moveSource = &(buffer->lines[lineToInsertBefore - 1]);
    Line *moveDestination = moveSource + linesAddedAmt;
    memmove(moveDestination, moveSource, amtToMove * sizeof(Line));
    
    // Copy the lines that were inserted to the newly created space
    Line *copySource = lines;
    Line *copyDestination = moveSource;
    size_t copyAmt = linesAddedAmt * sizeof(Line);
    memcpy(copyDestination, copySource, copyAmt);
    
    // Set the current line to the line that the lines were inserted before
    buffer->modified = true;
    buffer->currentLine = lineToInsertBefore + linesAddedAmt;
    
    // Return the first line that was moved to make room for the inserted lines
    return buffer->currentLine;
}

// Pass in a char buffer to append to line
// Does not free the chars buffer
void buffer_appendToLine(Buffer *buffer, int line, char *chars) {
    int lineToAppendTo = line;
    if (line == -1) {
        lineToAppendTo = buffer->currentLine;
    }
    // Remove the new line character from the line
    buf_pop(buffer->lines[lineToAppendTo - 1].chars);
    
    // Copy from the passed-in chars buffer to the line's chars buffer
    size_t num = buf_len(chars);
    char *destination = buf_add(buffer->lines[lineToAppendTo - 1].chars, num);
    strncpy(destination, chars, num);
    
    buffer->modified = true;
    buffer->currentLine = lineToAppendTo;
}

// Pass in a char buffer that will be used in place of the new line. This buffer should not end in a new line.
// The old char buffer of the line will be freed.
void buffer_prependToLine(Buffer *buffer, int line, char *chars) {
    int lineToPrependTo = line;
    if (line == -1) {
        lineToPrependTo = buffer->currentLine;
    }
    
    // Store the old buffer
    char *oldBuffer = buffer->lines[lineToPrependTo - 1].chars;
    
    // Set the line to the new buffer passed in
    buffer->lines[lineToPrependTo - 1].chars = chars;
    
    // Push onto the buffer the chars of the old buffer
    size_t num = buf_len(oldBuffer);
    char *destination = buf_add(buffer->lines[lineToPrependTo - 1].chars, num);
    strncpy(destination, oldBuffer, num);
    
    // Free the old buffer
    buf_free(oldBuffer);
    
    buffer->modified = true;
    buffer->currentLine = lineToPrependTo;
}

// Pass in a char buffer that the line's char buffer will be replaced with. This buffer should likely end in a new line.
// The old char buffer of the line will be freed.
void buffer_replaceLine(Buffer *buffer, int line, char *chars) {
    int lineToReplace = line;
    if (line == -1) {
        lineToReplace = buffer->currentLine;
    }
    
    // Free the old buffer
    buf_free(buffer->lines[lineToReplace - 1].chars);
    
    // Set the line to the new buffer passed in
    buffer->lines[lineToReplace - 1].chars = chars;
    
    buffer->modified = true;
    buffer->currentLine = lineToReplace;
}

// Replace in the line from the startIndex to the endIndex (inclusive) with the provided char buffer
void buffer_replaceInLine(Buffer *buffer, int line, int startIndex, int endIndex, char *chars) {
    int lineToReplaceIn = line;
    if (line == -1) {
        lineToReplaceIn = buffer->currentLine;
    }
    
    int lengthOfStringToReplace = endIndex - startIndex;
    int addedAmt = buf_len(chars) - lengthOfStringToReplace - 1;
    int amtToMove = buf_len(buffer->lines[lineToReplaceIn - 1].chars) - endIndex;
    
    // If replacement string is bigger than string to replace, add characters to the buffer. Otherwise, if replacement string is smaller, pop off the correct amount
    if (addedAmt > 0)
        buf_add(buffer->lines[lineToReplaceIn - 1].chars, addedAmt);
    else if (addedAmt < 0) {
        for (int i = addedAmt; i < 0; i++) {
            buf_pop(buffer->lines[lineToReplaceIn - 1].chars);
        }
    }
    
    // Move over the characters due to the replacement string being bigger/smaller than the string being replaced
    char *moveSource = &(buffer->lines[lineToReplaceIn - 1].chars[endIndex + 1]);
    char *moveDestination = &(buffer->lines[lineToReplaceIn - 1].chars[endIndex + 1 + addedAmt]);
    size_t bytes = sizeof(char) * amtToMove;
    memmove(moveDestination, moveSource, bytes);
    
    // Copy over the replacement string
    char *source = chars;
    char *destination = &(buffer->lines[lineToReplaceIn - 1].chars[startIndex]);
    size_t copyAmt = buf_len(chars) * sizeof(char);
    memcpy(destination, source, copyAmt);
    
    buffer->modified = true;
    buffer->currentLine = lineToReplaceIn;
}

// TODO: Add the number to move up by
void buffer_moveLineUp(Buffer *buffer, int line) {
    int lineToMove = line;
    if (line == -1) {
        lineToMove = buffer->currentLine;
    }
    
    // Store the line where the given line is being moved up to
    Line tmp = buffer->lines[lineToMove - 2];
    
    // Set the new position of the line being moved
    buffer->lines[lineToMove - 2] = buffer->lines[lineToMove - 1];
    
    // Set the old position to the line stored in tmp (the line that's moved down)
    buffer->lines[lineToMove - 1] = tmp;
    
    // Set the currentLine to the new position of the line that was moved up
    buffer->modified = true;
    buffer->currentLine = lineToMove - 1;
}

// TODO: Add the number to move up by
void buffer_moveLineDown(Buffer *buffer, int line) {
    int lineToMove = line;
    if (line == -1) {
        lineToMove = buffer->currentLine;
    }
    
    // Store the line where the given line is being moved down to
    Line tmp = buffer->lines[lineToMove];
    
    // Set the new position of the line being moved
    buffer->lines[lineToMove] = buffer->lines[lineToMove - 1];
    
    // Set the old position to the line stored in tmp (the line that's moved up)
    buffer->lines[lineToMove - 1] = tmp;
    
    // Set the currentLine to the new position of the line that was moved down
    buffer->modified = true;
    buffer->currentLine = lineToMove + 1;
}

void buffer_deleteLine(Buffer *buffer, int line) {
    int lineToDelete = line;
    if (line == -1) {
        lineToDelete = buffer->currentLine;
    }
    
    if (lineToDelete == buf_len(buffer->lines)) {
        buf_free(buffer->lines[buf_len(buffer->lines) - 1].chars);
        buf_pop(buffer->lines);
    } else {
        // Delete the char buffer of the line that's being deleted
        buf_free(buffer->lines[lineToDelete - 1].chars);
        
        // Move all lines down one
        void *source = &(buffer->lines[lineToDelete - 1 + 1]);
        void *destination = &(buffer->lines[lineToDelete - 1]);
        size_t bytes = sizeof(Line) * (buf_len(buffer->lines) - lineToDelete);
        memmove(destination, source, bytes);
        
        // Decrease the length of the buffer, keeping the char buffer of this last line because is was moved down one.
        buf_pop(buffer->lines);
    }
    
    // Set the cursor the the line before the line that was deleted
    buffer->modified = true;
    if (lineToDelete - 1 > 0)
        buffer->currentLine = lineToDelete - 1;
    else if (lineToDelete > buf_len(buffer->lines))
        buffer->currentLine = buf_len(buffer->lines);
    else buffer->currentLine = lineToDelete;
}

// The string may be or not be a stretchy buffer. The length should be passed in.
// Returns index of first occurance of string, -1 for no occurance
int buffer_findStringInLine(Buffer *buffer, int line, char *str, int strLength) {
    int lineToSearch = line;
    if (line == -1) {
        lineToSearch = buffer->currentLine;
    }
    // Find the first occurance of the string in the current line
    int index = -1; // Column index
    int ii = 0;
    
    for (int i = 0; i < buf_len(buffer->lines[lineToSearch - 1].chars); i++) {
        if (buffer->lines[lineToSearch - 1].chars[i] == str[ii]) {
            if (ii == 0)
                index = i;
            ++ii;
        } else {
            ii = 0;
            index = -1;
        }
        
        // If string ends in a new line or 0 termination, subtract one from the string length so we don't match them
        if (str[strLength - 1] == '\0' || str[strLength - 1] == '\n') {
            if (ii == strLength - 1) break;
        } else {
            if (ii == strLength) break;
        }
    }
    
    buffer->currentLine = lineToSearch;
    
    // Return the index
    return index;
}

// The string must not be null terminated and can be or not be a stretchy buffer. The length should be passed in.
// Returns the index to the line where the string was found. Also sets the column index, that was passed in, to the index of the first occurance in that line (this index counts from 0).
int buffer_findStringInFile(Buffer *buffer, char *str, int strLength, int *colIndex) {
    // Go though all lines in buffer
    for (int line = 0; line < buf_len(buffer->lines); line++) {
        int index = buffer_findStringInLine(buffer, line + 1, str, strLength);
        
        if (index != -1) {
            (*colIndex) = index;
            buffer->currentLine = line + 1; // TODO
            return line;
        }
    }
    
    return -1;
}
