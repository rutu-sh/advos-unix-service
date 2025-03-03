# compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra $(INCLUDES) -MMD -MP

# directories 
BUILD_DIR  = .build
SRC_DIR = src
SERVER_SRC = $(SRC_DIR)/server
CLIENT_SRC = $(SRC_DIR)/client
COMMON_SRC = $(SRC_DIR)/common
TEST_SRC   = $(SRC_DIR)/test
INCLUDES = -I$(SRC_DIR)


TESTS = $(shell find $(TEST_SRC) -type f -name *.c)

SERVER_DEPS = $(SERVER_SRC) $(COMMON_SRC)
CLIENT_DEPS = $(CLIENT_SRC) $(COMMON_SRC)
TEST_DEPS   = $(COMMON_SRC) # $(SERVER_SRC) $(CLIENT_SRC)

SERVER_SOURCES = $(shell find $(SERVER_DEPS) -type f -name *.c)
CLIENT_SOURCES = $(shell find $(CLIENT_DEPS) -type f -name *.c)
TEST_SOURCES   = $(shell find $(TEST_DEPS)   -type f -name *.c ! -name main.c)

SERVER_OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SERVER_SOURCES))
CLIENT_OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CLIENT_SOURCES))
TEST_OBJECTS   = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(TEST_SOURCES))

SERVER_TARGET = server.out
CLIENT_TARGET = client.out
TEST_TARGETS  = $(TESTS:.c=.out)

all: clean $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT_TARGET): $(CLIENT_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_SRC)/%.out: $(BUILD_DIR)/test/%.o $(TEST_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $< 

$(BUILD_DIR)/test/%.o: $(SRC_DIR)/test/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $< 

test: clean $(TEST_TARGETS)


clean:
	@echo $(TEST_OBJECTS)
	@echo $(TEST_TARGETS)
	@rm -rf .build server.out client.out src/test/*.out