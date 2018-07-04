#ifndef QUEUE_H
# define QUEUE_H


#include "btree.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


//#############################################################################
//                               STRUCTS USED
//#############################################################################

typedef struct element element;
typedef struct btNode btNode;
typedef struct bTree bTree;

typedef struct queue{
    int size;
    int front;
    int rear;
    int itemCount;
    btNode *list;
}queue;

//#############################################################################
//                               METHODS
//#############################################################################

queue *createQueue(int size);
btNode peek(queue *q);
bool isEmpty(queue *q);
bool isFull(queue *q);
int size(queue *q);
void insert(queue *q ,btNode data);
btNode removeData(queue *q);

#endif
