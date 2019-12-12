#ifndef CLIENT_ORCHESTRE_H
#define CLIENT_ORCHESTRE_H

// Ici toutes les communications entre l'orchestre et les clients :
// - le sémaphore pour que 2 clients ne conversent pas en même
//   temps avec l'orchestre
// - les deux tubes nommés pour la communication bidirectionnelle
#define REQUEST_FAIL -1
#define REQUEST_STOP -2
#define REQUEST_ASK 1
#define REQUEST_ACCEPT 2
#define REQUEST_EOF 3


#define MON_FICHIER "client_orchestre.h"
#define MON_ID 5

typedef struct
{
    char *name;
    int fd;
} co_NamedPipe;

typedef struct
{
    co_NamedPipe CtoO;
    co_NamedPipe OtoC;
} co_Pair;

typedef struct
{
    int request;
} co_Connection;

typedef struct
{
    int password;
    int lengthCtoS;
    int lengthStoC;
    char *CtoS;
    char *StoC;
} co_Response;

/* ===================================================================================
                                   GESTION DES PIPES
   ===================================================================================   */

/* ============================
 	Constructeur et Destructeur
   ============================   */

void co_orchestraCreatePipes(co_Pair *pipes);

void co_orchestraDestroyPipes(co_Pair *pipes);

/* =================================
 	Ouverture et fermeture des tubes
   =================================  */

void co_orchestraOpenPipes(const char *nameCtoO, const char *nameOtoC, co_Pair *pipes);
void co_orchestraClosePipes(co_Pair *pipes);
void co_clientOpenPipes(const char *nameCtoO, const char *nameOtoC, co_Pair *pipes);
void co_clientClosePipes(co_Pair *pipes);

/* =================================
 		Envois et réceptions
   =================================  */

void co_clientWriteData(co_Pair *pipes, const void *buf, size_t size);
void co_orchestraWriteData(co_Pair *pipes, const void *buf, size_t size);
void co_clientReadData(co_Pair *pipes, void *buf, size_t size);
void co_orchestraReadData(co_Pair *pipes, void *buf, size_t size);

/* =================================
 		Attente d'une réponse
   =================================  */


/* =================================
		Accesseur et mutateur
   =================================  */


/* ===================================================================================
                                   GESTION DES SEMAPHORES
   ===================================================================================   */

/* ============================
        Abstraction pointeur
   ============================   */

struct SemaphoreP;
typedef struct SemaphoreP *Semaphore;

/* ============================
  Constructeur et Destructeur
   ============================   */

Semaphore createSema(int val);
void destroySema(Semaphore *self);

/* ============================
    Opération sur un semaphore
   ============================   */

void waitSema(Semaphore self);
void vSema(Semaphore self);
void pSema(Semaphore self);

/* ============================
      Accesseur et mutateur
   ============================   */

int getValSema(Semaphore self);
void setValSema(Semaphore self, int val);

#endif
