#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>



// Thread function to generate sum of 0 to N
void* sum_runner(void *arg)
{

    // Type casting void* to long long*
    long long *limit_ptr = (long long*) arg;

    // Dereference pointer to get the value
    long long limit = *limit_ptr;

    free(arg);

    long long sum = 0;

    for(long long i = 0; i<= limit; i++) {
        sum += i;
    }

    // Pass back data in dynamically allocated memory
    long long *answer = malloc(sizeof(*answer));
    *answer = sum;

    // My thread needs to finish
    pthread_exit(answer);

}



int main(int argc, char **argv)
{

    // Check user input. There has to be a number after a program name
    if (argc < 2) {
        printf("Usage: %s <num>\n", argv[0]);
        exit(-1);
    }

    long long *limit = malloc(sizeof(*limit));
    // Since argv[0] is a Str
    // Convert a Char array (Str) to Long long
    *limit = atoll(argv[1]);

    // Thread ID:
    pthread_t tid;

    // Create attributes:
    pthread_attr_t attr;

    pthread_attr_init(&attr);

    // Ready to create thread:
    pthread_create(&tid, &attr, sum_runner, limit);

    // Do other stuff here...



    // Wait until thread is done its work
    // The pthread_join() function shall suspend execution of the calling thread until the target thread terminates
    // Usage: int pthread_join(pthread_t thread, void **value_ptr);
    // leave value_ptr to NULL as we are going to return the value using Global variable
    long long *result; // join() allows void* but we already know the data type is long long
    pthread_join(tid, (void**)&result); // address of the pointer (**value_ptr), casting type (void**) to make the compiler happy

    printf("Sum is %lld\n", *result);
    free(result);
}
