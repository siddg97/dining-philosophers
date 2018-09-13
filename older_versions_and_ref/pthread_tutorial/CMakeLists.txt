cmake_minimum_required(VERSION 3.10)
project(pthread_tutorial C)

set(TARGETS= count sum_on_thread sum_on_many_threads sum_malloc)

set(CMAKE_C_STANDARD 99)
#set(CFLAGS "-${CFLAGS} -Wall -g -std=c99 -Werror -pthread")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -g -Werror")

#set_target_properties(${TARGETS} PROPERTIES COMPILE_FLAGS ${CFLAGS})

set(SOURCE_FILES
#        sum_on_thread.c
#        sum_on_many_threads.c
#        sum_malloc.c
        count.c
        )

set(VERBOSE=1)

add_executable(pthread_tutorial ${SOURCE_FILES})


