/*
condition variables using pthread_cond_t
SOURCE:
http://faculty.ycp.edu/~dhovemey/spring2011/cs365/lecture/lecture16.html
*/

/* Compile like this:
gcc -D_REENTRANT -Werror -Wall pthread_counter.c -o pthread_counter -lpthread
*/

#include <pthread.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct {
	int count;
	pthread_mutex_t lock;
	pthread_cond_t cond;
} Counter;

void counter_init(Counter *c);
int counter_get(Counter *c);
void counter_incr(Counter *c);
void counter_wait_threshold(Counter *c, int threshold);

void *worker(void *arg)
{
	Counter *c = arg;

	int i;
	for (i = 0; i < 10; i++) {
		printf("tick\n");
		counter_incr(c);
		sleep(1);
	}

	return NULL;
}

void *worker2(void *arg)
{
	Counter *c = arg;
	counter_wait_threshold(c, 6);
	printf("counter has reached 6!\n");
	return NULL;
}

int main(void)
{
	pthread_t thread1, thread2;

	Counter *c = malloc(sizeof(Counter));
	counter_init(c);

	pthread_create(&thread1, NULL, &worker, c);
	pthread_create(&thread2, NULL, &worker2, c);

	// wait for the threads to complete
	pthread_join(thread2, NULL);
	printf("thread 2 is finished\n");
	pthread_join(thread1, NULL);
	printf("thread 1 is finished\n");

	printf("final count is %d\n", c->count);

	return 0;
}

void counter_init(Counter *c)
{
	c->count = 0;
	pthread_mutex_init(&c->lock, NULL);
	pthread_cond_init(&c->cond, NULL);
}

int counter_get(Counter *c)
{
	pthread_mutex_lock(&c->lock);
	int val = c->count;
	pthread_mutex_unlock(&c->lock);
	return val;
}

void counter_incr(Counter *c)
{
	pthread_mutex_lock(&c->lock);
	int val = c->count;
	val = val + 1;
	c->count = val;
	pthread_cond_broadcast(&c->cond);
	pthread_mutex_unlock(&c->lock);
}

void counter_wait_threshold(Counter *c, int threshold)
{
	pthread_mutex_lock(&c->lock);

	while (c->count < threshold) {
		pthread_cond_wait(&c->cond, &c->lock);
	}
	// now c->count >= threshold

	pthread_mutex_unlock(&c->lock);
}

