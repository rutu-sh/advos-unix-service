# Nameserver 


A nameserver that allows clients to discover each other based on the services they need. Each registered client provides the nameserver with the service it provides. If a client needs some services, it communicates with the nameserver and the nameserver, after authenticating the client, provides it with the FDs to communicate with the the registered client. 