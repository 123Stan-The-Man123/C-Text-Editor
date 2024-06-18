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

/*** init ***/

int main() {
    enableRawMode();    //Enables raw mode

    while(1) {  //Infinite loop for the text editor
        char c = '\0';  //This is where the users input will be stored temporarily
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die ("read");  //Reads the users input. Requires the amount of bytes (here 1).
        if (iscntrl(c)) {   //If the input is a control code then only print the ASCII value
            printf("%d\r\n", c);
        } else {    //Else print both character and ASCII value
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == CTRL_KEY('q')) break;    //Breaks the infinite while loop if the user input is 'q'
    }

    return 0;   //Successful return
}