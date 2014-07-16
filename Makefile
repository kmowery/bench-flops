CC=gcc

#CFLAGS=-Wall -Wextra -msse2 -mfpmath=sse -DSSE
#CFLAGS=-Wall -Wextra -O0 -mfpmath=387 -mno-sse
CFLAGS=-Wall -Wextra -O0
OBJS = main.o
TARGET = bench-flops

all: bench-flops

bench-flops: ${OBJS}
	$(CC) $(CFLAGS) -g -o $@ ${OBJS}

32: ${OBJS}
	$(CC) $(CFLAGS) -m32 -g -o bench-flops32 ${OBJS}

%.o: %.c
	$(CC) $(CFLAGS) -E -g $< -o $<.pre
	$(CC) $(CFLAGS) -c -g $< -o $@

clean:
	rm -f ${OBJS}  ${TARGET} bench-flops32
