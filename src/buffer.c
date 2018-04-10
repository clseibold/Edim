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
    
    fclose(fp);
}

void buffer_insertAfterLine(Buffer *buffer, int line) { // TODO
    
}

void buffer_insertBeforeLine(Buffer *buffer, int line) { // TODO
    
}

void buffer_appendToLine(Buffer *buffer, int line, char *chars) {
    
}

void buffer_prependToLine(Buffer *buffer, int line, char *chars) {
    
}

void buffer_replaceLine(Buffer *buffer, int line, char *chars) {
    
}

void buffer_replaceInLine(Buffer *buffer, int line, int startIndex, int endIndex, char *chars) {
    
}

void buffer_moveLineUp(Buffer *buffer, int line) {
    
}

void buffer_moveLineDown(Buffer *buffer, int line) {
    
}

void buffer_deleteLine(Buffer *buffer, int line) {
    
}

// Returns index of first occurance of string
int buffer_findStringInLine(Buffer *buffer, int line, char *str, int strLength) {
    
}

// Returns the pointer to the line where the string was found. Also sets the index passed in to the index of the first occurance in that line.
Line *buffer_findStringInFile(Buffer *buffer, char *str, int strLength, int *index) {
    
}
