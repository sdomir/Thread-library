#ifndef THREAD_STRUCTURE_H
#define	THREAD_STRUCTURE_H
#endif

#define STACK_SIZE 1024*8;

#include "mythread.h"
#include <ucontext.h>
typedef enum {true,false} bool;
//typedef struct child_list child;
ucontext_t mainthread,handler,grand_parent;
typedef struct th
{
    int count_children;                                 
    ucontext_t thread_context; 
    struct th *parent;
    struct th *whojoined;
    int join_state;            //join_state = 1(joinall) 2(join) 0(default))
    bool isblocked;
}mythread;
mythread *current= NULL;

typedef struct queue
{
    mythread *enqueue;
    struct queue *nxt;
}q;
q *rhead = NULL;
q *rrear = NULL;
q *bhead = NULL;
q *brear = NULL;
/*typedef struct blocked_queue
{
    mythread *benqueue;
    struct blocked_queue *bnxt;
}block;
block *bhead = NULL;
block *brear = NULL;*/

typedef struct sema
{
    int initialValue;
    q *sem_head;
    q *sem_rear;
}semaphore;
semaphore *s = NULL;
