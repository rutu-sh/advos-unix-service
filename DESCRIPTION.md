## 1: UNIX Service

For this, you'll implement a trusted UNIX daemon that provides services to the rest of the system.
Examples of existing similar technologies are `systemd` and `launchd`.
Simpler historical services include `inetd` and `init`, for example in `busybox`.

Your service must do all of the following:

1. Use UNIX domain sockets for communication with a known path for the *name* of the socket.
2. Use `accept` to create a connection-per-client as in [here](https://gwu-cs-advos.github.io/sysprog/#communication-with-multiple-clients).
3. Use event notification (epoll, poll, select, etc...) to manage concurrency as in [here](https://gwu-cs-advos.github.io/sysprog/#event-loops-and-poll).
4. Use domain socket facilities to get a trustworthy identity of the client (i.e. user id).
5. Pass file descriptors between the service and the client.

[The Linux Programming Interface](https://man7.org/tlpi/code/index.html) is a great resource for code for domain sockets and related technologies.
For example, see the [client](https://man7.org/tlpi/code/online/dist/sockets/ud_ucase_cl.c.html) and [server](https://man7.org/tlpi/code/online/dist/sockets/ud_ucase_sv.c.html) code for 1. getting authenticated client id, and 2. [passing](https://man7.org/tlpi/code/online/dist/sockets/scm_multi_send.c.html) [file-descriptors](https://man7.org/tlpi/code/online/dist/sockets/scm_multi_recv.c.html) from the service to the client.
You should implement your service in C/C++/or Rust as I'd like you to not use a language that drastically hides the APIs.

This set of requirements enables the service to have higher permissions than its clients (e.g. to run as `root`), thus have access to many resources, yet to provide a service in which a subset of those resources are given to clients based on their requests and their user ids.

Other than this set of requirements, you're free to do whatever you'd like!
Some examples:

- The service can host a database (e.g. SQLite) and provide its own policies for allowing clients to access different parts of the data-base.
- The service can play the role of a nameserver, enabling clients to find either other based on what services they offer (identified with a string), and which they want to leverage ( this is the core of how notifications work in Linux, for example power, wifi, etc...).
- Provide a routing service to connect a client to another program (similar to the nameserver), but with an emphasis on enabling this communication to be local (on the machine), or remote (via network sockets) based on where the other program is located.
- Provide something similar to a [9p service](https://en.wikipedia.org/wiki/9P_(protocol)) to access a virtual (user-level service controlled) file system image.
  The service speaks 9p with the client, and shares `fd`s to provide direct access to files.
- Provide an LLM service with per-client context and replies; the shared file descriptor might be a scratchpad of the entire conversation with each client.
- ...

Detail what you've done in the `README.md`, including who worked on what part.
Provide high-level context for what your service's goals are, what it is trying to do, how it uses 1-5 above, and some very high-level documentation into the design with links to the corresponding code.
Provide information about your testing of the project, and how I can run the tests.
