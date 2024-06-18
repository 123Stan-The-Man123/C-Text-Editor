#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;    //Declares a variable in which the terminal attributes will be stored

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);  //Sets terminal attributes to their original state using tcsetattr
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios); //Gets terminal attributes and stores them in orig_termios
    atexit(disableRawMode); //Runs the disableRawMode function when the program exits from main

    struct termios raw = orig_termios; //Separate structure which we will modify
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);   //Disable some input flags
    raw.c_oflag &= ~(OPOST);    //Disable some output flags
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);    //Disable some local flags
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);   //Now we set the terminal attributes with our structure "raw". TCSAFLUSH discards any queued input and makes changes after all queued output has been written.
}

int main() {
    enableRawMode();    //Enables raw mode

    while(1) {  //Infinite loop for the text editor
        char c = '\0';  //This is where the users input will be stored temporarily
        read(STDIN_FILENO, &c, 1);  //Reads the users input. Requires the amount of bytes (here 1).
        if (iscntrl(c)) {   //If the input is a control code then only print the ASCII value
            printf("%d\r\n", c);
        } else {    //Else print both character and ASCII value
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q') break;    //Breaks the infinite while loop if the user input is 'q'
    }

    return 0;   //Successful return
}