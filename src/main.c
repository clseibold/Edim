#include "edimcoder.h"

#ifdef _WIN32

#include <conio.h>
#define getch _getch
#define kbhit _kbhit

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

char getch_nonblocking() {
    /*#include <unistd.h>   //_getch*/
    /*#include <termios.h>  //_getch*/
    char buf=0;
    struct termios old={0};
    fflush(stdout);
    if(tcgetattr(0, &old)<0)
        perror("tcsetattr()");
    old.c_lflag&=~ICANON;
    old.c_lflag&=~ECHO;
    old.c_cc[VMIN]=0;
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

int main(int argc, char **argv) {
#ifdef _WIN32
    // Used for printing in color on Windows
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    printf("Testing\n\n");
    
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    COORD pos = { consoleInfo.dwCursorPosition.X, consoleInfo.dwCursorPosition.Y - 1 };
    SetConsoleCursorPosition(hConsole, pos);
#endif
    //char c;
    //printf("Test\b \b\n");
    //getInput();
    /*while ((c = getch()) != 'Q') {
        printf("%d\n", c);
    }*/

    /*char *input = NULL; // TODO: Free at end
    bool canceled = false;
    input = getInput(&canceled, input, NULL);
    if (canceled || input == NULL || buf_len(input) == 0 || (buf_len(input) == 1 && input[0] == '\n')) {
        buf_free(input);
        return 0;
    }*/
    
    char args[MAXLENGTH] = { 0 };
    int argsLength = 0;
    int running = true;
    
    printf("Edim - Ed Improved\n");
    printf("Copyright (c) Christian Seibold. All Rights Reserved.\n\n");
    
    printf("Press 'h' for help.\n");
    
    // Open Scratch Buffer
    // Then, go straight to prompt
    
    if (argc <= 1)
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
    
    // Open file that was passed into executable
    // TODO: Do this for multiple files passed in?
    if (argc > 1) {
        {
            Buffer buffer;
            buffer_initEmptyBuffer(&buffer);
            buf_push(buffers, buffer);
            currentBuffer = buf_end(buffers) - 1;
        }
        
        if (!buffer_openFile(currentBuffer, argv[1])) {
            printf("File doesn't exist... Creating it.\n\n", argv[1]);
            currentBuffer->modified = true;
            editorState_editor();
        } else {
            printf("Opening '%s'\n\n", argv[1]);
            printFileInfo();
        }
    }
    
    while (running) {
        State state = editorState_menu();
        
        switch (state) {
            case EXIT:
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
            case FORCE_EXIT:
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
            case QUIT:
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
            case FORCE_QUIT:
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
