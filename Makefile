# compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra $(INCLUDES) -MMD -MP

# directories 
BUILD_DIR  = .build
SRC_DIR = src
SERVER_SRC = $(SRC_DIR)/server
CLIENT_SRC = $(SRC_DIR)/client
COMMON_SRC = $(SRC_DIR)/common
TEST_SRC   = tests
INCLUDES = -I$(SRC_DIR)

# project 
COMMON_DEPS = $(COMMON_SRC)
SERVER_DEPS = $(SERVER_SRC) $(COMMON_DEPS)
CLIENT_DEPS = $(CLIENT_SRC) $(COMMON_DEPS)

SERVER_SOURCES = $(shell find $(SERVER_DEPS) -type f -name *.c)
CLIENT_SOURCES = $(shell find $(CLIENT_DEPS) -type f -name *.c)

SERVER_TARGET = server.out
CLIENT_TARGET = client.out

SERVER_OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SERVER_SOURCES))
CLIENT_OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CLIENT_SOURCES))

# test
COMMON_TESTS = $(shell find $(TEST_SRC) -type f -name common_*.c)
SERVER_TESTS = $(shell find $(TEST_SRC) -type f -name server_*.c)
CLIENT_TESTS = $(shell find $(TEST_SRC) -type f -name client_*.c)

COMMON_TEST_SOURCES   = $(shell find $(COMMON_DEPS)   -type f -name *.c ! -name main.c)
SERVER_TEST_SOURCES   = $(shell find $(SERVER_DEPS)   -type f -name *.c ! -name main.c)
CLIENT_TEST_SOURCES   = $(shell find $(CLIENT_DEPS)   -type f -name *.c ! -name main.c)

COMMON_TEST_OBJECTS   = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(COMMON_TEST_SOURCES))
SERVER_TEST_OBJECTS   = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SERVER_TEST_SOURCES))
CLIENT_TEST_OBJECTS   = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CLIENT_TEST_SOURCES))


# project build targets
all: clean $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT_TARGET): $(CLIENT_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $< 

# tests build targets
compile_common_tests: $(COMMON_TEST_OBJECTS)
	@for target in $(COMMON_TESTS); do \
		target_obj=$$(echo $$target | sed 's/\.c$$/.out/'); \
		echo $(COMMON_TEST_OBJECTS); \
		$(CC) $(CFLAGS) -o $$target_obj $$target $(COMMON_TEST_OBJECTS); \
	done


compile_server_tests: $(SERVER_TEST_OBJECTS)
	@for target in $(SERVER_TESTS); do \
		target_obj=$$(echo $$target | sed 's/\.c$$/.out/'); \
		echo $(SERVER_TEST_OBJECTS); \
		$(CC) $(CFLAGS) -o $$target_obj $$target $(SERVER_TEST_OBJECTS); \
	done


compile_client_tests: $(CLIENT_TEST_OBJECTS)
	@for target in $(CLIENT_TESTS); do \
		target_obj=$$(echo $$target | sed 's/\.c$$/.out/'); \
		echo $(CLIENT_TEST_OBJECTS); \
		$(CC) $(CFLAGS) -o $$target_obj $$target $(CLIENT_TEST_OBJECTS); \
	done


$(BUILD_DIR)/tests/%.o: $(TEST_SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $< 

test: clean compile_common_tests compile_server_tests compile_client_tests

# cleanup
clean:
	@rm -rf .build server.out client.out tests/*.out tests/*.o tests/*.d