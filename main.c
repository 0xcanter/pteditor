#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
struct termios orig_termios;

void disable_raw_mode(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    printf("\x1b[?1049l");
      fflush(stdout);
}

void enable_raw_mode()
{
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    atexit(disable_raw_mode);
    raw.c_lflag &= ~(ICANON | ECHO | ISIG);
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
        if (c == 3){
            break;
        }

    }
    char cee = 'c';
    int decimal = cee & 0x1F;
    printf("%d\n",decimal);
    return 0;
}
