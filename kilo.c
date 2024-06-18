/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f) //Here, we define a macro. It takes an input 'k' and bitwise-ANDs it. If both bits are 1, it is set to 1. If not, it is set to 0.

/*** data ***/

struct termios orig_termios;    //Declares a variable in which the terminal attributes will be stored

/*** terminal ***/

void die(const char *s) {   //Takes the value or errno and sets it as a constant at "s"
    perror(s);  //Prints the error message
    exit(1);    //Exits as failed (because there's an error)
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)  //Sets terminal attributes to their original state using tcsetattr
        die("tcsetattr");
}

void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr"); //Gets terminal attributes and stores them in orig_termios
    atexit(disableRawMode); //Runs the disableRawMode function when the program exits from main

    struct termios raw = orig_termios; //Separate structure which we will modify
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


/*** output ***/

void editorRefreshScreen() {
    write(STDOUT_FILENO, "\x1b[2J", 4);  //Clears the screen
}

/*** input ***/

void editorProcessKeypress () {
    char c = editorReadKey();   //Gets user input from editorReadKey()

    switch (c) {
        case CTRL_KEY('q'):
            exit(0);        //Exits if the input is CTRL+q
            break;
    }
}

/*** init ***/

int main() {
    enableRawMode();    //Enables raw mode

    while(1) {  //Infinite loop for the text editor
        editorRefreshScreen();
        editorProcessKeypress();
    }

    return 0;   //Successful return
}