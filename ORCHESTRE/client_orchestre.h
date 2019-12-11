#ifndef CLIENT_ORCHESTRE_H
#define CLIENT_ORCHESTRE_H

// Ici toutes les communications entre l'orchestre et les clients :
// - le sémaphore pour que 2 clients ne conversent pas en même
//   temps avec l'orchestre
// - les deux tubes nommés pour la communication bidirectionnelle
#define REQUEST_FAIL -1
#define REQUEST_STOP 0
#define REQUEST_ASK 1
#define REQUEST_ACCEPT 2
#define REQUEST_EOF 3

typedef struct 
{
    char *name;
    int fd;
} NamedPipe;

typedef struct 
{
    NamedPipe CtoO;
    NamedPipe OtoC;
} Pair;

typedef struct 
{
    int request;     
} Connection;

typedef struct 
{
    int password;
    int lengthCtoO;
    int lengthOtoC;
    char *CtoO;
    char *OtoC;
} Response;



/* ============================
 	Constructeur et Destructeur
   ============================   */ 

void co_orchestraCreatePipes(Pair *pipes);
void co_orchestraCreateThread(Pair *pipes);

void co_orchestraDestroyPipes(Pair *pipes);

/* =================================
 	Ouverture et fermeture des tubes
   =================================  */ 

void co_orchestraOpenPipes(const char *nameCtoO, const char *nameOtoC, Pair *pipes);
void co_orchestraClosePipes(Pair *pipes);
void co_clientOpenPipes(const char *nameCtoO, const char *nameOtoC, Pair *pipes);
void co_clientClosePipes(Pair *pipes);

/* =================================
 		Envois et réceptions
   =================================  */ 

void co_clientWriteData(Pair *pipes, const void *buf, size_t size);
void co_orchestraWriteData(Pair *pipes, const void *buf, size_t size);
void co_clientReadData(Pair *pipes, void *buf, size_t size);
void co_orchestraReadData(Pair *pipes, void *buf, size_t size);

/* =================================
 		Attente d'une réponse
   =================================  */ 


/* =================================
		Accesseur et mutateur
   =================================  */ 

//pthread_mutex_t getMutex(int numService);




#endif
