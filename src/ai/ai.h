#ifndef __AI__
#define __AI__

#include <stdint.h>
#include <unistd.h>

void solve(char const *path);

void enqueue(gate_t* data, int* enqueued);
void dequeue(data_t* data, int* dequeued);

typedef struct queuenode_t 
typedef queuenode_t *queuenode_ptr;

node_t {
    gate_t* data;
    queuenode_ptr *next;
}


queuenode
#endif
