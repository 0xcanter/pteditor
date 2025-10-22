// #include <cstdlib>c
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include "lib/rope.h"


int main(){
  FILE *f = fopen("test2.txt", "r");
    if(!f){
        perror("failed to open file!");
        return 1;
    }
    char buffer[CHUNK_SIZE * 4 + 1];
    size_t bytes_read;


    rope_node **leaves;
    leaves = NULL;
    int cap = 0,count = 0;
    clock_t start,end;
    double cpu_time_used;
    while ((bytes_read = fread(buffer, 1, CHUNK_SIZE * 4, f)) > 0) {
        buffer[bytes_read] = '\0';
        if(count >= cap){
            cap = (cap == 0) ? 8 : cap * 2;
            leaves = realloc(leaves, cap * sizeof(*leaves));
        }
        leaves[count++] = make_leaf(buffer);
    }
    mem_for_special mem;
    init_mem_f_s(&mem, 1);
    
    rope_node *root = build_balanced_rope(leaves, count);
    start = clock();
    rope_node *new;
    free(leaves);
    delete_rope(root, 169394250, &root, 3000, &mem);
    // insert_rope(root,169394250 , "heyllo", &root, &mem);
    end = clock();
    cpu_time_used = ((double)(end-start)) / CLOCKS_PER_SEC;
    printf("time taken: %f seconds\n",cpu_time_used);
    int depth = count_depth(root);
    printf("%lu weight\n",root->weight);
    free_ropes(root,&mem);
    // add_to_mem(&mem, root);
    free_mem(&mem);
    fclose(f);
    char *tt = "";
    printf("%zu\n",strlen(tt)); 
    return 0;
}
