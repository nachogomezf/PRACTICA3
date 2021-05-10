#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"

int numelems = 0;

//To create a queue
queue* queue_init(int size){
    queue * q = (queue *)malloc(sizeof(queue)); //Reserve memory for the queue
    q->_size = size; //Set size
    q->_elems = (struct element*) malloc(sizeof(struct element) * size); //Allocate memory for elements
    q->_inpos = 0; //The attribute inpos will be the index of the element to be enqueued
    q->_outpos = 0; //outpos will be the index of the element to be dequeued
    q->_numelems = 0; //Free queue
    return q;
}


// To Enqueue an element
int queue_put(queue *q, struct element* x) {
    if (!queue_full(q)){
        q->_elems[q->_inpos] = *x; //Insert the element in the queue to the input index
        q->_inpos = (q->_inpos + 1) % q->_size; //Update the index value (if we are in the end we must go to first position)  
        q->_numelems++; //Increment the queue size
        return 0;
    } else {
        printf("Cannot queue_put: queue is full\n");
        return -1;
    }
}


// To Dequeue an element.
struct element* queue_get(queue *q) {
    struct element* element = NULL; //Initialize an empty struct* element to save result
    if (!queue_empty(q)){
        element = &(q->_elems[q->_outpos]); //Get the address of the element to be dequeued
        q->_outpos = (q->_outpos + 1) % q->_size; //Update outpos index
        q->_numelems--; //Decrease queue size
        return element;
    } else {
        printf("Cannot queue_get: queue is empty\n");
        return NULL;
    }
}


//To check queue state
int queue_empty(queue *q){
    return q->_numelems == 0;
}

int queue_full(queue *q){
    return q->_numelems == q->_size;
}

//To destroy the queue and free the resources
int queue_destroy(queue *q){
    free(q->_elems);
    free(q);
    return 0;
}
