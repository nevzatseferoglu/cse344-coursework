#ifndef _QUEUE
#define _QUEUE

typedef struct Node {
    int fd;
    struct Node *next;
} Node;

typedef struct Queue {
    int size;
    Node *front;
    Node *rear;
} Queue;

Queue *init_queue();
void enqueue(Queue *queue, int fd);
int dequeue(Queue *queue);
int queue_size(Queue *queue);
void destroy_queue(Queue *queue);


#endif /* _QUEUE */