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
  char *str = malloc(size+1);
  if(!str){
    perror("malloc failed");
    fclose(f);
  }
  long long fr = fread(str, 1, size, f);
  // printf("%llu\n",fr);
  str[size] = '\0';
  fclose(f);
  rope_node **leaves;
  leaves = NULL;
  long cap = 0,count = 0;
  clock_t start,end;
  double cpu_time_used;
  for(size_t i = 0;i < size; i+=(CHUNK_SIZE * 4) ){
      size_t len = (i + (CHUNK_SIZE *4) < size) ? (CHUNK_SIZE * 4) : size - i;
      if(count >= cap){
        cap = (cap == 0) ? 8 : cap * 2;
        leaves = realloc(leaves,sizeof(rope_node *) * cap);
      }
      char *buf = malloc((CHUNK_SIZE * 4) + 1);
      memcpy(buf, str + i,len);
      buf[len] = '\0';
      leaves[count++] = make_leaf_owned(buf, len);
  }
  free(str);
  // printf("hello %lu\n",count);
  
  mem_for_special mem;
  init_mem_f_s(&mem, 1); 
  rope_node *root = build_balanced_rope(leaves, count);
  rope_node *del;
  delete_rope(root, 5000000, &root, 50000, &mem, &del);
  free(leaves);
  // rope_node buff;
  start = clock();
  rope_node *i;
  i = NULL;
  insert_rope(root, 1, "HELLO", &root, &mem);
  // fast_substr(root, 1, 5, &uff, &mem);
  // print_rope(uff);
  end = clock();
  if (is_balanced(root) == 0){
      rebalance(root, &root, &mem);
  }else{
    i = root;
  }
  printf("\n");
  cpu_time_used = ((double)(end-start)) / CLOCKS_PER_SEC;
  printf("time taken %f seconds",cpu_time_used);
  // free_ropes(uff,&mem);
  // free_internal(root, &mem);
  free_ropes(del, &mem);
  free_ropes(root,&mem);
  free_mem(&mem);
  char *t = "🥰";
  // printf("string length is %zu\n",strlen(t));
  // free(str);
  return 0;
}
