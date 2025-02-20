# Nameserver 


A nameserver that allows clients to discover each other based on the services they need. Each registered client provides the nameserver with the service it provides. If a client needs some services, it communicates with the nameserver and the nameserver, after authenticating the client, provides it with the FDs to communicate with the the registered client. 

## Requirements 
### Phase 1
1. UNIX domain sockets + accept per client (Rutu, Swarup)
2. event notification (epoll etc.) (Swarup)
3. Auth for client (Prince)
    - Counters to track resource consumption 
4. Pass file descriptors to and from clients (Boxin, Rutu)
5. Set up project compilation etc (makefile) (Swarup)

### Phase 2
1. Different clients talking to each other (1 client per person), spec:
        - Duplicate FDs
        - Protocol to talk to server for different actions
2. Tests for each component