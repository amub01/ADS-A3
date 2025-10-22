#ifndef __AI__
#define __AI__

#include <stdint.h>
#include <unistd.h>
#include <gate.h>
typedef struct queuenode_t {
    gate_t* data;
    struct queuenode_t *next;
} queuenode_t, *queuenode_ptr;

typedef struct queue_t {
    queuenode_ptr head;
    queuenode_ptr tail;
    int queuelen;
} queue_t, *queue_ptr;

typedef struct radixnode_t {
    struct radixnode_t* children[36]; // assuming only alphanumeric keys to save space
    char* key;
    int keylen;
    int is_leaf;
} radixnode_t, *radixnode_ptr;

typedef struct radixtree_t {
    radixnode_ptr root;
    int size;
} radixtree_t, *radixtree_ptr;

void solve(char const *path);
void enqueue(gate_t* data, queue_ptr queue);
gate_t *dequeue(queue_ptr queue);
void free_queue(queue_ptr queue, gate_t *data);
int state_change(gate_t * old_state, gate_t *new_state);
void append_sol(gate_t *node, int piece, int move);
#endif
