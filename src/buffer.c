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
    
    (*buffer).openedFilename = NULL;
    (*buffer).fileType = FT_UNKNOWN;
    (*buffer).lines = NULL;
    (*buffer).lastOperation = emptyOperation;
    (*buffer).modified = false;
}

// filename should be zero-terminated
void buffer_openFile(Buffer *buffer, char *filename) {
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
    
    //printf("Opening file '%s'.\n", filename);
    for (int i = 0; i < strlen(filename) + 1; i++) {
        buf_push(buffer->openedFilename, filename[i]);
    }
    
    // Make sure the filename ends with '\0'
    assert(buffer->openedFilename[buf_len(buffer->openedFilename) - 1] == '\0');
    char *chars = NULL;
    int line = 1;
    char c;
    
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
    
    // Create the outline
    createOutline(); // TODO
    
    //printFileInfo();
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

void buffer_insertAfterLine(Buffer *buffer, int line) { // TODO
    buffer->modified = true;
}

void buffer_insertBeforeLine(Buffer *buffer, int line) { // TODO
    
    buffer->modified = true;
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
    // TODO: Switch to using memmove/memcpy
    for (int i = 0; i < buf_len(chars); i++) {
        buf_push(buffer->lines[lineToAppendTo - 1].chars, chars[i]);
    }
    
    buffer->modified = true;
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
    // TODO: Switch to using memmove/memcpy
    for (int i = 0; i < buf_len(oldBuffer); i++) {
        buf_push(buffer->lines[lineToPrependTo - 1].chars, oldBuffer[i]);
    }
    
    // Free the old buffer
    buf_free(oldBuffer);
    
    buffer->modified = true;
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
}

// Replace in the line from the startIndex to the endIndex (inclusive) with the provided char buffer
// TODO
void buffer_replaceInLine(Buffer *buffer, int line, int startIndex, int endIndex, char *chars) {
    int lineToReplaceIn = line;
    if (line == -1) {
        lineToReplaceIn = buffer->currentLine;
    }
    
    buffer->modified = true;
}

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
        // TODO: Switch to memmove (which allows overlapping)
        for (int i = lineToDelete - 1; i < buf_len(buffer->lines) - 1; i++) {
            buffer->lines[i] = buffer->lines[i + 1];
        }
        
        // Decrease the length of the buffer, keeping the char buffer of this last line because is was moved down one.
        buf_pop(buffer->lines);
    }
    
    buffer->modified = true;
}

// Returns index of first occurance of string
int buffer_findStringInLine(Buffer *buffer, int line, char *str, int strLength) {
    
}

// Returns the pointer to the line where the string was found. Also sets the index passed in to the index of the first occurance in that line.
Line *buffer_findStringInFile(Buffer *buffer, char *str, int strLength, int *index) {
    
}
