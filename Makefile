#CFLAGS=-Wall -Wextra -msse2 -mfpmath=sse -DSSE
CFLAGS=-Wall -Wextra -O0 -mfpmath=387 -mno-sse -m32
OBJS = main.o
TARGET = bench-flops

all: bench-flops

bench-flops: ${OBJS}
	gcc $(CFLAGS) -g -o $@ ${OBJS}

%.o: %.c
	gcc $(CFLAGS) -c -g $< -o $@

clean:
	rm -f ${OBJS}  ${TARGET}
