#include <stdio.h>
#include <malloc.h>

#include "lineeditor.h"

#ifdef _WIN32
#include <conio.h>
#define getch _getch

// Hack for clearing screen for Windows // TODO: Improve this
void clrscr() {
    system("cls");
}
#else
#include <unistd.h>
#include <termios.h>
char getch() {
    /*#include <unistd.h>   //_getch*/
    /*#include <termios.h>  //_getch*/
    char buf=0;
    struct termios old={0};
    fflush(stdout);
    if(tcgetattr(0, &old)<0)
        perror("tcsetattr()");
    old.c_lflag&=~ICANON;
    old.c_lflag&=~ECHO;
    old.c_cc[VMIN]=1;
    old.c_cc[VTIME]=0;
    if(tcsetattr(0, TCSANOW, &old)<0)
        perror("tcsetattr ICANON");
    if(read(0,&buf,1)<0)
        perror("read()");
    old.c_lflag|=ICANON;
    old.c_lflag|=ECHO;
    if(tcsetattr(0, TCSADRAIN, &old)<0)
        perror ("tcsetattr ~ICANON");
    return buf;
}

// Hack for clearing screen for Linux and Mac // TODO: Improve this
void clrscr() {
    //system("clear");
    printf("\e[1;1H\e[2J"); // Much better // TODO: Check whether actually portable though
}
#endif

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
    //char c;
    //printf("Test\b \b\n");
    //getInput();
    /*while ((c = getch()) != 'Q') {
        printf("%d\n", c);
    }*/
    
    char args[MAXLENGTH] = { 0 };
    int argsLength = 0;
    int running = true;
    State state = MAIN_MENU;
    State statePrev = state;
    
    printf("Edim - Ed Improved\n");
    printf("Copyright (c) Christian Seibold. All Rights Reserved.\n\n");
    
    printf("Press '?' for help.\n");
    
    // Open Scratch Buffer
    // Then, go straight to prompt
    
    printf("Opening '-Scratch-' Buffer\n");
    buffers = NULL;
    {
        Buffer buffer;
        buffer_initEmptyBuffer(&buffer);
        buffer.modified = true;
        buffer.fileType = FT_MARKDOWN;
        
        char *name = "-Scratch-";
        for (int i = 0; i < strlen(name); i++) {
            buf_push(buffer.openedFilename, name[i]);
        }
        
        buffer.currentLine = 0;
        
        buf_push(buffers, buffer);
        currentBuffer = buf_end(buffers) - 1;
    }
    
    while (running) {
        EditorState state = editorState_menu();
        
        switch (state) {
            case ED_EXIT:
            {
                if (currentBuffer->modified) {
                    if (currentBuffer - buffers == 0) {
                        // TODO: Save changes and close buffer if last one open
                        if (buf_len(buffers) > 1) {
                            printError("You cannot close the '-Scratch-' buffer when other buffers are open.");
                        } else {
                            exit(0);
                        }
                    } else {
                        printError("There are unsaved changes. Use 'E' or 'Q' to close without changes.");
                    }
                    
                } else {
                    int isLast = false;
                    if (currentBuffer == &(buffers[buf_len(buffers) - 1]))
                        isLast = true;
                    buffer_close(currentBuffer);
                    if (isLast) {
                        buf_pop(buffers);
                        currentBuffer = buf_end(buffers) - 1;
                    } else {
                        Buffer *source = currentBuffer + 1;
                        Buffer *destination = currentBuffer;
                        int amtToMove = buf_end(buffers) - source;
                        memmove(destination, source, sizeof(Buffer) * amtToMove);
                        buf_pop(buffers);
                    }
                    if (buf_len(buffers) <= 0) {
                        exit(0);
                    }
                }
            } break;
            case ED_FORCE_EXIT:
            {
                int isLast = false;
                if (currentBuffer == &(buffers[buf_len(buffers) - 1]))
                    isLast = true;
                buffer_close(currentBuffer);
                if (isLast) {
                    buf_pop(buffers);
                    currentBuffer = buf_end(buffers) - 1;
                } else {
                    Buffer *source = currentBuffer + 1;
                    Buffer *destination = currentBuffer;
                    int amtToMove = buf_end(buffers) - source;
                    memmove(destination, source, sizeof(Buffer) * amtToMove);
                    buf_pop(buffers);
                }
                if (buf_len(buffers) <= 0) {
                    exit(0);
                }
            } break;
            case ED_QUIT:
            {
                int canQuit = true;
                
                // Check that no buffers (aside from -Scratch-, which will always be buffer 0) are modified.
                for (int i = 1; i < buf_len(buffers); i++) {
                    if (buffers[i].modified == true) {
                        canQuit = false;
                        break;
                    }
                }
                
                // TODO: If -Scratch- buffer is modified, save it.
                if (buffers[0].modified) {
                }
                
                if (!canQuit) {
                    printError("There are unsaved changes in at least one of the open buffers. Use 'E' or 'Q' to close without changes.");
                } else exit(0);
            } break;
            case ED_FORCE_QUIT:
            {
                exit(0);
            } break;
        }
        /*switch (state) {
            case MAIN_MENU:
            {
                statePrev = NEW_FILE;
                state = NEW_FILE;
                state = editorState(ED_MENU, args, argsLength);
                if (state == KEEP) state = statePrev;
                //state = mainMenu(args, &argsLength);
                //if (state == KEEP) state = statePrev;
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
        
        statePrev = state;*/
    }
    
    return(0);
}
