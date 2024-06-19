/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f) //Here, we define a macro. It takes an input 'k' and bitwise-ANDs it. If both bits are 1, it is set to 1. If not, it is set to 0.

/*** data ***/

struct editorConfig {
    int screenrows;     //Number of terminal rows
    int screencols;     //Number of terminal colours
    struct termios orig_termios;    //Declares a variable in which the terminal attributes will be stored
};

struct editorConfig E;

/*** terminal ***/

void die(const char *s) {   //Takes the value or errno and sets it as a constant at "s"
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);  //Prints the error message
    exit(1);    //Exits as failed (because there's an error)
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)  //Sets terminal attributes to their original state using tcsetattr
        die("tcsetattr");
}

void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr"); //Gets terminal attributes and stores them in orig_termios
    atexit(disableRawMode); //Runs the disableRawMode function when the program exits from main

    struct termios raw = E.orig_termios; //Separate structure which we will modify
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);   //Disable some input flags
    raw.c_oflag &= ~(OPOST);    //Disable some output flags
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);    //Disable some local flags
    raw.c_cc[VMIN] = 0; //This determines how many bytes must be present for read to return. Here we want an instant return, so we set it to 0.
    raw.c_cc[VTIME] = 1; //This determines the amount of time before read must return. Here it is 100ms.

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");   //Now we set the terminal attributes with our structure "raw". TCSAFLUSH discards any queued input and makes changes after all queued output has been written.
}

char editorReadKey() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {      //Proceeds while the value of the 1 byte read is != 1
        if (nread == -1 && errno != EAGAIN) die("read");    //Return an error if nread == -1 and errno != EAGAIN
    }
    return c;
}

int getCursorPosition(int *rows, int *cols) {
    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

    return 0;
}

int getWindowSize(int *rows, int *cols) {
    struct winsize ws;  //Structure to store terminal size

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {    //Return error if either of these conditions is true
        if (write(STDOUT_FILENO, "\x1b[999\x1b[999B", 12) != 12) return -1;
        return getCursorPosition(rows, cols);
    } else {
        *cols = ws.ws_col;  //Dereference operator to set the memory address of 'cols' to 'ws.ws_col
        *rows = ws.ws_row;  //Same as above but for rows
        return 0;
    }
}

/*** output ***/

void editorDrawRows() {
    int y;
    for (y = 0; y < E.screenrows; y++) {    //Prints a tilda on every row
        write(STDOUT_FILENO, "~", 1);

        if (y < E.screenrows - 1) {
            write(STDOUT_FILENO, "\r\n", 2);
        }
    }
}

void editorRefreshScreen() {
    write(STDOUT_FILENO, "\x1b[2J", 4);  //Clears the screen
    write(STDOUT_FILENO, "\x1b[H", 3);   //Positions the cursor in the top left

    editorDrawRows();

    write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

void editorProcessKeypress () {
    char c = editorReadKey();   //Gets user input from editorReadKey()

    switch (c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);        //Exits if the input is CTRL+q
            break;
    }
}

/*** init ***/

void initEditor() {
    if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");    //Returns an error if it cannot fetch the window size
}

int main() {
    enableRawMode();    //Enables raw mode
    initEditor();

    while(1) {  //Infinite loop for the text editor
        editorRefreshScreen();
        editorProcessKeypress();
    }

    return 0;   //Successful return
}