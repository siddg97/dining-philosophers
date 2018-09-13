#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


struct sum_runner_struct {
    long long limit;
    long long answer;
};

// Thread function to generate sum of 0 to N
void* sum_runner(void *arg)
{

    struct sum_runner_struct *arg_struct =
            (struct sum_runner_struct*) arg;

    long long sum = 0;

    for(long long i = 0; i<= arg_struct->limit; i++) {
        sum += i;
    }

    arg_struct->answer = sum;

    // My thread needs to finish
    // 0 means i'm not returning anything
    pthread_exit(0);

}

int main(int argc, char **argv)
{

    // Check user input. There has to be a number after a program name
    if (argc < 2) {
        printf("Usage: %s <num> <num 1> <num 2> ... <num n>\n", argv[0]);
        exit(-1);
    }

    // Ready to create many threads by taking multiple arguments
    int num_args = argc - 1; // -1 because of the filename at argv[0]


    // Variables that exist outside of the for loop below so we can print them
    struct sum_runner_struct args[num_args];

    //Launch threads
    // Thread IDs:
    pthread_t tids[num_args];

    // Ready to create many threads
    for (int i = 0; i < num_args; i++) {

        args[i].limit = atoll(argv[i+1]);

        // Create attributes:
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&tids[i], &attr, sum_runner, &args[i]);
    }



    // Do other stuff here...



    // Wait until thread is done its work
    // The pthread_join() unction shall suspend execution of the calling thread until the target thread terminates
    // Usage: int pthread_join(pthread_t thread, void **value_ptr);

    for (int i = 0; i < num_args; i++) {
        pthread_join(tids[i], NULL);
        printf("Sum for thread %d is %lld\n",
                i,
                args[i].answer);
    }
}
