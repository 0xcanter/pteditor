#include "lib/rope.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
int main(){
  rope_node *root;
  char str[] = "hello\n my\n name\n is\n temples\n";
  int len = strlen(str)/2;
  printf("%zu\n",strlen(str));
  
  rope_node *left,*right;
  char *str2 = str+len;
  right = make_leaf(str2);
  str[len] = '\0';
  char *str1 = str;

  left = make_leaf(str1);
  root = concat(left,right);
  mem_for_special mem;
  init_mem_f_s(&mem, 1);
  if(mem.arr == NULL) {
    perror("initialization of mem failed");
  }  
 // delete_rope(root, 17,  &root, 4, &mem);
  print_rope(root);
  free_ropes(root, &mem);
  free_mem(&mem);
  free(mem.arr);
 // printf("total line is %zu\n",root->line_count);
}
