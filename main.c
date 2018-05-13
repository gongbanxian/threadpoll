#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "threadpool.h"

void *task_process(void *arg)
{
	printf("thread id is %x, working on task %d\n", (uint32_t)pthread_self(), *((int *)arg));	

	sleep(1);

	return NULL;
}

int main(int argc, char *argv[])
{
	int32_t workarg[10]; 
	uint32_t i;

	thread_pool_init(3);

	for (i = 0; i < 10; i++) {
		workarg[i] = i;
		thread_pool_add_work(task_process, (void *)&workarg[i]);
	}

	sleep(5);

	thread_pool_destroy();
	
	return 0;
}
