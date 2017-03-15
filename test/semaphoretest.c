#include <sys/time.h>
#include <stdio.h>
#include "mythread.h" 
#include <errno.h>
//#include "queue.h"

//pthread_mutex_t produce_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t consume_mutex = PTHREAD_MUTEX_INITIALIZER;

MySemaphore consume_semaphore, produce_semaphore;

int b[10];  /* buffer size = 1; */
int n; 


void add_buffer(int i){
  b[i] = i;
  printf("\nAdded %d\n",b[i])	;
}
int get_buffer(int i){
  
	printf("\nConsumed %d\n",b[i]);	
	return b[i] ;
}
 
void producer(void *dummy)
{
	int i = 0;
	printf("I'm a producer\n");
		for (i=0;i<10;i++){
		  MySemaphoreWait(produce_semaphore);
		  add_buffer(i);
			//MyThreadExit();
		  MySemaphoreSignal(consume_semaphore);
		  //i = i + 1;
		}
	

	MyThreadExit();
}
void consumer(void *dummy)
{
	int i,v;
	printf("I'm a consumer\n");
	 for (i=0;i<10;i++){
	   MySemaphoreWait(consume_semaphore);
	   v = get_buffer(i);
	   MySemaphoreSignal(produce_semaphore);
	   printf("got %d \n ",v);
	}
	MyThreadExit();
}

void temp(void *dummy)
{

	MyThread t_consumer;	
	t_consumer = MyThreadCreate(consumer,dummy);
	MyThreadCreate(producer,dummy);
	MyThreadJoin(t_consumer);
        MyThreadExit();

}

 
main()  
{
	int p,c;	
	n =10;	
	produce_semaphore = MySemaphoreInit(2); 
	consume_semaphore = MySemaphoreInit(1);
 
	MyThreadInit(temp,(void *)&n);	
	if(MySemaphoreDestroy(produce_semaphore) == -1)
		printf("\n Error destroying producer semaphore");
	if(MySemaphoreDestroy(consume_semaphore))
		printf("\nError destroying consumer semaphore");
        printf("end main\n");
}
