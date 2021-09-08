#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

Queue *init_queue() {
    Queue *queue = NULL;
    queue = (Queue*)malloc(sizeof(Queue));
    if (queue == NULL) {
        return NULL;
    }

    queue->size = 0;
    queue->front = NULL;
    queue->rear = NULL;
    return queue;
}

Node* new_node(int fd)
{
    Node *temp = (Node*)malloc(sizeof(Node));
    if (temp == NULL) {
        return NULL;
    }

    temp->fd = fd;
    temp->next = NULL;
    return temp;
}

void enqueue(Queue *queue, int fd) {

    Node *temp = NULL;

    if (queue == NULL) {
        return;
    }

    temp = new_node(fd);
    if (temp == NULL) {
        destroy_queue(queue);
        queue = NULL;
        return;
    }

    queue->size += 1;

    if (queue->rear == NULL) {
        queue->front = queue->rear = temp;
        return;
    }

    queue->rear->next = temp;
    queue->rear = temp;
}

int dequeue(Queue *queue) {

    int ret;
    Node *temp = NULL;
    if (queue == NULL || queue->front == NULL) {
        return -1;
    }

    temp = queue->front;

    queue->front = queue->front->next;
    queue->size -= 1;
    
    if (queue->front == NULL)
        queue->rear = NULL;

    ret = temp->fd;
    free(temp);

    return ret;
}

int queue_size(Queue *queue) {
    if (queue == NULL) {
        return 0;
    }

    return queue->size;
}

void destroy_queue(Queue *queue) {

    Node *temp;
    if (queue == NULL) {
        return;
    }

    if (queue->front == NULL) {
        free(queue);
        queue = NULL;
        return;
    }

    while (queue->front) {
        temp = queue->front;
        queue->front = queue->front->next;
        free(temp);
    }

    free(queue);
    queue = NULL;
}