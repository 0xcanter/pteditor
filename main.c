#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
struct termios orig_termios;

void disable_raw_mode(){
    /* clear previous user input and set to previous terminal default settings */
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    /* exit alternate screen */
    printf("\x1b[?1049l");
    /* write out everything in the buffer still waiting to be printed in the terminal */
      fflush(stdout);
    printf("end\n");
    fflush(stdout);
}

void enable_raw_mode()
{
    /* get the terminal settings and save it in the orig_termios */
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    /* if the program terminates or end then disable raw mode and go back to original terminal settings */
    atexit(disable_raw_mode);

    /* disabling BRKINT which will make the program not to terminate if a break is triggered */
    /* disabling ISTRIP which will make the program to keep the full 8 bit charcters  */
    /* disabling ICRNL which will make the terminal not to do anything when \n or \r */
    raw.c_iflag &= ~(BRKINT | ICRNL | IXON | ISTRIP );
    raw.c_lflag &= ~(ICANON | ECHO | ISIG);
    raw.c_oflag &= ~OPOST;
    raw.c_cflag |= (CS8);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    printf("\x1b[?1049h");
      fflush(stdout);
}

void clear_screen() {
    printf("\x1b[2J");
    printf("\x1b[H");
    fflush(stdout);
}
int main(){

    enable_raw_mode();
    clear_screen();

    char c;

    while(1){
        if(read(STDIN_FILENO, &c, 1) == -1&&errno != EAGAIN ){
            perror("read");
            exit(1);
        }
        printf("%c",c);
        fflush(stdout);

        //exit with ctrl C
        if (c == 3){
                break;
        }
    }


    return 0;
}
