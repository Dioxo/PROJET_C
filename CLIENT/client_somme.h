/* NOMS ET PRENOMS : HAYRAULT JULIEN - MOLINARES VALENCIA DIOGENES */
#ifndef CLIENT_SOMME
#define CLIENT_SOMME

// on ne déclare ici que les deux fonctions appelables par le client
void client_somme_sendData(Pair *pipes, int argc, char * argv[]);
void client_somme_receiveResult(Pair *pipes, int argc, char * argv[]);

#endif
