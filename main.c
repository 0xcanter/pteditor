#include "lib/rope.h"
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>

struct termios orig_termios;
typedef int ReturnCode ;
typedef int Position;
typedef  unsigned char Item;

typedef struct Point{
    int x;
    int y;
    int px;
    int py;
}Point;

typedef struct config{
    char *filename;
    int dirty;
    mem_for_special mem;
    rope_node *rope;
    rope_node **paste_leaves;
    size_t pcount;
    int p;
}config ;

static config C;

typedef struct buff_lines{
    unsigned char *str;
    int len;
    int buffer_line; 
}buff_lines;

void get_window_size(struct winsize *ws){
    ioctl(STDOUT_FILENO, TIOCGWINSZ, ws);
}

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
    if (sequence->capacity <= sequence->length+1){

        size_t new_capacity = sequence->capacity == 0 ? 2 : (sequence->capacity*2) + 1;

        void *tmp = realloc(sequence->data, new_capacity * sizeof(Item));
        if(!tmp){
            perror("memory reallocation failed");
            return -1;
        }
        sequence->data = tmp;
        sequence->capacity = new_capacity;
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
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    write(STDOUT_FILENO, "\x1b[?1049h", 8);

}

void clear_screen(void) {
    write(STDOUT_FILENO, "\x1b[H", 3);
}

void write_to_file(const char *filename,const char *buff,size_t byte){
    if(!buff)return;
    if(!filename){
        return;
     }   
    FILE *file = fopen(filename, "w");
    if (!file){
        perror("could not open file");
        return;
    }
    fwrite(buff,1 ,byte, file);
    fclose(file);
}

enum {
    BACKSPACE = 127,
    ESC = 27,
    ENTER = 13,
    PASTE = 200,
    PASTE_END = 201,
    TAB = 9,
    ARROW_LEFT = 1000,
    ARROW_UP,
    ARROW_RIGHT,
    ARROW_DOWN,
    HOME,
    END,
    DEL,
    PAGE_UP,
    PAGE_DOWN,
};

void get_cursor_position(int *rows,int *cols){
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

int is_paste_end(char buff[],int size){
    int i;
    for(i = 0;i<size;i++){
        if(buff[i] == ESC){
            return i;
        }
    }
    return -1;
}

int read_key(int fd){
    char c,seq[7];
    int nread;
    while((nread = read(fd,&c,1)) == 0);
    if (nread == -1)exit(1);

    while(1){
        switch(c){
            case ESC:
                if(read(fd,seq,1) == 0) return ESC;
                if(read(fd,seq+1,1) == 0) return ESC;
                if(seq[0] == '['){
                    if(seq[1] >= '0' && seq[1] <= '9'){
                        if(read(fd,seq+2,1) == 0) return ESC;
                        if(seq[2] == '0'){
                            if(read(fd,seq+3,1) == 0)return ESC;
                            if(read(fd,seq+4,1) == 0)return ESC;
                            if(seq[4] == '~'){
                                char buff[4096];
                                int nread;
                                size_t cap = 8;
                                C.pcount = 0;
                                C.paste_leaves = malloc(sizeof(rope_node*) * cap);
                                C.p = 0;
                                switch(seq[3]){
                                    case '0':
                                        while(1){
                                            if((nread = read(fd,buff,sizeof(buff) - 1)) == 0){
                                                C.p = 0;
                                            }
                                            int count = is_paste_end(buff, nread);
                                            if(count >= 0){
                                                buff[count] = '\0';
                                                if(C.pcount >= cap){
                                                    cap *= 2;
                                                    C.paste_leaves = realloc(C.paste_leaves, sizeof(rope_node*) * cap);
                                                }
                                                C.paste_leaves[C.pcount++] = make_leaf_paste(buff, count );
                                                return PASTE_END;
                                            }else{

                                                if(C.pcount >= cap){
                                                    buff[nread] = '\0';
                                                    cap *= 2;
                                                    C.paste_leaves = realloc(C.paste_leaves, sizeof(rope_node*) * cap);
                                                }
                                                C.paste_leaves[C.pcount++] = make_leaf_paste(buff, nread);
                                                
                                            }
                                            
                                                
                                            
                                       }
                                        break;
                                }
                            }
                        }
                        if(seq[2] == '~'){
                            switch(seq[1]){
                                case '3': return DEL;
                                case '5': return PAGE_UP;
                                case '6': return PAGE_DOWN;
                                case '1': return HOME;
                                case '4': return END;
                            }
                        }
                    }else{
                        switch(seq[1]){
                            case 'A': return ARROW_UP;
                            case 'B': return ARROW_DOWN;
                            case 'C': return ARROW_RIGHT;
                            case 'D': return ARROW_LEFT;
                            case '1': return HOME;
                            case 'F': return END;
                        }
                    }
                }
                 break;
             default:
                 return c;
        }
    }
    
}    



void move_cursor(struct Point *p ,int dx,int dy){
    p->x += dx;
    p->y += dy;
    if(p->x < 1)p->x = 1;
    if(p->y < 1)p->y = 1;
    char seq[32];
    snprintf(seq, sizeof(seq), "\x1b[%d;%dH", p->y,p->x);
    write(STDOUT_FILENO, seq, strlen(seq));
}



void insert_to_buff(unsigned char buff[],unsigned char c,size_t buff_count){
    buff[buff_count] =  c;
}

void render_lines(){
    
}

void editor_draw_status_bar(struct Point *p){
    int x,y;
    struct winsize ws;
    get_window_size(&ws);
    y = ws.ws_row - 1;
    x = ws.ws_col;
    write(STDOUT_FILENO,"\x1b[?25l",6);
    write(STDOUT_FILENO,"\x1b[s",3);
    p->px = p->x;
    p->py = p->y;
    p->y = 0;
    move_cursor(p, 0, y);
    write(STDOUT_FILENO,"\033[2K\r",5);
    char buf[50];
    int size = snprintf(buf, sizeof(buf), "\x1b[47m\x1b[KThis whole line %d:%d is white!\x1b[0m", p->py,p->px);
    write(STDOUT_FILENO,buf,size);
    write(STDOUT_FILENO,"\x1b[u",3);
    write(STDOUT_FILENO,"\x1b[?25h",6);
    p->x = p->px;
    p->y = p->py;
    move_cursor(p, 0, 0);
    // p->y = 0;
}

void write_editor(unsigned char c,size_t *buff_count,unsigned char buff[],rope_node **root,struct Point *p){
    
     static unsigned char ct[5];
     static int count = 0,expected = 0;
     
     if (count == 0) {
        if (c < 0x80) expected = 1;
        else if ((c & 0xE0) == 0xC0) expected = 2;
        else if ((c & 0xF0) == 0xE0) expected = 3;
        else if ((c & 0xF8) == 0xF0) expected = 4;
        else expected = 1; 
     }
     ct[count++] = c;
     
     if (count == expected) {
         ct[count] = '\0';
         if (*buff_count >= (CHUNK_SIZE * 4 - 1)) {
           rope_append(root,(const char *)buff);
           *buff_count = 0;
         }
         write(STDOUT_FILENO,ct ,count);
         if(count == expected){
             for(size_t i = 0;i < count;i++){
                 if(ct[i] == '\0')break;
                 insert_to_buff(buff, ct[i], *buff_count);
                 *buff_count += 1;
                 buff[*buff_count] = '\0';
             }
         }else{
                 insert_to_buff(buff, c, *buff_count);
                 *buff_count += 1;
                 buff[*buff_count] = '\0';
         }
         if(count > 2)move_cursor(p, 1, 0);
         else move_cursor(p, 1, 0);
         if(C.p == 1)
             editor_draw_status_bar(p);
         count = 0;  
     }
}


void init(Point *p){
    
    Sequence s = empty();
    enable_raw_mode();
    clear_screen();
    write(STDOUT_FILENO,"\x1b[?2004h",8);
    size_t buff_count = 0;
    unsigned char buff[CHUNK_SIZE * 4] ;
    editor_draw_status_bar(p);
    if(C.filename && C.rope != NULL){
        int line = lines(C.rope);
        if(line == 0){
            
            unsigned char *t = flatten_to_string(C.rope);
            write(STDOUT_FILENO,(char*)t,strlen((char*)t));
            move_cursor(p, strlen((char *)t),0);
            free(t);
            editor_draw_status_bar(p);
            
        }else{
            int i = line - 30;
            for(;i<= line;i++){
            
                rope_node *d;
                d = NULL;
                rope_slice(C.rope, i, 1, &C.mem, &d);
                unsigned char *t = flatten_to_string(d);
                write(STDOUT_FILENO,(char*)t,strlen((char*)t));
                free(t);
                p->x = 1;
                move_cursor(p, 0, 1);
                editor_draw_status_bar(p);
            }
            
        }
    }
    
    while(1){
        int c = read_key(STDIN_FILENO);
        // editor_draw_status_bar(&p);
        switch (c) {
            case ENTER:
                
                if(buff_count >= (CHUNK_SIZE * 4 - 1)){
                    rope_append(&C.rope, (const char *)buff);
                    buff_count = 0;
                }
                insert_to_buff(buff, '\n', buff_count);
                buff_count++;
                buff[buff_count] = '\0';
                // write(STDOUT_FILENO,"\n",1);
                p->x = 1;
                p->y += 1;
                move_cursor(p, 0, 0);
                editor_draw_status_bar(p);
                break;
            case ARROW_DOWN:
                if(p->y < 1)p->y = 1;
                move_cursor(p,0 , 1);
                editor_draw_status_bar(p);
                break;
            case ARROW_UP:
                if(p->y < 1)p->y = 1;
                move_cursor(p, 0, -1);
                editor_draw_status_bar(p);
                break;
            case ARROW_LEFT:
                if(p->x < 1)p->x = 1;
                move_cursor(p, -1, 0);
                editor_draw_status_bar(p);
                break;
            case ARROW_RIGHT:
                if(p->x < 1)p->x = 1;
                move_cursor(p, 1, 0); 
                editor_draw_status_bar(p);
                 break;
            case PAGE_UP:
                break;
            case PAGE_DOWN:
                break;
            case ESC:
                if((CHUNK_SIZE * 4 - 1) >= buff_count)rope_append(&C.rope, (const char *)buff);
                if(C.rope == NULL && s.data != NULL){
                    sleep(3);
                    free(s.data);
                    return;
                    break;
                }
                unsigned char *stt;
                stt = flatten_to_string(C.rope);
                write_to_file(C.filename, (const char*)stt,strlen((const char *)stt));
                free_ropes(C.rope, &C.mem);
                free_mem(&C.mem);
                free(stt);
                free(s.data);
                return ;
                break;
            case DEL:
                break;
            case HOME:
                break;
            case END:
                break;
            case PASTE_END:
                C.p = 1;
                p->x = 1;
                // move_cursor(p, 0, 0);
                if(C.paste_leaves != NULL){
                    
                    C.rope = build_balanced_rope(C.paste_leaves, C.pcount);
                    C.pcount = 0;
                    free(C.paste_leaves);
                    C.paste_leaves = NULL;
                    size_t n = lines(C.rope);
                    size_t i = n > 38 ? n-38:0;

                    for(;i<n;i++){
                        rope_node *t;
                        t = NULL;
                        rope_slice(C.rope, i,1, &C.mem, &t);
                        unsigned char *d = flatten_to_string(t);
                        write(STDOUT_FILENO,(char *)d,strlen((char *)d));
                        free(d);
                        // free(t);
                        // p->x = 1;
                        move_cursor(p, 0, 1);
                        editor_draw_status_bar(p);
                    }
                    
                }
                break;
            case 10:
                
                if(buff_count >= (CHUNK_SIZE * 4 - 1)){
                    rope_append(&C.rope, (const char *)buff);
                    buff_count = 0;
                }
                insert_to_buff(buff, '\n', buff_count);
                buff_count++;
                buff[buff_count] = '\0';
                p->x = 1;
                p->y += 1;
                move_cursor(p, 0, 0);
                editor_draw_status_bar(p);
                break;
            case BACKSPACE:

                if(p->x > 0){
                    write(STDOUT_FILENO, "\b ", 3);
                    move_cursor(p, -1,0);
                    char seq[32];
                    snprintf(seq, sizeof(seq), "\033[%d;%dH",p->y,p->x);
                    write(STDOUT_FILENO, seq, 7);
                 }   
                break;
            default:
                write_editor(c, &buff_count, buff, &C.rope, p);
                break;
        }
        
    }
}

void open_file(char *filename){
    
}
void openfile(FILE *f){
        
  
  
  fseek(f,0,SEEK_END);
  size_t size = ftell(f);
  rewind(f);
  unsigned char *str = malloc(size+1);
  if(!str){
    perror("malloc failed");
    fclose(f);
  }
  long long fr = fread(str, 1, size, f);
  str[size] = '\0';
  fclose(f);
  rope_node **leaves;
  leaves = NULL;
  long cap = 0,count = 0;
  clock_t start,end;
  double cpu_time_used;
  for(size_t i = 0;i < size; i+=(CHUNK_SIZE * 4) ){
      static int trails = 0;
      size_t len = (i + (CHUNK_SIZE *4) < size) ? (CHUNK_SIZE * 4) : size - i;
      if(count >= cap){
        cap = (cap == 0) ? 8 : cap * 2;
        leaves = realloc(leaves,sizeof(rope_node *) * cap);
      }
      unsigned char *buf = malloc((CHUNK_SIZE * 4) + 1);
      memcpy(buf, str + i,len);
      buf[len] = '\0';
      trails++;
      leaves[count++] = make_leaf_owned(buf, len);
  }
  free(str);
  C.rope = build_balanced_rope(leaves, count);
  free(leaves);
}

int main(int argc,char **argv){
    if(argc > 2){
            fprintf(stderr,"Usage: ptex <filename> OR ptex\n");
            exit(1);
     }
    
    Point p = {1,1};
    C.filename = argv[1];
    C.rope = NULL;
    init_mem_f_s(&C.mem, 1);
    FILE *f = fopen(C.filename, "r");
    if(f){
        openfile(f);
        // return 0;
    }
    
    init(&p);
    return 0;
}

