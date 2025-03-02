# compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra $(INCLUDES) -MMD -MP

# directories 
BUILD_DIR  = .build
SRC_DIR = src
SERVER_SRC = $(SRC_DIR)/server
CLIENT_SRC = $(SRC_DIR)/client
COMMON_SRC = $(SRC_DIR)/common
INCLUDES = -I$(SRC_DIR)

SERVER_DEPS = $(SERVER_SRC) $(COMMON_SRC)
CLIENT_DEPS = $(CLIENT_SRC) $(COMMON_SRC)

SERVER_SOURCES = $(shell find $(SERVER_DEPS) -type f -name *.c)
CLIENT_SOURCES = $(shell find $(CLIENT_DEPS) -type f -name *.c)

SERVER_OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SERVER_SOURCES))
CLIENT_OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CLIENT_SOURCES))

SERVER_TARGET = server.out
CLIENT_TARGET = client.out

all: clean $(SERVER_TARGET) $(CLIENT_TARGET)


$(SERVER_TARGET): $(SERVER_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT_TARGET): $(CLIENT_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c -o $@ $< $(CFLAGS)


clean:
	rm -rf .build server.out client.out