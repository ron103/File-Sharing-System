# Compiler and flags
CC = gcc
CFLAGS = -pthread

# Targets
SERVER = server
CLIENT = client

# Default target to build and run
all: $(SERVER) $(CLIENT)
	./$(SERVER)

# Build the server
$(SERVER): server.c
	$(CC) $(CFLAGS) server.c -o $(SERVER)

# Build the client
$(CLIENT): client.c
	$(CC) client.c -o $(CLIENT)

# Clean up generated files
clean:
	rm -f $(SERVER) $(CLIENT)

# Phony targets
.PHONY: all clean