#include <stdio.h>
#include <malloc.h>

#include "lineeditor.h"

State mainMenu(char *args, int *argsLength) {
    /* Prompt */
    printPrompt("\n<> ");
    
    /* get first character - the menu item */
    char c;
    c = getchar();
    
    /* Store rest of line in rest */
    *argsLength = parsing_getLine(args, MAXLENGTH / 4, true);
    
    printf("\n");
    
    switch (c) {
        case '?':
        {
            printf(" * 'n' - New File\n");
            printf(" * 'o' - Open File\n");
            printf(" * 'q' or 'Q' - Quit\n");
        } break;
        case 'n':
        {
            return NEW_FILE;
        } break;
        case 'o':
        {
            return OPEN_FILE;
        } break;
        case 'q':
        case 'Q':
        {
            return QUIT;
        } break;
        default:
        printError("Unknown command");
    }
    
    return KEEP;
}

int main() {
#ifdef _WIN32
    // Used for printing in color on Windows
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
    
    char args[MAXLENGTH] = { 0 };
    int argsLength = 0;
    int running = true;
    State state = MAIN_MENU;
    State statePrev = state;
    
    printf("Edim - Ed Improved\n");
    printf("Copyright (c) Christian Seibold. All Rights Reserved.\n\n");
    
    printf("Press '?' for help.\n");
    
    while (running) {
        switch (state) {
            case MAIN_MENU:
            {
                state = mainMenu(args, &argsLength);
                if (state == KEEP) state = statePrev;
            } break;
            case NEW_FILE:
            {
                printf("\n");
                state = editorState(ED_NEW, args, argsLength);
                if (state == KEEP) state = statePrev;
            } break;
            case OPEN_FILE:
            {
                state = editorState(ED_OPEN, args, argsLength);
                if (state == KEEP) state = statePrev;
            } break;
            case QUIT:
            {
                running = false;
                break;
            }
        }
        
        statePrev = state;
    }
    
    return(0);
}
