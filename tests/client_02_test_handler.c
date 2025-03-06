#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "client/client.h"
#include "common/passfd.h"
#include "common/errorcodes.h"

extern int data_sock;
const char* test_name = "client_02_test_handler";

// Test for handle_stdin_event() on a PUB command.
// This test uses a socketpair to emulate data_sock so that we can check what is written.
void test_handle_stdin_event_pub() {
    char w_buffer[BUFFER_SIZE] = "PUB test_pub.txt";
    char fr_buffer[BUFFER_SIZE];

    // Create a socketpair to simulate data_sock communication.
    int sv[2];
    int rc = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    assert(rc != -1);

    // Assign one end to data_sock.
    data_sock = sv[0];

    // Remove file if it exists.
    unlink("test_pub.txt");

    // Call handle_stdin_event which should process the PUB command.
    int ret = handle_stdin_event(w_buffer, fr_buffer);
    assert(ret >= 0);

    // The PUB branch writes the command to data_sock.
    // Read what was sent on the other end of the socket pair.
    char sock_buffer[BUFFER_SIZE];
    memset(sock_buffer, 0, sizeof(sock_buffer));
    read(sv[1], sock_buffer, sizeof(sock_buffer));
    // We expect that the command "PUB ..." is sent.
    assert(strncmp(sock_buffer, "PUB", 3) == 0);

    close(sv[0]);
    close(sv[1]);

    // Verify that the file "test_pub.txt" was created.
    int fd = open("test_pub.txt", O_RDONLY);
    assert(fd != -1);
    close(fd);
    unlink("test_pub.txt");

    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", test_name, "test_handle_stdin_event_pub");
}

// Test for handle_datasock_event() on a non-REQ message.
// We simulate a simple server message that isn't a REQ command.
void test_handle_datasock_event_nonreq() {
    char r_buffer[BUFFER_SIZE];
    int write_bytes;

    // Create a socketpair and assign one end as data_sock.
    int sv[2];
    int rc = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    assert(rc != -1);
    data_sock = sv[0];

    // Write a non-REQ message (e.g. a simple text) to the other socket.
    const char *message = "Hello from server";
    write_bytes = write(sv[1], message, strlen(message));
    assert(write_bytes > 0);

    // Call handle_datasock_event, which should read and print the message.
    int ret = handle_datasock_event(r_buffer);
    assert(ret == 0);

    close(sv[0]);
    close(sv[1]);

    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", test_name, "test_handle_datasock_event_nonreq");
}

int main() {
    test_handle_stdin_event_pub();
    test_handle_datasock_event_nonreq();
    return 0;
}