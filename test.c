#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "lib/rope.h"
int main(){
  FILE *f = fopen("test2.txt", "r");
  
  fseek(f,0,SEEK_END);
  size_t size = ftell(f);
  rewind(f);
  unsigned char *str = malloc(size+1);
  if(!str){
    perror("malloc failed");
    fclose(f);
  }
  long long fr = fread(str, 1, size, f);
  printf("%llu\n",fr);
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
      // if(trails == 0)printf("%s\n",(char *)buf);
      // printf("first: %zu",strlen((char *)buf));
      trails++;
      leaves[count++] = make_leaf_owned(buf, len);
  }
  free(str);
  // printf("hello %lu\n",count);
  
  mem_for_special mem;
  init_mem_f_s(&mem, 1); 
  rope_node *root = build_balanced_rope(leaves, count);
  // print_rope(root);
  printf("lines count %zu\n",lines(root->right));
  // rope_node *del;
  // delete_rope(root, 5-100000, &root, 50000, &mem, &del);
  free(leaves);
  rope_node *buff;
  printf("length of node: %zu\n",root->weight);
  start = clock();
  // insert_rope(root, 1, "HELLO", &root, &mem);
 // substr(root, 50000, 70000, &buff, &mem);
  // size_t y = find_start(root, 50);
  // printf("this is y: %zu\n",y);
  // const unsigned char *t = find_char_rope(root, 11);
  // printf("your character is : %s\n",t);
  // const unsigned char *e = find_char_rope(root, 12);
  // printf("your character is : %s\n",e);
  // const unsigned char *m = find_char_rope(root, 13);
  // printf("your character is : %s\n",m);
  // const unsigned char *p = find_char_rope(root, 14);
  // printf("your character is : %s\n",p);
  // const unsigned char *l = find_char_rope(root, 15);
  // printf("your character is : %s\n",l);
  // print_rope(i);
  // fast_substr(root, 1, 5, &uff, &mem);
  rope_slice(root, 0, 4,&mem,&buff);
  print_rope(buff);
  // rope_node *g = find_leaf(root, length(root) - 10);
  // printf("this is the size of the last leaf: %zu",strlen((char *)g->str));
  end = clock();
  printf("\n");
  cpu_time_used = ((double)(end-start)) / CLOCKS_PER_SEC;
  printf("time taken %f seconds\n",cpu_time_used);
  // free_ropes(uff,&mem);
  // free_internal(root, &mem);
  // free_ropes(del, &mem);
  free_ropes(root,&mem);
  free_mem(&mem);
  // char *t = "🥰";
  // printf("string length is %zu\n",strlen(t));
  // free(str);
  printf("size is %zu\n",sizeof(rope_node*));
  return 0;
}
