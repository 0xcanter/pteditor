#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
struct termios orig_termios;

void disable_raw_mode(void){
    /* clear previous user input and set to previous terminal default settings */
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    /* exit alternate screen */
    printf("\x1b[?1049l");
    /* write out everything in the buffer still waiting to be printed in the terminal */
      fflush(stdout);
    printf("end\n");
    fflush(stdout);
}

char *readfile(const char *filename){
    FILE *file = fopen(filename, "r");
    if(!file){
        perror("could not open file");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *buffer = malloc(size+1);
    if (!buffer){
        perror("Memory allocation failed!");
        fclose(file);
        return NULL;
    }
    fread(buffer, 1, size , file);
    buffer[size] = '\0';
    fclose(file);

    return buffer;
}

void enable_raw_mode(void)
{
    /* get the terminal settings and save it in the orig_termios */
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    /* if the program terminates or end then disable raw mode and go back to original terminal settings */
    atexit(disable_raw_mode);
                               // c_iflag DISABLE keywords
    /* disabling BRKINT  which will make the program not to terminate if a break is triggered
       disabling ISTRIP which will make the program to keep the full 8 bit charcters
       disabling ICRNL prevent the terminal from converting \r to \n when a return key is pressed
       disabling IXON prevent CTRL S AND  CTRL Q from pausing and resuming the terminal */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | IXON | ISTRIP );

                              // c_lflag DISABLE keywords
    /* disabling ICANON will give you full manual control over user input
       disabling ECHO prevents the terminal from echoing back user input
       disabling ISIG suspends ^C and ^Z from suspending or killing the program */
    raw.c_lflag &= ~(ICANON | ECHO | IEXTEN | ISIG);

                            // c_oflag DISABLE keywords
    /* disabling OPOST this prevents the terminal trigering \r\n when the return key is pressed...it just send input as it is */
    raw.c_oflag &= ~OPOST;

                            // c_cflag ENABLE keyword
    /* enabling  CS8 sets character site to 8bit per byte*/
    raw.c_cflag |= (CS8);

    // set the new terminal settings and  printing the ansii charcters to create alternate screen
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    printf("\x1b[?1049h");
      fflush(stdout);
}

/* the clear screen function */
void clear_screen(void) {
    printf("\x1b[2J");
    printf("\x1b[H");
    fflush(stdout);
}

void write_to_file(const char *filename,const char *value){
    FILE *file = fopen(filename, "w");
    if (!file){
        perror("could not open file");
        return;
    }
    fputs(value, file);
    fclose(file);
}

int main(void){
    enable_raw_mode();
    clear_screen();
    char c;
    printf("1");
    fflush(stdout);
    while(1){
        if(read(STDIN_FILENO, &c, 1) == -1&&errno != EAGAIN ){
            perror("read");
            exit(1);
        }
        printf("%c",c);
        fflush(stdout);

        //exit with ctrl C which sends ASCII 3 (ETX), used here to manually exit since ISIG is disabled
        if (c == 3){
                break;
        }

        if (c == '\r'){
            printf("\n%d",2);
            fflush(stdout);
        }
    }
    char *d = readfile("test.txt");
    write_to_file("tender.txt",d);
    free(d);
    return 0;
}
