#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
struct termios orig_termios;

typedef int ReturnCode ;
typedef int Position;
typedef unsigned char Item;

typedef struct {
    Item *data;
    int length;
    int capacity;
} Sequence;

Sequence empty(){
    Sequence s ;
    s.capacity = 8;
    s.length = 0;
    s.data = malloc(s.capacity * sizeof(Item));
    if (!s.data){
        perror("memory reallocation failed");
        s.capacity = 0;
    }
    return s;
}

ReturnCode insert(Sequence *sequence,Position position,Item *ch){
    if(position > sequence->length ||position < 0) return -1;
    if(!sequence->data)return -1;
    if (sequence->capacity == sequence->length){
        sequence->capacity *= 2;
        sequence->data = realloc(sequence->data, sequence->capacity * sizeof(Item));
        if(!sequence->data){
            perror("memory reallocation failed");
            return -1;
        }
    }
    for(int i = sequence->length;i > position;i--){
        sequence->data[i] = sequence->data[i - 1];
    }
    sequence->data[position] = *ch;
    sequence->length++;
    sequence->data[sequence->length] = '\0';
    return 1;
}

ReturnCode delete_at(Sequence *sequence,Position position){
    if(position >= sequence->length || position < 0) return -1;
    if(!sequence->data)return -1;
    for(int i = position;i < sequence->length - 1; i++){
        sequence->data[i] = sequence->data[i+1];
    }
    sequence->length--;
    sequence->data[sequence->length] = '\0';
    return 1;
}


ReturnCode itemAt(Sequence *sequence,Position position,Item *out){
    if(!sequence || !sequence->data || position < 0 || position >= sequence->length) return -1;
    *out = sequence->data[position];
    return 1;
}


ReturnCode load_file(Sequence *sequence,const char *filename){
    FILE *f = fopen(filename, "r");
    if(!f){
        perror("Could not open file!");
        return -1;
    }

    int c;
    while((c = fgetc(f)) != EOF){
        Item ch = (unsigned char)c;
        if (insert(sequence, sequence->length, &ch) == -1){
            fclose(f);
            return -1;
        }
    }
    fclose(f);
    return 1;
}

ReturnCode close_sequence(Sequence *sequence){
    if (!sequence)return -1;
    if (sequence->data){
        free(sequence->data);
        sequence->data = NULL;
    }
    sequence->length = 0;
    sequence->capacity =  0;
    return 1;
}


void disable_raw_mode(void){
    /* clear previous user input and set to previous terminal default settings */
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    /* exit alternate screen */
    write(STDOUT_FILENO,"\x1b[?1049l" , 8);
    /* write out everything in the buffer still waiting to be printed in the terminal */

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
    write(STDOUT_FILENO, "\x1b[?1049h", 8);

}

/* the clear screen function */
void clear_screen(void) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
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

enum {
    ESC = 27,
    ARROW_LEFT = 1000,
    ARROW_UP,
    ARROW_RIGHT,
    ARROW_DOWN,
    DEL,
    PAGE_UP,
    PAGE_DOWN,
};

void get_cursor_position(int *rows,int *cols){
    // get cursor position
    write(STDOUT_FILENO, "\x1b[6n", 4);
    char buff[32];
    int i = 0;

    while(i < sizeof(buff) - 1){
        if(read(STDIN_FILENO,&buff[i], 1) != 1) break;
        if(buff[i] == 'R')break;
        i++;
    }
    buff[i] = '\0';
    if(buff[0] == '\x1b' && buff[1] == '['){
        sscanf(&buff[2], "%d;%d", rows,cols);
    }
}

int read_key(int fd){
    char c,seq[3];
    int nread;
    nread = read(fd,&c, 1);
    if(nread == -1) exit(1);

        switch (c) {
            case ESC:
                if(read(fd, seq, 1) == 0)return ESC;
                if(read(fd,seq+1,1) == 0)return ESC;

                if(seq[0] == '['){
                    if(seq[1] >= '0' && seq[1] <= '9'){

                        if(read(fd, seq+2, 1) == 0)return ESC;
                            if(seq[2] == '~'){
                            switch (seq[1]) {
                                    case '3': return DEL;
                                    case '5': return PAGE_UP;
                                    case '6': return PAGE_DOWN;
                             }
                        }
                    }else{
                        switch (seq[1]) {
                            case 'A': return ARROW_UP;
                            case 'B': return ARROW_DOWN;
                            case 'C': return ARROW_RIGHT;
                            case 'D': return ARROW_LEFT;
                        }
                    }
                }
                break;
            default:
                return c;
        }
        return c;
}


struct Point{
    int x;
    int y;
};

void move_cursor(struct Point *p ,int dx,int dy){
    p->x += dx;
    p->y += dy;
        char seq[32];
        snprintf(seq, sizeof(seq), "\x1b[%d;%dH", p->y,p->x);
        write(STDOUT_FILENO, seq, strlen(seq));
}

int main(void){
    struct Point p = {1,1};
    enable_raw_mode();
    clear_screen();
    write(STDOUT_FILENO, "\x1b[1;1H", 6);
    while(1){
        int c = read_key(STDIN_FILENO);
        switch (c) {
            case ARROW_DOWN:
                if(p.y < 1)p.y = 1;
                move_cursor(&p,0 , 1);
                break;
            case ARROW_UP:
                if(p.y < 1)p.y = 1;
                move_cursor(&p, 0, -1);
                break;
            case ARROW_LEFT:
                if(p.x < 1)p.x = 1;
                move_cursor(&p, -1, 0);
                break;
            case ARROW_RIGHT:
                if(p.x < 1)p.x = 1;
                move_cursor(&p, 1, 0);
                break;
            case ESC:
                return 1;
                break;


        }
        if (c >= 32 && c <= 126)
            write(STDOUT_FILENO, &c, 1);

        // //exit with ctrl C which sends ASCII 3 (ETX), used here to manually exit since ISIG is disabled
        // if (c == 27){
        //         break;
        // }

        // if (c == '\r'){
        //     write(STDOUT_FILENO, "\n",1);
        // }
    }
    char *d = readfile("test.txt");
    if (d){
        write_to_file("tender.txt",d);
        free(d);
    }
    return 0;
}
