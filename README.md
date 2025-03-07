# Nameserver

## Group Members
- Ruturaj Shitole
- Boxin Yang
- Swarup Totloor
- Prince Noah Johnson

## Description

A nameserver that listens on a named domain socket and listens for connections, where a client connecting to the nameserver can perform one of the three actions: 

- Publish a resource - `PUB <resource>`
- Request a resource - `REQ <resource>`
- Exit - `exit`

The server maintains a list of active connections and the associated resources they provide. 

Whenever a client requests for a resource, the server asks the associated client with the resource to provide a file descriptor and then it relays the file descriptor to the client requesting for it. 

In this project, we are only concerned with files as resources being passed around. So when a file is published, the client just opens that particular file with some identifying text (if the file doesn't exist.)  

## Methodology

### Project Contributions

The project was divided into several parts, with each group member contributing to different aspects:

- **Ruturaj Shitole**: Core structure with domain sockets, testing and Makefile
- **Boxin Yang**: Client implementation, passing file descriptors
- **Swarup Totloor**: Server implementation with epoll, request relay between clients
- **Prince Noah Johnson**: Authentication and its testing

### High-Level Design

How do we handle each of the requirements for the project? The nameserver:
1. Listens on a named domain socket for incoming client connections. (`/tmp/sock-test.socket`)
2. Uses `accept()` to create unique fd per client in the main event loop of the server.
3. The event loop of the server uses `epoll` for concurrency.
4. Authenticates each client using `getsockopt`.
5. Relays file descriptors of resources between clients using `sendmsg` and `recvmsg`.

For more detailed information on the implementation, please refer to the following files:
- [Core server loop](./src/server/main.c)
- [Client loop](./src/client/main.c)
- [Auth implementation](./src/server/auth.c)
- [Tests](./tests/)

### Testing

To run our project:
1. Compile the project using the provided Makefile: 
    ```bash
    make
    ```
    The expected output should look like this: 
    ```bash
    (TERMINAL_1)@/advos-unix-service$ make
    gcc -Wall -Wextra -Isrc -MMD -MP -c -o .build/server/auth.o src/server/auth.c 
    gcc -Wall -Wextra -Isrc -MMD -MP -c -o .build/server/main.o src/server/main.c 
    gcc -Wall -Wextra -Isrc -MMD -MP -c -o .build/server/server.o src/server/server.c 
    gcc -Wall -Wextra -Isrc -MMD -MP -c -o .build/common/logger.o src/common/logger.c 
    gcc -Wall -Wextra -Isrc -MMD -MP -c -o .build/common/passfd.o src/common/passfd.c 
    gcc -Wall -Wextra -Isrc -MMD -MP -o server.out .build/server/auth.o .build/server/main.o .build/server/server.o .build/common/logger.o .build/common/passfd.o
    gcc -Wall -Wextra -Isrc -MMD -MP -c -o .build/client/client.o src/client/client.c 
    gcc -Wall -Wextra -Isrc -MMD -MP -c -o .build/client/main.o src/client/main.c 
    gcc -Wall -Wextra -Isrc -MMD -MP -o client.out .build/client/client.o .build/client/main.o .build/common/logger.o .build/common/passfd.o
    ```
2. Run the server: 
    ```bash
    make start_nameserver
    ```
    The expected output should look like this:
    ```bash
    (TERMINAL_1)@/advos-unix-service$ make start_nameserver
    ./server.out > server.log 2>&1
    ```
3. Run client 1: 
    ```bash
    make start_client
    ```
    The expected output should look like this:
    ```bash
    (TERMINAL_2)@/advos-unix-service$ make start_client 
    ./client.out
    [info] creating data socket
    [info] created data socket
    Commands available:
        1. PUB <resource>
        2. REQ <resource>
        3. exit
    client>
    ```

4. Run client 2 in another terminal: 
    ```bash
    make start_client
    ```
    The expected output should look like this:
    ```bash
    (TERMINAL_3)@/advos-unix-service$ make start_client 
    ./client.out
    [info] creating data socket
    [info] created data socket
    Commands available:
        1. PUB <resource>
        2. REQ <resource>
        3. exit
    client>
    ```

4. Input commands in any client to interact with the other clients.

5. Observe `server.log` to see the logs for nameserver. If you would like to kill the nameserver, simply use `kill <pid>` using the PID from the log file.

6. Run the tests: 
    ```bash
    make test
    ```
    The expected output should look like this:
    ```bash
    (TERMINAL_1)@/advos-unix-service$ make test
    gcc -Wall -Wextra -Isrc -MMD -MP -c -o .build/common/logger.o src/common/logger.c 
    gcc -Wall -Wextra -Isrc -MMD -MP -c -o .build/common/passfd.o src/common/passfd.c 
    gcc -Wall -Wextra -Isrc -MMD -MP -c -o .build/server/auth.o src/server/auth.c 
    gcc -Wall -Wextra -Isrc -MMD -MP -c -o .build/server/server.o src/server/server.c 
    gcc -Wall -Wextra -Isrc -MMD -MP -c -o .build/client/client.o src/client/client.c 
    ---------RUNNING TESTS---------
    test:common_02_test_passfd:test_send_recv_fd:PASSED
    test:client_02_test_handler:test_handle_stdin_event_pub:PASSED
    read 17 bytes
    server> Hello from server

    server> Hello from server
    test:client_02_test_handler:test_handle_datasock_event_nonreq:PASSED
    test:server_01_test_utils:test_init:PASSED
    test:server_01_test_utils:test_create_connection_socket:PASSED
    test:server_01_test_utils:test_bind_connection_socket:PASSED
    test:server_01_test_utils:test_create_epoll_fd:PASSED
    test:server_01_test_utils:test_set_nonblocking:PASSED
    test:server_01_test_utils:test_do_op:PASSED
    test:server_01_test_utils:test_get_resource_from_message:PASSED
    test:client_01_test_utils:test_init:PASSED
    test:client_01_test_utils:test_create_data_socket:PASSED
    test:client_01_test_utils:test_create_epoll_fd:PASSED
    test:client_01_test_utils:test_set_nonblocking:PASSED
    test:server_01_test_auth:test_get_client_credentials_invalid:PASSED
    test:common_01_test_logger:test_std_logger_init:PASSED
    test:common_01_test_logger:test_std_log:PASSED
    ---------FINISHED TESTS---------
    ```


## Attributions

These resources proved to be useful during the development of this project.

1. Domain sockets and event loops - 
    - [Systems Programming textbook](https://gwu-cs-advos.github.io/sysprog/#communication-with-multiple-clients)
    - [Sockets man page](https://man7.org/linux/man-pages/man2/socket.2.html)

2. Passing FDs - 
    - [The Linux Programming Interface](https://man7.org/tlpi/code/online/dist/sockets/scm_multi_recv.c.html)

3. Authentication - 
    - [IBM Docs](https://www.ibm.com/docs/en/zos/3.1.0?topic=functions-getsockopt-get-options-associated-socket)
    - [Getsockopt man page](https://man7.org/linux/man-pages/man2/getsockopt.2.html)

4. Testing with socket pairs - 
    - [Socket pairs man page](https://man7.org/linux/man-pages/man2/socketpair.2.html)
    - [Stack Overflow post](https://stackoverflow.com/questions/11461106/socketpair-in-c-unix)

5. ChatGPT - 
    - Color codes: Prompt("give me all the color codes for linux terminals")

