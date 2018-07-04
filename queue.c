#include "queue.h"


queue *createQueue(int size) {
    queue *q;
    if((q = malloc(sizeof(queue))) == NULL)
        return NULL;
    if((q->list = malloc(sizeof(btNode) * size)) == NULL)
        return NULL;
    q->size = size;
    q->front = 0;
    q->rear = -1;
    q->itemCount = 0;
    return q;
}

btNode peek(queue *q) {
    return q->list[q->front];
}

bool isEmpty(queue *q) {
    return q->itemCount == 0;
}

bool isFull(queue *q) {
    return q->itemCount == q->size;
}

int size(queue *q) {
    return q->itemCount;
}

void insert(queue *q ,btNode data) {

    if(!isFull(q)) {

        if(q->rear == q->size-1) {
            q->rear = -1;
        }

        q->list[++q->rear] = data;
        q->itemCount++;
    }
}

btNode removeData(queue *q) {
    btNode data = q->list[q->front++];

    if(q->front == q->size) {
        q->front = 0;
    }

    q->itemCount--;
    return data;
}
