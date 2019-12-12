#define _POSIX_C_SOURCE 200809L    // pour strdup

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>



#include "myassert.h"

#include "client_orchestre.h"

/* ===================================================================================
                                   GESTION DES PIPES
   ===================================================================================   */ 


//---------------------------------------------------------------------------------
/* ============================
    Constructeur et Destructeur
   ============================   */ 
//---------------------------------------------------------------------------------
static void createPipe(const char *basename, const char *msg , co_NamedPipe *pipe)
{
	int nameLength;
	nameLength = snprintf(NULL, 0, "%s", basename);
	pipe->name = malloc((nameLength + 1) * sizeof(char));
	sprintf(pipe->name, "%s", basename);

	int ret = mkfifo(pipe->name, 0600);
    myassert(ret == 0, msg);
}
//---------------------------------------------------------------------------------
void co_orchestraCreatePipes(co_Pair *pipes)
{
	createPipe("pipeClientToOrchestra", "Création tube CtoO", &(pipes->CtoO));
	createPipe("pipeOrchestraToClient", "Création tube OtoC", &(pipes->OtoC));

}


//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
static void destroyPipe(const char *msg, co_NamedPipe *pipe)
{
    int ret;

    ret = unlink(pipe->name);
    myassert(ret == 0, msg);
    free(pipe->name);
    pipe->name = NULL;
}
//---------------------------------------------------------------------------------
void co_orchestraDestroyPipes(co_Pair *pipes)
{
    destroyPipe("destruction tube CtoO", &(pipes->CtoO));
    destroyPipe("destruction tube OtoC", &(pipes->OtoC));
}


//---------------------------------------------------------------------------------
/* =================================
    Ouverture et fermeture des tubes
   =================================  */ 
//---------------------------------------------------------------------------------
static void openPipe(const char *name, int flag, const char *msg, co_NamedPipe *pipe)
{
    pipe->name = strdup(name);
    pipe->fd = open(pipe->name, flag);
    myassert(pipe->fd != -1, msg);
}
//---------------------------------------------------------------------------------
void co_orchestraOpenPipes(const char *nameCtoO, const char *nameOtoC, co_Pair *pipes)
{
	openPipe(nameOtoC, O_WRONLY, "client->orchestra ouvert en ecriture", &(pipes->OtoC));
	openPipe(nameCtoO, O_RDONLY, "orchestra->client ouvert en lecture", &(pipes->CtoO));
}
//---------------------------------------------------------------------------------
void co_clientOpenPipes(const char *nameCtoO, const char *nameOtoC, co_Pair *pipes)
{
	openPipe(nameOtoC, O_RDONLY, "client->orchestra ouvert en lecture", &(pipes->OtoC));
	openPipe(nameCtoO, O_WRONLY, "orchestra->client ouverture en ecriture", &(pipes->CtoO));

}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
static void closePipe(const char *msg, co_NamedPipe *pipe)
{
	int ret;
    
    free(pipe->name);
    pipe->name = NULL;
    ret = close(pipe->fd);
    myassert(ret == 0, msg);
    pipe->fd = -1;
}
//---------------------------------------------------------------------------------
void co_orchestraClosePipes(co_Pair *pipes)
{
	closePipe("orchestra fermeture tube CtoO",&(pipes->CtoO));
	closePipe("orchestra fermeture tube OtoC", &(pipes->OtoC));
}
//---------------------------------------------------------------------------------
void co_clientClosePipes(co_Pair *pipes)
{
	closePipe("client fermeture tube CtoO",&(pipes->CtoO));
	closePipe("client fermeture tube OtoC", &(pipes->OtoC));
}

//---------------------------------------------------------------------------------
/* =================================
        Envois et réceptions
   =================================  */ 
//---------------------------------------------------------------------------------
static void writeData(co_NamedPipe *pipe, const void *buf, size_t size, const char *msg)
{
    ssize_t ret = write(pipe->fd, buf, size);
    myassert(ret != -1, msg);
    myassert((size_t)ret == size, msg);
}
//---------------------------------------------------------------------------------
void co_orchestraWriteData(co_Pair *pipes, const void *buf, size_t size)
{
	writeData(&(pipes->OtoC), buf, size, "orchestra ouvert en écriture/client ouvert en lecture");
}
//---------------------------------------------------------------------------------
void co_clientWriteData(co_Pair *pipes, const void *buf, size_t size)
{
	writeData(&(pipes->CtoO), buf, size, "client ouvert en écriture/orchestra ouvert en lecture");
}

//---------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
static void readData(co_NamedPipe *pipe, void *buf, size_t size, const char *msg)
{
	ssize_t ret = read(pipe->fd, buf, size);
    myassert(ret != -1, msg);
    myassert((size_t)ret == size, msg);
}
//---------------------------------------------------------------------------------
void co_orchestraReadData(co_Pair *pipes, void *buf, size_t size)
{
    readData(&(pipes->CtoO), buf, size, "orchestra lecture de client");
}
//---------------------------------------------------------------------------------
void co_clientReadData(co_Pair *pipes, void *buf, size_t size)
{
    readData(&(pipes->OtoC), buf, size, "client lecture de orchestra");
}

//--------------------------------------------------------------------------------
/* =================================
        Attente d'une réponse
   =================================  */ 
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
/* =================================
        Accesseur et mutateur
   =================================  */ 
//--------------------------------------------------------------------------------

/*
NE SERT A RIEN
int main()
{
	    return EXIT_SUCCESS;
}

*/

/* ===================================================================================
                                   GESTION DES SEMAPHORES
   ===================================================================================   */ 

struct SemaphoreP
{
    int nbSem;
    int semId;
};

//--------------------------------------------------------------------------------
/* ============================
    Constructeur et Destructeur
   ============================   */ 
//--------------------------------------------------------------------------------

static key_t token()
{
    //Constantes définies dans le .h
    return ftok(MON_FICHIER, MON_ID);
}
//--------------------------------------------------------------------------------
Semaphore createSema(int val)
{
    Semaphore self = malloc(sizeof(struct SemaphoreP));
    self->nbSem =1;
    key_t tok = token();
    self->semId = semget(tok, self->nbSem, IPC_CREAT | 0666);
    myassert(self->semId != -1, "Création du sémaphore");
    semctl(self->semId, 0, SETVAL, val);
    myassert(self->semId != -1, "Initiatisation du sémaphore");

    return self;
}
//--------------------------------------------------------------------------------
void destroySema(Semaphore *self)
{
    int ret = semctl((*self)->semId, -1, IPC_RMID);
    myassert(ret != -1, "Destruction du Semaphore");
    free(*self);
    *self = NULL;
}

//--------------------------------------------------------------------------------
/* ============================
    Opération sur un semaphore
   ============================   */ 
//--------------------------------------------------------------------------------
static int OperationSema(Semaphore self, int num, int val)
{
    struct sembuf sops;
    sops.sem_num = num;
    sops.sem_op = val;
    sops.sem_flg = 0;

    return semop(self->semId, &sops, 1);
}
//--------------------------------------------------------------------------------
void waitSema(Semaphore self)
{
    int ret = OperationSema(self, 0 , 0);
    myassert(ret != -1, "Erreur : 0 sur le semaphore");
}
//--------------------------------------------------------------------------------
void vSema(Semaphore self)
{
    int ret = OperationSema(self, 0, 1);
    myassert(ret != -1, "Erreur : +1 sur le semaphore");
}
//--------------------------------------------------------------------------------
void pSema(Semaphore self)
{
    int ret = OperationSema(self, 0, -1);
    myassert(ret != -1, "Erreur : -1 sur le semaphore");
}
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
/* ============================
      Accesseur et mutateur
   ============================   */ 
//--------------------------------------------------------------------------------
int getValSema(Semaphore self)
{
    int ret = semctl(self->semId, 0, GETVAL);
    myassert(ret != -1, "Erreur : Lecture semaphore");
    return ret;
}
//--------------------------------------------------------------------------------
void setValSema(Semaphore self, int val)
{
    int ret = semctl(self->semId, 0, SETVAL, val);
    myassert(ret != -1, "Erreur : Ecriture semaphore");
}
//--------------------------------------------------------------------------------



