#ifdef _WIN32
#include <windows.h>
#include <Wincon.h>
#endif

#include <stdio.h>
#include <stdarg.h>

#include "edimcoder.h"

void setColor(COLOR foreground)
{
#ifdef _WIN32
    WORD win32foreground;
    switch (foreground)
    {
        case COLOR_RED:
        {
            win32foreground = FOREGROUND_RED;
        } break;
        case COLOR_GREEN:
        {
            win32foreground = FOREGROUND_GREEN;
        } break;
        case COLOR_BLUE:
        {
            win32foreground = FOREGROUND_BLUE;
        } break;
        case COLOR_YELLOW:
        {
            win32foreground = FOREGROUND_YELLOW;
        } break;
        case COLOR_CYAN:
        {
            win32foreground = FOREGROUND_CYAN;
        } break;
        case COLOR_MAGENTA:
        {
            win32foreground = FOREGROUND_MAGENTA;
        } break;
        case COLOR_WHITE:
        {
            win32foreground = FOREGROUND_WHITE;
        } break;
        default:
        printf("ERROR: Wrong Color - Windows\n");
        return;
    }
    SetConsoleTextAttribute(hConsole, win32foreground);
#else
    switch (foreground)
    {
        case COLOR_RED:
        {
            printf(COL_RED);
        } break;
        case COLOR_GREEN:
        {
            printf(COL_GREEN);
        } break;
        case COLOR_BLUE:
        {
            printf(COL_BLUE);
        } break;
        case COLOR_YELLOW:
        {
            printf(COL_YELLOW);
        } break;
        case COLOR_CYAN:
        {
            printf(COL_CYAN);
        } break;
        case COLOR_MAGENTA:
        {
            printf(COL_MAGENTA);
        } break;
        case COLOR_WHITE:
        {
            printf(COL_RESET);
        } break;
        default:
        printf("ERROR: Wrong Color - Unix\n");
        return;
    }
#endif
}

void resetColor(void)
{
#ifdef _WIN32
    SetConsoleTextAttribute(hConsole, FOREGROUND_WHITE);
#else
    printf(COL_RESET);
#endif
}

void colors_printf(COLOR foreground, const char *fmt, ...)
{
    setColor(foreground);
    
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    
    resetColor();
}

void colors_puts(COLOR foreground, const char *fmt, ...)
{
    setColor(foreground);
    
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    
    resetColor();
    
    printf("\n");
}

void printError(const char *fmt, ...)
{
    setColor(COLOR_RED);
    
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    
    resetColor();
}

void printPrompt(const char *fmt, ...)
{
    setColor(COLOR_GREEN);
    
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    
    resetColor();
}

void printLineNumber(const char *fmt, ...)
{
    setColor(COLOR_YELLOW);
    
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    
    resetColor();
}
