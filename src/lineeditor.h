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
EditorState openFile(char *filename);
EditorState editorState_menu(void);
EditorState editorState_editor(void);
EditorState editorState_insertAfter(int line);
EditorState editorState_insertBefore(int line);
EditorState editorState_appendTo(int line);
EditorState editorState_prependTo(int line);
EditorState editorState_replaceLine(int line);
EditorState editorState_replaceString(int line, char *str, int strLength);
void editorState_findStringInLine(int line, char *str, int strLength);
void editorState_findStringInFile(char *str, int strLength);
void editorState_save(void);
void editorState_deleteLine(int line);
void editorState_moveUp(int line);
void editorState_moveDown(int line);
void printText(void);
void printLine(int line, char operation);
void printFileInfo(void);

/* === parsing.c === */

int parsing_getLine(char *line, int max, int trimSpace);

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

#define buf_free(b) ((b) ? (free(buf__hdr(b)), (b) = NULL) : 0)

void *buf__grow(const void *buf, size_t new_len, size_t elem_size);

/* === Text Editing Data Structures === */

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
        Line original; // The original line that was modified/deleted
    };
} Operation;

typedef struct Buffer {
    char *openedFilename; // char Streatchy buffer for the currently opened filename
    Line *lines;
    Operation lastOperation;
} Buffer;

Buffer currentBuffer;

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