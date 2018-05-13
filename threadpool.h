#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__
#include <stdint.h>

typedef struct worker {
	void *(*process)(void *arg);
	void *arg;
	struct worker *next;
} worker_t;

typedef struct {
	pthread_mutex_t queue_lock;
	pthread_cond_t queue_ready;
    worker_t *queue_head;
	int32_t shutdown;
	pthread_t *thread_id;
	int32_t max_thread_num;
	int32_t cur_queue_size;
} pool_t;

void thread_pool_init(int32_t max_thread_num);
int32_t thread_pool_add_work(void *(*process)(void *arg), void *arg);
int32_t thread_pool_destroy(void);

#endif
