#ifndef CLIENT_ORCHESTRE_H
#define CLIENT_ORCHESTRE_H

// Ici toutes les communications entre l'orchestre et les clients :
// - le sémaphore pour que 2 clients ne conversent pas en même
//   temps avec l'orchestre
// - les deux tubes nommés pour la communication bidirectionnelle

#define REQUEST_ASK 1
#define REQUEST_FAIL 0
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
    char *CtoO;
    char *OtoC;
} Response;



/* ============================
 	Constructeur et Destructeur
   ============================   */ 

void orchestraCreatePipes(Pair *pipes);
void orchestraCreateThread(Pair *pipes);

void orchestraDestroyPipes(Pair *pipes);

/* =================================
 	Ouverture et fermeture des tubes
   =================================  */ 

void orchestraOpenPipes(const char *nameCtoO, const char *nameOtoC, Pair *pipes);
void orchestraClosePipes(Pair *pipes);
void clientOpenPipes(const char *nameCtoO, const char *nameOtoC, Pair *pipes);
void clientClosePipes(Pair *pipes);

/* =================================
 		Envois et réceptions
   =================================  */ 

void clientWriteData(Pair *pipes, const void *buf, size_t size);
void orchestraWriteData(Pair *pipes, const void *buf, size_t size);
void clientReadData(Pair *pipes, void *buf, size_t size);
void orchestraReadData(Pair *pipes, void *buf, size_t size);

/* =================================
 		Attente d'une réponse
   =================================  */ 

int orchestraWaitResponse();
int clientWaitResponse();

/* =================================
		Accesseur et mutateur
   =================================  */ 

Pair getPipes();
//pthread_mutex_t getMutex(int numService);




#endif
