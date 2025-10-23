#include "lib/rope.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
int main(){
  rope_node *root;
  char str[] = "templechuks";
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
    return 1;
  }  
 // delete_rope(root, 17,  &root, 4, &mem);
 rope_node *buff;
 fast_substr(root, 3, 1,&buff,&mem);
 insert_rope(root, 10, "temp", &root, &mem);
 rope_node *del;
 delete_rope(root, 10, &root, 4, &mem, &del);
 char t = find_char_rope(root, 10);
  print_rope(buff);
  printf("%c\n",t);
  free_ropes(root, &mem);
  free_ropes(buff,&mem);
  free_ropes(del, &mem);
  free_mem(&mem);
  // free(mem.arr);
  
  // free(mem.arr);
  return 0;
 // printf("total line is %zu\n",root->line_count);
}
