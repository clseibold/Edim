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

void buffer_openFile(Buffer *buffer, char *filename) {
    
}

// If openedFilename is not set in the buffer, then filename is used.
void buffer_saveFile(Buffer *buffer, char *filename) {
    
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
