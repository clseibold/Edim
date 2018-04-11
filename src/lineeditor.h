#ifndef LINEEDITOR_H
#define LINEEDITOR_H

#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

#define true 1
#define false 0
#define forever for(;;)

#define internal static

typedef enum State {
    KEEP,
    MAIN_MENU,
    NEW_FILE,
    OPEN_FILE,
    QUIT
} State;

/* === editor.c === */
#define MAXLENGTH 200000 /* 2000000 was too big on Windows */

typedef enum EditorState {
    ED_KEEP,
    ED_OPEN,
    ED_NEW,
    ED_EDITOR,
    ED_MENU,
    ED_EXIT, // Exit to main menu
    ED_QUIT, // Quit program
} EditorState;

State editorState(EditorState state, char args[MAXLENGTH / 4], int argsLength);
EditorState editorState_menu(void);
EditorState editorState_editor(void);

void printText(int startLine);
void printLine(int line, char operation, int printNewLine);
void printFileInfo(void);

/* === parsing.c === */

int parsing_getLine(char *line, int max, int trimSpace);
int parsing_getLine_dynamic(char **chars, int trimSpace);
void createOutline(void);
void recreateOutline(void);
void showOutline(void);
void createMarkdownOutline(void);
void createCOutline(void);
void showMarkdownOutline(void);
void showCOutline(void);

/* == Streatchy Buffers (by Sean Barratt) === */

#define MAX(x, y) ((x) >= (y) ? (x) : (y))

void *xrealloc(void *prt, size_t num_bytes);
void *xmalloc(size_t num_bytes);
void fatal(const char *fmt, ...);

typedef struct BufHdr {
    size_t len;
    size_t cap;
    char buf[0]; // [0] new in c99
} BufHdr;

#define buf__hdr(b) ((BufHdr *) ((char *) (b)  - offsetof(BufHdr, buf)))
#define buf__fits(b, n) (buf_len(b) + (n) <= buf_cap(b))
#define buf__fit(b, n) (buf__fits((b), (n)) ? 0 : ((b) = buf__grow((b), buf_len(b) + (n), sizeof(*(b)))))

#define buf_len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf_push(b, x) (buf__fit((b), 1), (b)[buf__hdr(b)->len++] = (x))
#define buf_end(b) ((b) + buf_len(b))

#define buf_add(b, n) (buf__fit((b), n), buf__hdr(b)->len += n, &(b)[buf__hdr(b)->len - n]) // TODO: Not sure if I should be returning the address or not
#define buf_pop(b) (buf__hdr(b)->len--, &(b)[buf__hdr(b)->len + 1]) // TODO: Check that array exists and length doesn't go below 0
#define buf_pop_all(b) (buf__hdr(b)->len = 0)

#define buf_free(b) ((b) ? (free(buf__hdr(b)), (b) = NULL) : 0)

void *buf__grow(const void *buf, size_t new_len, size_t elem_size);

/* === buffer.c - Text Editing Data Structures === */

typedef enum FileType {
    FT_UNKNOWN, FT_TEXT, FT_MARKDOWN, FT_C, FT_CPP, FT_C_HEADER // TODO: Add Batch and Bash files
} FileType;

void getFileTypeExtension(FileType ft, char **ftExt);

typedef struct Line {
    char *chars;
} Line;

typedef enum OperationKind {
    Undo, InsertAfter, InsertBefore, AppendTo, PrependTo, ReplaceLine, ReplaceString, DeleteLine
} OperationKind;

typedef struct Operation {
    OperationKind kind;
    int *lines; // Line(s) that have been modified/added/deleted by the operation
    union {
        Line original; // The original line that was modified/deleted - TODO: Not very memory efficient
    };
} Operation;

// Levels:
//  0 for #
//  1 for ##
//  ...
typedef struct MarkdownOutlineNode {
    Line *line;
    int lineNum;
    int level;
} MarkdownOutlineNode;

typedef struct COutlineNode {
    Line *line;
    int lineNum;
} COutlineNode;

typedef struct Buffer {
    char *openedFilename; // char Streatchy buffer for the currently opened filename
    FileType fileType;
    Line *lines;
    Operation lastOperation;
    // Used by default when no line passed into a command.
    // Commands that modify the file will change the currentLine to the last line it modified. Some commands, like 'c', don't modify the file based on the current line, but will change the current line to what it's modifying ('c' will change the current line to the last line in the file and start inserting from there).
    int currentLine;
    int modified;
    union outline {
        MarkdownOutlineNode *markdown_nodes;
        COutlineNode *c_nodes;
    } outline;
} Buffer;

Buffer currentBuffer;

void buffer_initEmptyBuffer(Buffer *buffer);
int buffer_openFile(Buffer *buffer, char *filename);
void buffer_saveFile(Buffer *buffer, char *filename);
int buffer_insertAfterLine(Buffer *buffer, int line, Line *lines); // TODO
int buffer_insertBeforeLine(Buffer *buffer, int line, Line *lines); // TODO
void buffer_appendToLine(Buffer *buffer, int line, char *chars);
void buffer_prependToLine(Buffer *buffer, int line, char *chars);
void buffer_replaceLine(Buffer *buffer, int line, char *chars);
// TODO: This will basically just delete the lines and then insert before the line number
// void buffer_replaceLines(Buffer *buffer, int lineStart, int lineEnd, Line *lines);
void buffer_replaceInLine(Buffer *buffer, int line, int startIndex, int endIndex, char *chars);
// TODO: Add the number to move up by
void buffer_moveLineUp(Buffer *buffer, int line);
// TODO: Add the number to move up by
void buffer_moveLineDown(Buffer *buffer, int line);
void buffer_deleteLine(Buffer *buffer, int line);
// TODO
// void buffer_deleteLines(Buffer *buffer, int lineStart, int lineEnd);
int buffer_findStringInLine(Buffer *buffer, int line, char *str, int strLength);
Line *buffer_findStringInFile(Buffer *buffer, char *str, int strLength, int *index);

/* === Colors === */

#ifdef _WIN32
#include <windows.h>

#define FOREGROUND_YELLOW FOREGROUND_RED|FOREGROUND_GREEN
#define FOREGROUND_CYAN FOREGROUND_GREEN|FOREGROUND_BLUE
#define FOREGROUND_MAGENTA FOREGROUND_RED|FOREGROUND_BLUE
#define FOREGROUND_WHITE FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE

#define BACKGROUND_YELLOW BACKGROUND_RED|BACKGROUND_GREEN
#define BACKGROUND_CYAN BACKGROUND_GREEN|BACKGROUND_BLUE
#define BACKGROUND_MAGENTA BACKGROUND_RED|BACKGROUND_BLUE
#define BACKGROUND_WHITE BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE

#endif

#define COL_RED "\x1b[31m" // Error
#define COL_GREEN "\x1b[32m" // Prompt, Success?
#define COL_YELLOW "\x1b[33m" // Line Numbers
#define COL_BLUE "\x1b[34m"
#define COL_MAGENTA "\x1b[35m"
#define COL_CYAN "\x1b[36m" // Information
#define COL_RESET "\x1b[0m" // Input

typedef enum COLOR
{
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_YELLOW,
    COLOR_CYAN,
    COLOR_MAGENTA,
    COLOR_WHITE,
    COLOR_BLACK
} COLOR;

#ifdef _WIN32
HANDLE hConsole; // Used for coloring output on Windows
#endif

void setColor(COLOR foreground);
void resetColor(void);
void colors_printf(COLOR foreground, const char *fmt, ...);
void colors_puts(COLOR foreground, const char *fmt, ...);
void printError(const char *fmt, ...);
void printPrompt(const char *fmt, ...);
void printLineNumber(const char *fmt, ...);

#endif