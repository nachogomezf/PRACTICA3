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

void * _producer(void * q){
    printf("Hi from producer thread!");
    pthread_exit(NULL);
}

void *_consumer(void *tot){
    int *tot = (int*) tot;
    //*tot = 10;
    printf("Hi from consumer thread!");
    pthread_exit(NULL);
}/*HAY QUE MODIFICAR*/

typedef struct produc_args{
    int num_op;
    struct element *elem;
}produc_args;

int main (int argc, const char * argv[] ) {
    int size_buf, num_prod;
    int num_operations;
    int *total;
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
    if (statbuf.st_size == 0){ //Check if the file is empty or not
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
    int i = 0;
    while(i < num_operations){
        if (scanf("%*d %d %d", &elems[i].type, &elems[i].time) == EOF ){
            break;
        }
        i++;
    }
    if (i < num_operations){
        fprintf(stderr, "NOT ENOUGH LINES READ\n");
        close(STDIN_FILENO);
        free(th_prod);
        free(elems);
        return -1;
    }
    produc_args arg;
    //Create threads
    for (int i = 0; i < num_prod; i++){
        pthread_create(&th_prod[i], NULL, _producer, &arg); //CHECK INPUTS
    }
    pthread_create(&th_cons, NULL, _consumer, NULL);
    //Join threads
    for (int i = 0; i < num_prod; i++){
        pthread_join(th_prod[i], NULL); //Join
    }
    pthread_join(th_cons, &total); //Retrieve result
    free(th_prod);

    printf("Total: %i €.\n", *total);
    queue_destroy(circ_buf);
    free(elems);
    return 0;
}
