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

//Mutex and cond variables
pthread_mutex_t circ_buf_mutex;

pthread_cond_t circ_buf_not_full;
pthread_cond_t circ_buf_not_empty;

//Global vairables
struct element *elems;
int total = 0;
queue *circ_buf;
int num_operations;

//Our definitions
typedef struct produc_args{
    int first_elem, last_elem; //Store first and last elements to read
}produc_args;

/**
 * Entry point
 * @param argc
 * @param argv
 * @return
 */

void * _producer(void * arg){
    produc_args *prod_arg = (produc_args*) arg;
    struct element enq_elem;
    for(int i = prod_arg->first_elem; i <= prod_arg->last_elem; i++){
        enq_elem = elems[i]; //Never accessing the same element so no need of mutex. 
        pthread_mutex_lock(&circ_buf_mutex); //Lock mutex to write into circular buffer
        while(queue_full(circ_buf)) //Wait for buffer to have space
            pthread_cond_wait(&circ_buf_not_full, &circ_buf_mutex);
        queue_put(circ_buf, &enq_elem); //Write into buffer
        pthread_cond_signal(&circ_buf_not_empty); //Signal not empty
        pthread_mutex_unlock(&circ_buf_mutex); //Unlock mutex
    }
    pthread_exit(NULL);
    return NULL;
}

void *_consumer(void *arg){
    struct element* elem;
    int cost, num_deq = 0;
    while(num_deq < num_operations){
        pthread_mutex_lock(&circ_buf_mutex); //Lock mutex to read from circular buffer
        while(queue_empty(circ_buf)) //Wait for buffer to be nonempty
            pthread_cond_wait(&circ_buf_not_empty, &circ_buf_mutex);
        elem = queue_get(circ_buf); //Read from buffer
        if (elem->type == 1){ //Check type to sum
            cost = 1;
        }else if (elem->type == 2){
            cost = 3;
        }else if (elem->type == 3){
            cost = 10;
        }
        total += cost * elem->time; //Perform operation
        pthread_cond_signal(&circ_buf_not_full); //Signal buffer is now not full
        pthread_mutex_unlock(&circ_buf_mutex); //Unlock mutex
        num_deq++;
    }
    pthread_exit(NULL);
    return NULL;
}

int main (int argc, const char * argv[]) {
    int size_buf, num_prod, num_op_prod;
    struct stat statbuf;
    pthread_t *th_prod;
    pthread_t th_cons;
    produc_args *prod_arg;

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

    if (num_prod <= 0){ //Check if valid number of producers
        fprintf(stderr, "Number of producers must be greater than 0\n");
        return -1;
    }
    if (size_buf <= 0){ //Check if valid size of buffer
        fprintf(stderr, "Size of the buffer must be greater than 0\n");
        return -1;
    }

    circ_buf = queue_init(size_buf); //Initialize buffer

    //Allocate array of producer threads
    th_prod = (pthread_t*) malloc(num_prod * sizeof(pthread_t));
    if (th_prod == NULL){ 
        perror("Error allocating producer threads.");
        queue_destroy(circ_buf);
        return -1;
    }

    close(STDIN_FILENO); //Redirect stdin to use scanf
    if (open(argv[1], O_RDONLY) < 0){
        perror("Error opening the file.");
        queue_destroy(circ_buf);
        free(th_prod);
        return -1;
    }

    scanf("%d", &num_operations); //Read number of operations

    //Allocate array for elems
    elems = (struct element*) malloc(num_operations * sizeof(struct element));
    if (elems == NULL){ 
        perror("Error allocating elems.");
        close(STDIN_FILENO);
        queue_destroy(circ_buf);
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

    if (i < num_operations){ //Check if valid number of read lines
        fprintf(stderr, "NOT ENOUGH LINES READ\n");
        close(STDIN_FILENO);
        free(th_prod);
        queue_destroy(circ_buf);
        free(elems);
        return -1;
    }

    //Allocate arguments for threads
    prod_arg = (produc_args*)malloc(num_prod * sizeof(produc_args));
    if (prod_arg == NULL){ 
        perror("Error allocating elems");
        close(STDIN_FILENO);
        queue_destroy(circ_buf);
        free(th_prod);
        free(elems);
        return -1;
    }
    num_op_prod = num_operations / num_prod;


    //Initialize mutex and condition variable
    pthread_mutex_init(&circ_buf_mutex, NULL);
    pthread_cond_init(&circ_buf_not_full, NULL);
    pthread_cond_init(&circ_buf_not_empty, NULL);

    //Create threads
    for (int i = 0; i < num_prod; i++){
        prod_arg[i].first_elem = i*num_op_prod; //Set arguments for threads
        if (i != num_prod - 1){
            prod_arg[i].last_elem = (i+1)*num_op_prod - 1;
        } else {
            prod_arg[i].last_elem = num_operations - 1;
        }
        pthread_create(&th_prod[i], NULL, _producer, &prod_arg[i]); //Create producer thread with argument
    }
    pthread_create(&th_cons, NULL, _consumer, NULL); //Create producer thread

    //Join threads
    for (int i = 0; i < num_prod; i++){
        pthread_join(th_prod[i], NULL);
    }
    pthread_join(th_cons, NULL);

    printf("Total: %i €.\n", total);

    //Free memory
    free(th_prod);
    queue_destroy(circ_buf);
    free(elems);
    free(prod_arg);
    pthread_cond_destroy(&circ_buf_not_full);
    pthread_cond_destroy(&circ_buf_not_empty);
    pthread_mutex_destroy(&circ_buf_mutex);
    close(STDIN_FILENO);
    return 0;
}
