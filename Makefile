.PHONY: all test clean coverage

CC = clang
CFLAGS = -Wall -Wextra -Og -g3 -std=c11 -pedantic -Wimplicit-fallthrough

PROG = x
SRCS = test.c jsonmodoki.c debug.c string.c util.c
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)
GCNO = $(SRCS:.c=.gcno)
GCDA = $(SRCS:.c=.gcda)
GCOV = $(SRCS:.c=.c.gcov)
LDFLAGS =

all: $(PROG)

coverage: CFLAGS += --coverage
coverage: all
	./$(PROG)
	for gcda in $(GCDA); do llvm-cov gcov $$gcda; done

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

clean:
	rm -f $(PROG) $(OBJS) $(DEPS) $(GCNO) $(GCDA) $(GCOV)

test: all
	./$(PROG)

sinclude $(DEPS)
