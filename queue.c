


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"



//To create a queue
queue* queue_init(int size){

    queue * q = (queue *)malloc(sizeof(queue));

    return q;
}


// To Enqueue an element
int queue_put(queue *q, struct element* x) {
    return 0;
}


// To Dequeue an element.
struct element* queue_get(queue *q) {
    struct element* element;
    
    return element;
}


//To check queue state
int queue_empty(queue *q){
    
    return 0;
}

int queue_full(queue *q){
    
    return 0;
}

//To destroy the queue and free the resources
int queue_destroy(queue *q){
    return 0;
}
