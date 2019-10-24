Interactive Dining Philosophers Problem
=======================================

Solved by using Resource Hierarchy Solution. Implemented with C pthread library Condition variables.


Compile dinphil.c
---------------------
```
gcc --std=c99 -Wall -g -Werror dinphil.c -o dinphil -lpthread
```

**Run**
```
./dinphil
```

How to interact with the program
-------------------------------------
The commands for user interface should be interpreted as follows:

- `T 0` is a request to think from Philosopher 0;
- `E 1` is a request to eat from Philosopher 1.
- In general, T i and E i are a think request and an eat request from Philosopher i, respectively.

- If the input is `P`, the output should be a representation of states of philosophers in turn and separated by one space, immediately followed by a '\n' at the end. '0' represents the state of thinking. '1' represents the state of eating.
For instance, entering P in the beginning of program execution would output `0 0 0 0 0\n`. 

- Entering `!` should immediately terminate the program.

![alt text](https://github.com/jdiggidawg/dining-philosophers/blob/master/demo.png "demo")


Abstract
------------

In the Dining Philosophers problem, a deadlock can happen if all 5 philosophers pick up 1 fork either only their left or only on their right.

Resource hierarchy helps resolve this deadlock by taking out the circular wait condition ( 1 of the 4 conditions that are required for a deadlock to happen).

The resources (forks) will be numbered 0 through 4 and each unit of work (philosopher) will always pick up the lower-numbered fork first, and then the higher-numbered fork, from among the two forks they plan to use. If four of the five philosophers simultaneously pick up their lower-numbered fork, only the highest-numbered fork will remain on the table, so the fifth philosopher will not be able to pick up any fork.

Notes about the program:
---------------------------

- It is assumed that once a philosopher has requested to EAT, they cannot request to THINK and
put the acquired fork down. They can only think once their EAT request has been serviced.

- The shared resources are managed by:
  1. A global array fork_state[]
  2. A condition variable array fork_cond[]
  3. Bool Has_left_fork, has_right_fork in a philosopher_t struct

- Pthread_cond_wait() and and pthread_cond_signal() are used extensively to manage the
acquirement of the forks.

- You may change NUM_OF_PHILS to see more than 5 philosophers.

- Under user input handling for input ‘P’. You can uncomment printStateVerbose() to see more
state information. Note that number 2 indicates the philosopher is hungry- for example:  `1 2 1 0 0\n`


