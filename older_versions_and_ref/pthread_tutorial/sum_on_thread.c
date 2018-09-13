#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// The sum computed by the background thread
// Global variable
long long sum = 0;

// Thread function to generate sum of 0 to N
void* sum_runner(void *arg)
{

    // Type casting void* to long long*
    long long *limit_ptr = (long long*) arg;

    // Dereference pointer to get the value
    long long limit = *limit_ptr;


    for(long long i = 0; i<= limit; i++) {
        sum += i;
    }

    // What to do with the answer???
    // sum is a Global variable, so other threads can access

    // My thread needs to finish
    // 0 means i'm not returning anything
    pthread_exit(0);

}

int main(int argc, char **argv)
{

    // Check user input. There has to be a number after a program name
    if (argc < 2) {
        printf("Usage: %s <num>\n", argv[0]);
        exit(-1);
    }

    // Since argv[0] is a Str
    // Convert a Char array (Str) to Long long
    long long limit = atoll(argv[1]);

    // Thread ID:
    pthread_t tid;

    // Create attributes:
    pthread_attr_t attr;

    pthread_attr_init(&attr);

    // Ready to create thread:
    pthread_create(&tid, &attr, sum_runner, &limit);

    // Do other stuff here...



    // Wait until thread is done its work
    // The pthread_join() unction shall suspend execution of the calling thread until the target thread terminates
    // Usage: int pthread_join(pthread_t thread, void **value_ptr);
    // leave value_ptr to NULL as we are going to return the value using Global variable
    pthread_join(tid, NULL);
    printf("Sum is %lld\n", sum);
}
