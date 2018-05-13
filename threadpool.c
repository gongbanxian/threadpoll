#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/syscall.h>
#include "threadpool.h"

static pool_t *thread_pool = NULL;

void *routine_thread(void *arg) 
{
	printf("starting thread %x\n", (uint32_t)pthread_self());

	while (1) {
		pthread_mutex_lock(&(thread_pool->queue_lock));

		while (thread_pool->cur_queue_size == 0 && !thread_pool->shutdown) {
			printf("thread %x is waiting ...\n", (uint32_t)pthread_self());
			pthread_cond_wait(&(thread_pool->queue_ready), &(thread_pool->queue_lock));
		}

		if (thread_pool->shutdown) {
			pthread_mutex_unlock(&(thread_pool->queue_lock));
			printf("thread %x will exit\n", (uint32_t)pthread_self());
			pthread_exit(NULL);
		}

		printf("thread %x is starting to work ...\n", (uint32_t)pthread_self());

		assert(thread_pool->cur_queue_size != 0);
		assert(thread_pool->queue_head != NULL);

		thread_pool->cur_queue_size--;
		worker_t *work = thread_pool->queue_head;
		thread_pool->queue_head = work->next;
		pthread_mutex_unlock(&(thread_pool->queue_lock));

		(*(work->process))(work->arg);
		free(work);
		work = NULL;
	}
}

void thread_pool_init(int32_t max_thread_num)
{
	int32_t ret;
	uint32_t i;

	thread_pool = (pool_t *)malloc(sizeof(pool_t));
	if (!thread_pool) {
		perror("malloc");
		return;
	}

	memset(thread_pool, 0, sizeof(pool_t));

	ret = pthread_mutex_init(&(thread_pool->queue_lock), NULL);
	if (ret != 0) {
		printf("init mutex failed!\n");
		return;
	}

	ret = pthread_cond_init(&(thread_pool->queue_ready), NULL);
	if (ret != 0) {
		printf("init cond failed!\n");
		return;
	}
	
	thread_pool->queue_head = NULL;
	thread_pool->max_thread_num = max_thread_num;
	thread_pool->cur_queue_size = 0;
	thread_pool->shutdown = 0;
	
	thread_pool->thread_id = (pthread_t *)malloc(sizeof(pthread_t) * max_thread_num);
	if (!thread_pool->thread_id) {
		perror("malloc");
		return;
	}

	for (i = 0; i < max_thread_num; i++) {
		ret = pthread_create(&(thread_pool->thread_id[i]), NULL, routine_thread, NULL);
		if (ret != 0) {
			printf("create thread failed!\n");
			return;
		}
	}	

	return;
}

int32_t thread_pool_add_work(void *(*process)(void *arg), void *arg)
{
	worker_t *work = (worker_t *)malloc(sizeof(worker_t));
	if (!work) {
		perror("malloc");
		return -1;
	}	
	
	work->process = process;
	work->arg = arg;
	work->next = NULL;

	pthread_mutex_lock(&(thread_pool->queue_lock));

	worker_t *wkqueue = thread_pool->queue_head;

	if (wkqueue != NULL) {
		while ( wkqueue->next != NULL)
			wkqueue = wkqueue->next;
		wkqueue->next = work;
	} else {
		thread_pool->queue_head = work;
	}

	assert(thread_pool->queue_head != NULL);

	thread_pool->cur_queue_size++;

	pthread_mutex_unlock(&(thread_pool->queue_lock));

	pthread_cond_signal(&(thread_pool->queue_ready));

	return 0;
}	

int32_t thread_pool_destroy(void) 
{
	worker_t *work = NULL;
	uint32_t i;

	if (thread_pool->shutdown)
		return -1;

	thread_pool->shutdown = 1;

	pthread_cond_broadcast(&(thread_pool->queue_ready));

	for (i = 0; i < thread_pool->max_thread_num; i++)
		pthread_join(thread_pool->thread_id[i], NULL);

	free(thread_pool->thread_id);

	while (thread_pool->queue_head != NULL) {
		work = thread_pool->queue_head;
		thread_pool->queue_head = work->next;
		free(work);
	}

	pthread_mutex_destroy(&(thread_pool->queue_lock));
	pthread_cond_destroy(&(thread_pool->queue_ready));

	free(thread_pool);
	thread_pool = NULL;

	return 0;
}
