#ifndef __THREADPOOL_H_
#define __THREADPOOL_H_

#include <pthread.h>

typedef struct thread_pool thread_pool;

thread_pool *pool_init(int max_threads);

int pool_get_max_threads(thread_pool *);

void pool_add_task(thread_pool *, void *(*work_routine)(void *), void *arg);

void pool_wait(thread_pool *);

void pool_destroy(thread_pool *);

#endif