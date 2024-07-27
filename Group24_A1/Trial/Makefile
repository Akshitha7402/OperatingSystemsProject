CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99
LDFLAGS = -lrt

SERVER_SRC = server.c
CLIENT_SRC = client.c
UTILS_SRC = utils.h
CLEANUP_SRC = cleanup.c

SERVER_OUT = server.o
CLIENT_OUT = client.o
CLEANUP_OUT = cleanup.o

.PHONY: all clean

all: compile-server compile-client

compile-server: $(SERVER_OUT)

compile-client: $(CLIENT_OUT)

compile-cleanup: $(CLEANUP_OUT)

$(SERVER_OUT): $(SERVER_SRC) $(UTILS_SRC)
	$(CC) $(CFLAGS) -o $(SERVER_OUT) $(SERVER_SRC)

$(CLIENT_OUT): $(CLIENT_SRC) $(UTILS_SRC)
	$(CC) $(CFLAGS) -o $(CLIENT_OUT) $(CLIENT_SRC)

$(CLEANUP_OUT): $(CLEANUP_SRC) $(UTILS_SRC)
	$(CC) $(CFLAGS) -o $(CLEANUP_OUT) $(CLEANUP_SRC)

server: compile-server 
	./$(SERVER_OUT)

client: compile-client
	./$(CLIENT_OUT)

cleanup: compile-cleanup
	./$(CLEANUP_OUT)

clean:
	rm -f $(SERVER_OUT) $(CLIENT_OUT) $(CLEANUP_OUT)
