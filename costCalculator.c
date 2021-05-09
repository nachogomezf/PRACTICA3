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

pthread_mutex_t circ_buf_mutex;

pthread_cond_t circ_buf_not_full;
pthread_cond_t circ_buf_not_empty;

typedef struct produc_args{
    struct element *elems;
    int first_elem, last_elem;
    queue *circ_buf;
}produc_args;
typedef struct consum_args{
    int total;
    queue *circ_buf;
    int num_op;
}consum_args;

/**
 * Entry point
 * @param argc
 * @param argv
 * @return
 */

void * _producer(void * arg){
    printf("HELLO FROM PROD THREAD\n");
    produc_args *prod_arg = (produc_args*) arg;
    struct element* enq_elem;
    for(int i = prod_arg->first_elem; i <= prod_arg->last_elem; i++){
        pthread_mutex_lock(&circ_buf_mutex); 
        while(queue_full(prod_arg->circ_buf))
            pthread_cond_wait(&circ_buf_not_full, &circ_buf_mutex);
        enq_elem = &prod_arg->elems[i];
        printf("Enqueing:%d\t%d\n", enq_elem->type, enq_elem->time);
        queue_put(prod_arg->circ_buf, enq_elem);
        pthread_cond_signal(&circ_buf_not_empty);
        pthread_mutex_unlock(&circ_buf_mutex);
    }
    pthread_exit(NULL);
}

void *_consumer(void *arg){
    printf("HELLO FROM CONS THREAD\n");
    consum_args *cons_arg = (consum_args*) arg;
    struct element *elem;
    for(int i = 0; i < cons_arg->num_op; i++){
        pthread_mutex_lock(&circ_buf_mutex);
        printf("Dequeing %d\n", i);
        while(queue_empty(cons_arg->circ_buf))
            pthread_cond_wait(&circ_buf_not_empty, &circ_buf_mutex);
        elem = queue_get(cons_arg->circ_buf);   
        cons_arg->total += elem->type * elem->time;
        pthread_cond_signal(&circ_buf_not_full);
        pthread_mutex_unlock(&circ_buf_mutex);
    }
    pthread_exit(NULL);
}/*HAY QUE MODIFICAR*/

int main (int argc, const char * argv[] ) {
    int size_buf, num_prod;
    int num_operations, num_op_prod;
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
    if (statbuf.st_size == 0){ //Check if the file is empty or not
        perror("File is empty");
        return -1;
    }

    num_prod = atoi(argv[2]);
    size_buf = atoi(argv[3]);

    circ_buf = queue_init(size_buf); //Initialize buffer

    //Create array of producer threads
    th_prod = (pthread_t*) malloc(num_prod * sizeof(pthread_t));
    if (th_prod == NULL){ 
        perror("Error allocating producer threads");
        queue_destroy(circ_buf);
        return -1;
    }

    close(STDIN_FILENO); //Redirect stdin to use scanf
    if (open(argv[1], O_RDONLY) < 0){
        perror("Error opening the file");
        queue_destroy(circ_buf);
        free(th_prod);
        return -1;
    }

    scanf("%d", &num_operations); //Read number of operations

    //Create structure for elems
    elems = (struct element*) malloc(num_operations * sizeof(struct element));
    if (elems == NULL){ 
        perror("Error allocating elems");
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
    if (i < num_operations){
        fprintf(stderr, "NOT ENOUGH LINES READ\n");
        close(STDIN_FILENO);
        free(th_prod);
        queue_destroy(circ_buf);
        free(elems);
        return -1;
    }

    //Arguments for threads
    produc_args *prod_arg;
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

    consum_args *cons_arg = NULL;
    cons_arg = (consum_args*) malloc(sizeof(consum_args));
    cons_arg->circ_buf = circ_buf;
    cons_arg->total = total;
    cons_arg->num_op = num_operations;

    //Initialize mutex and condition variable
    pthread_mutex_init(&circ_buf_mutex, NULL);
    pthread_cond_init(&circ_buf_not_full, NULL);
    pthread_cond_init(&circ_buf_not_empty, NULL);

    //Create threads
    pthread_create(&th_cons, NULL, _consumer, (void *) &cons_arg);

    for (int i = 0; i < num_prod; i++){
        prod_arg[i].elems = elems;
        prod_arg[i].circ_buf = circ_buf;
        prod_arg[i].first_elem = i*num_op_prod;
        if (i != num_prod - 1){
            prod_arg[i].last_elem = (i+1)*num_op_prod - 1;
        } else {
            prod_arg[i].last_elem = num_operations - 1;
        }
        pthread_create(&th_prod[i], NULL, _producer, &prod_arg[i]); //CHECK INPUTS
    }

    //Join threads
    for (int i = 0; i < num_prod; i++){
        pthread_join(th_prod[i], NULL);
    }
    pthread_join(th_cons, NULL);

    total = cons_arg->total;
    printf("Total: %i €.\n", total);

    free(th_prod);
    queue_destroy(circ_buf);
    free(elems);
    free(prod_arg);
    pthread_cond_destroy(&circ_buf_not_full);
    pthread_cond_destroy(&circ_buf_not_empty);
    pthread_mutex_destroy(&circ_buf_mutex);
    free(cons_arg);
    close(STDIN_FILENO);
    return 0;
}
