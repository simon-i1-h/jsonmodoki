.PHONY: all test clean

CC = clang
CFLAGS = -Wall -Wextra -Og -g3 -std=c11 -pedantic -Wimplicit-fallthrough

PROG = x
SRCS = test.c jsonmodoki.c debug.c string.c util.c
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

clean:
	rm -f $(PROG) $(OBJS) $(DEPS)

test: all
	./$(PROG)

sinclude $(DEPS)
