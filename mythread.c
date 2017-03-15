#include <stdio.h>
#include <stdlib.h>
#include "mythread.h"
#include "thread_structure.h"
#include <ucontext.h>

void insert(mythread *thread, q **head, q **rear)
{
    
    q *newnode = (q *)calloc(1, sizeof(struct queue));
    newnode->enqueue = thread;
    newnode->nxt = NULL;
    
    if(*head == NULL){
        //printf("%s\n","inside insert");
        *head = *rear = newnode;}
    else
    {
        
        (*rear)->nxt = newnode;
        *rear = newnode;
    }
    
}

mythread* dequeue_ready(q **rhead, q **rrear)
{
    mythread *remove;
    q *temp = *rhead;
    if(*rhead == NULL)
    
      return NULL;
    else
    {
    remove = (*rhead)->enqueue;
    if(*rhead == *rrear){
        *rhead = NULL;
        *rrear = NULL;
    }
    else
        *rhead = (*rhead)->nxt;
    free(temp);
    return remove;
    }
}

mythread* dequeue_block(mythread *child_parent,q **bhead)
{
    mythread *remove;
    q *temp = *bhead;
    if(*bhead == NULL)
      return NULL;
    else if(temp->enqueue == child_parent)
    {
        remove = temp->enqueue;
        *bhead = (*bhead)->nxt;
        free(temp);
    }    
    else
    {
        while(temp->nxt->enqueue != child_parent)
            temp=temp->nxt;
        q *temp2 = temp->nxt->nxt;
        remove = temp->nxt->enqueue;
        free(temp->nxt);
        temp->nxt = temp2;
    }
    return remove;
}


void ThreadHandler(void *args)
{
    int flag=0;
    getcontext(&handler);
    //printf("%s\n","Inside handler");
    current = dequeue_ready(&rhead,&rrear);
    if(current == NULL)
        setcontext(&grand_parent);
    else
        setcontext(&current->thread_context);
}

void MyThreadYield(void)
{
    insert(current,&rhead,&rrear);
    swapcontext(&current->thread_context,&handler);
}

int last_child()
{
    int count = current->parent->count_children;
    count--;
    if(count == 0)
        return 1;
    
    return 0;            
}

void allchild(mythread *current,q **ready_head,q **block_head)
{
    q *temp = *ready_head;
    q *temp2 = *block_head;
    while(temp != NULL)
    {
        if(temp->enqueue->parent == current)
            temp->enqueue->parent == NULL;
        temp = temp->nxt;
    }
    while(temp2 != NULL)
    {
        if(temp2->enqueue->parent == current)
            temp2->enqueue->parent == NULL;
        temp2 = temp2->nxt;
    }
}

void MyThreadExit(void)                         
{
    mythread *return_thread;
    if(current->parent != NULL && current->parent->isblocked == true && current->parent->join_state == 2)
    {
        
        if(current->parent->whojoined == current)
        {
        return_thread = dequeue_block(current->parent,&bhead);
        insert(return_thread,&rhead,&rrear);
        }
        current->parent->count_children--;
    }
    else if(current->parent != NULL && current->parent->isblocked == true && current->parent->join_state == 1)
    {
        
        if(last_child())
        {
        return_thread = dequeue_block(current->parent,&bhead);
        insert(return_thread,&rhead,&rrear);
        }
        current->parent->count_children--;
    }
    else if(current->parent != NULL)
        current->parent->count_children--;
    
    if(current->count_children > 0)
    allchild(current,&rhead,&bhead);
        //free(current);                                   //Might need to remove
    //printf("%s\n","start");
    setcontext(&handler);    
}

int existchild(mythread *son, q **ready_head, q **block_head)
{
    q *temp = *ready_head;
    q *temp2 = *block_head;
    while(temp != NULL)
    {
        if(temp->enqueue == son)
            return 1;
        temp = temp->nxt;
    }
    while(temp2 != NULL)
    {
        if(temp2->enqueue == son)
            return 1;
        temp2 = temp2->nxt;
    }
    
    return 0;    
}

int MyThreadJoin(MyThread thread)
{
    volatile int flag = 0;
    
    mythread *son = (mythread*)thread;
    
    if(!existchild(son,&rhead,&bhead))                             
    {                                                       
        //printf("Child does not exists.\n");
        return 0;}
    
    else if(son->parent != current)
    {
        //printf("Error:Join failed\n");
        return -1;
    }    
    
    else
    {
        
        current->isblocked = true;
        current->join_state = 2;
        current->whojoined = son;
        
        insert(current,&bhead,&brear);
	swapcontext(&current->thread_context,&handler);
    }
}

void MyThreadJoinAll(void)
{
    volatile int flag = 0;
    //printf("Inside join all\n");
    if(current->count_children > 0)
    {
        current->isblocked = true;
        current->join_state = 1;
        getcontext(&current->thread_context);
        if(flag == 0)
        {
            flag = 1;
            insert(current,&bhead,&brear);
            setcontext(&handler);
        }
    }
    
}

MyThread MyThreadCreate (void(*start_funct)(void *), void *args)  
{
    mythread *thread = (mythread*)calloc(1, sizeof(struct th));
    getcontext(&thread->thread_context);
    (thread->thread_context).uc_link          = NULL;
    (thread->thread_context).uc_stack.ss_sp   = calloc(1024, 8);
    (thread->thread_context).uc_stack.ss_size = STACK_SIZE;

    makecontext(&(thread->thread_context), (void (*)(void)) start_funct, 1, args);
    thread->isblocked = false;
    thread->parent = current;
    thread->join_state = 0;
    thread->count_children = 0;
    thread->whojoined = NULL;
    current->count_children++;
    insert(thread,&rhead,&rrear);
    MyThread t = (MyThread)thread;
    return t;
}

void MyThreadInit(void(*start_funct)(void *), void *args)
{
    volatile int flag = 0;
    
    mythread *initial_thread = (mythread*)calloc(1, sizeof(struct th));
    getcontext(&grand_parent);
    if (flag == 0)
    {
       flag = 1;
        getcontext(&handler);
        handler.uc_link          = &grand_parent;
        handler.uc_stack.ss_sp   = calloc(1024, 8);                              
        handler.uc_stack.ss_size = STACK_SIZE;                                   
        makecontext(&handler, (void (*)(void)) ThreadHandler, 1, args);
        getcontext(&mainthread);
        mainthread.uc_link          = NULL;
        mainthread.uc_stack.ss_sp   = calloc(1024, 8);             
        mainthread.uc_stack.ss_size = STACK_SIZE;
        makecontext(&mainthread, (void (*)(void)) start_funct, 1, args);
        initial_thread->thread_context = mainthread;
        initial_thread->parent = NULL;
        initial_thread->isblocked = false;
        initial_thread->count_children = 0;
        initial_thread->join_state = 0;
        initial_thread->whojoined = NULL;
        
           
        insert(initial_thread,&rhead,&rrear);
        
        setcontext(&handler);
    }
    
}

MySemaphore MySemaphoreInit(int initialValue)
{
    if(initialValue < 0)
        return NULL;
    else
    {
        semaphore *newsem = (semaphore *)calloc(1, sizeof(struct sema));
        newsem->initialValue = initialValue;
        newsem->sem_head = NULL;
        newsem->sem_rear = NULL;
        MySemaphore sem = (MySemaphore)newsem;
        return sem;
    }
}

void MySemaphoreWait(MySemaphore sem)
{
    semaphore *sema = (semaphore *)sem;
    sema->initialValue--;
    if(sema->initialValue < 0)
    {
        insert(current,&sema->sem_head,&sema->sem_rear);
        swapcontext(&current->thread_context,&handler);
    }
}

void MySemaphoreSignal(MySemaphore sem)
{
    mythread *dequeue;
    semaphore *sema = (semaphore *)sem;
    sema->initialValue++;
    if(sema->initialValue <= 0)
    {
        dequeue = dequeue_ready(&sema->sem_head,&sema->sem_rear);
        
        insert(dequeue,&rhead,&rrear);
    }
}

int MySemaphoreDestroy(MySemaphore sem)
{
    semaphore *sema = (semaphore*)sem;
    if(sema->sem_head == NULL)
    {
        free(sema);
        return 0;
    }
    else
        return -1;
}






