#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lib/rope.h"
int main(){
  FILE *f = fopen("test2.txt", "r");
  
  fseek(f,0,SEEK_END);
  size_t size = ftell(f);
  rewind(f);
  char *str = malloc(size+1);
  if(!str){
    perror("malloc failed");
    fclose(f);
  }
  fread(str, 1, size, f);
  str[size] = '\0';
  fclose(f);
  rope_node **leaves;
  leaves = NULL;
  long cap = 0,count = 0; 
  for(size_t i = 0;i < size; i+=CHUNK_SIZE){
      size_t len = (i + CHUNK_SIZE < size) ? CHUNK_SIZE : size - i;
      if(count >= cap){
        cap = (cap == 0) ? 8 : cap * 2;
        leaves = realloc(leaves,sizeof(rope_node *) * cap);
      }
      char *buf = malloc(CHUNK_SIZE + 1);
      memcpy(buf, str + i,len);
      buf[len] = '\0';
      leaves[count++] = make_leaf_owned(buf, len);
  }
  rope_node *root = build_balanced_rope(leaves, count);
  free(leaves);
  mem_for_special mem;
  init_mem_f_s(&mem, 1); 
  free_ropes(root,&mem);
  free_mem(&mem);
  free(str);
  return 0;
}
