CC = gcc
CFLAGS = -pthread -std=gnu99 -ggdb

DEPS = webthreads/webthreads.h webthreads/threads.h utils.h string_util.h

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

OBJ_S = server.o webthreads/webthreads.o
server: $(OBJ_S)
	$(CC) $(CFLAGS) -o $@ $^

OBJ_C = client.o
client: $(OBJ_C)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf ./server
	rm -rf ./client
	find . -name "*.o" -type f -delete
