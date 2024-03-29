/* NOMS ET PRENOMS : HAYRAULT JULIEN - MOLINARES VALENCIA DIOGENES */
#ifndef CLIENT_SERVICE_H
#define CLIENT_SERVICE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>


#define SERVICE_EOF 4

// Ici toutes les communications entre les services et les clients :
// - les deux tubes nommés pour la communication bidirectionnelle
typedef struct {
    char *name;
    int fd;
} NamedPipe;

typedef struct {
    NamedPipe C_S;
    NamedPipe S_C;
} Pair;



/* ============================
 	Constructeur et Destructeur
   ============================   */

void createPipes(int numService, Pair *pipes);

void destroyPipes(Pair *pipes);

/* =================================
 	Ouverture et fermeture des tubes
   =================================  */

void clientOpenPipes(const char *s_c,const char *c_s, Pair *pipes);
void clientClosePipes(Pair *pipes);
void serviceOpenPipes(const char *s_c,const char *c_s,Pair *pipes);
void serviceClosePipes(Pair *pipes);

/* =================================
 		Envois et réceptions
   =================================  */

void clientWriteData(Pair *pipes, const void *buf, size_t size);
void serviceWriteData(Pair *pipes, const void *buf, size_t size);
void clientReadData(Pair *pipes, void *buf, size_t size);
void serviceReadData(Pair *pipes, void *buf, size_t size);

#endif
