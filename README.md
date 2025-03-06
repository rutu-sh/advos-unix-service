# Nameserver 

A nameserver that listens on a named domain socket and listens for connections. 
A client connecting to the nameserver can perform one of the two actions: 

- Publish a resource 
- Request a resource 

The server maintains a list of active connections and the associated resources they provide. 

Whenever a client requests for a resource, the server asks the associated client with the resource to provide a file descriptor and then it relays the file descriptor to the client requesting for it. 

In this project, we are only concerned with Files as resources being passed around. 

