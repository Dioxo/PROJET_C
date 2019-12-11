#ifndef CLIENT_COMPRESSION
#define CLIENT_COMPRESSION

// on ne déclare ici que les deux fonctions appelables par le client
void client_compression_sendData(Pair *pipes, int argc, char * argv[]);
void client_compression_receiveResult(Pair *pipes, int argc, char * argv[]);

#endif
