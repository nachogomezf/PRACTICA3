
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <pthread.h>
#include "queue.h"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


#define NUM_CONSUMERS 1

/**
 * Entry point
 * @param argc
 * @param argv
 * @return
 */

void * _producer(queue * q){

}

void *_consumer(queue *q){

}

typedef struct produc_args{
    int num_op;
    struct element *elem;
}produc_args;

int main (int argc, const char * argv[] ) {
    int size_buf, num_prod, num_operations;
    int total = 0;
    struct element* elems = NULL;
    struct stat statbuf;
    pthread_t *th_prod = NULL;
    pthread_t th_cons;
    queue* circ_buf;
    if (argc != 4){ //Check correct number of inputs
        printf("Error. Structure of the command is: ./calculator <file_name> <num_producers> <buff_size>\n");
        return -1;
    }
    if (stat(argv[1], &statbuf) < 0 ){ //Check if the inputed file exists
        perror("File doesn't exist");
        return -1;
    }
    if (statbuf.st_size == 0){
        perror("File is empty");
        return -1;
    }
    num_prod = atoi(argv[2]);
    size_buf = atoi(argv[3]);
    circ_buf = queue_init(size_buf); //Initialize buffer
    th_prod = (pthread_t*) malloc(num_prod * sizeof(pthread_t));
    if (th_prod == NULL){ 
        perror("Error allocating producer threads");
        return -1;
    }
    close(STDIN_FILENO); //Redirect stdin to use scanf
    open(argv[1], O_RDONLY);
    scanf("%d", &num_operations);
    elems = (struct element*) malloc(num_operations * sizeof(struct element));
    if (elems == NULL){ 
        perror("Error allocating elems");
        close(STDIN_FILENO);
        free(th_prod);
        return -1;
    }
    //Retrieve elements
    for (int i = 0; i < num_operations; i++){
        scanf("%*d %d %d", &elems[i].type, &elems[i].time);
    } //MIRAR SI num_operations > file_lines
    produc_args arg;
    //Create threads
    for (int i = 0; i < num_prod; i++){
        arg.num_op = 10;
        memcpy(arg.elem, elems[i*10-1:i*11-1], 10*sizeof(struct element));
        pthread_create(&th_prod[i], NULL, _producer, &arg); /*CHECK INPUTS*/
    }
    pthread_create(&th_cons, NULL, _consumer, NULL);
    for (int i = 0; i < num_prod; i++){
        pthread_join(&th_prod[i], NULL); //Join
    }
    pthread_join(&th_cons, &total); //Retrieve result
    printf("Total: %i €.\n", total);
    free(elems);
    free(th_prod);
    return 0;
}
