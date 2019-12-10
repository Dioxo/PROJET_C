#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#ifndef SERVICE_ORCHESTRE_H
#define SERVICE_ORCHESTRE_H

// Ici toutes les communications entre l'orchestre et les services :
// - le tube anonyme pour que l'orchestre envoie des données au service
// - le sémaphore pour que  le service indique à l'orchestre la fin
//   d'un traitement


typedef struct {
    int fd[2];
} AnonymeTube;

/* =================================
 		Envois et réceptions
   =================================  */
void orchestreWrite(AnonymeTube *anonyme, const void *buf, size_t size);
void serviceRead(AnonymeTube *anonyme, void *buf, size_t size);

#endif
