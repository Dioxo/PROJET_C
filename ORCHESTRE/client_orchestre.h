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



/* ============================
 	Constructeur et Destructeur
   ============================   */ 

void co_orchestraCreatePipes(co_Pair *pipes);
void co_orchestraCreateThread(co_Pair *pipes);

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

//pthread_mutex_t getMutex(int numService);




#endif
