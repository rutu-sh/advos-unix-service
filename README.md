# Nameserver

<!-- TODO - 
- daemonize
    - redirect logs to logfile
- QoL for client input -->


## Group Members
- Ruturaj Shitole
- Boxin Yang
- Swarup Totloor
- Prince Noah Johnson

## Description

A nameserver that listens on a named domain socket and listens for connections, where a client connecting to the nameserver can perform one of the two actions: 

- Publish a resource - `PUB <resource>`
- Request a resource - `REQ <resource>`

The server maintains a list of active connections and the associated resources they provide. 

Whenever a client requests for a resource, the server asks the associated client with the resource to provide a file descriptor and then it relays the file descriptor to the client requesting for it. 

In this project, we are only concerned with files as resources being passed around. 

## Methodology

### Project Contributions

The project was divided into several parts, with each group member contributing to different aspects:

- **Ruturaj Shitole**: Core structure with domain sockets, testing and Makefile
- **Boxin Yang**: Client implementation, passing file descriptors
- **Swarup Totloor**: Server implementation with epoll, request relay between clients
- **Prince Noah Johnson**: Authentication

### High-Level Design

How do we handle each of the requirements for the project? The nameserver:
1. Listens on a named domain socket for incoming client connections.
2. Maintain a registry of active connections and their associated resources.
3. Handle `PUB <resource>` commands to publish resources.
4. Handle `REQ <resource>` commands to request resources.
5. Facilitate the transfer of file descriptors between clients.

For more detailed information on the implementation, please refer to the following files:
- [Core server loop](./src/server/main.c)
- [Client loop](./src/client/main.c)
- [Auth implementation](./src/server/auth.c)
- [Tests](./tests/)

### Testing

To run the tests:
1. Compile the project using the provided Makefile: `make`
2. Run the server: `./server.out`
3. Run client(s): `./client.out`

4. Run the tests: `make test`

## Attributions

These resources proved to be useful during the development of this project.

1. Domain sockets and event loops - 
    - [Systems Programming textbook](https://gwu-cs-advos.github.io/sysprog/#communication-with-multiple-clients)
    - [Sockets man page](https://man7.org/linux/man-pages/man2/socket.2.html)

2. Passing FDs - 
    - [The Linux Programming Interface](https://man7.org/tlpi/code/online/dist/sockets/scm_multi_recv.c.html)

3. Authentication - 
    - resource

4. Testing with socket pairs - 
    - [Socket pairs man page](https://man7.org/linux/man-pages/man2/socketpair.2.html)
    - [Stack Overflow post](https://stackoverflow.com/questions/11461106/socketpair-in-c-unix)
