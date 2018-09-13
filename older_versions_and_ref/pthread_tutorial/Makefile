TARGETS= sum_on_thread sum_on_many_threads sum_malloc count

# std=c99 because of verbal declaration in the for loop
CFLAGS = -Wall -g -std=c99 -Werror -pthread

CC_CPP = g++
CC_C = gcc

all: clean $(TARGETS)

$(TARGETS):
	$(CC_C) $(CFLAGS) $@.c -o $@

clean:
	-rm -f $(TARGETS)
