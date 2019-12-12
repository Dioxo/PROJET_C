/* NOMS ET PRENOMS : HAYRAULT JULIEN - MOLINARES VALENCIA DIOGENES */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "myassert.h"


#define CODE_ACCEPT 0
	
#include "client_orchestre.h"
#include "client_service.h"

#include "client_somme.h"
#include "client_compression.h"
#include "client_max.h"

/* Information : Le préfixe co_ est présent pour différentier les méthodes et structure provenant 
*  de client_orchestre.h et celle de client_service.h
*/



static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <num_service> ...\n", exeName);
    fprintf(stderr, "        ... : les paramètres propres au service\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}
//-------------------------------------------------------------
static void askConnection(co_Pair *pipes, co_Connection *c)
{
    co_clientWriteData(pipes, &(c->request), sizeof(int));
}
//-------------------------------------------------------------
static int establishedConnection(co_Pair *pipes, co_Connection *c)
{
    co_clientReadData(pipes, &(c->request), sizeof(int));
    return c->request;
}
//-------------------------------------------------------------
static void receive(co_Pair *pipes, co_Response *response)
{

    printf("AVANT\n");
    co_clientReadData(pipes, &(response->password), sizeof(int));
    printf("MDP\n");
    co_clientReadData(pipes, &(response->lengthCtoS), sizeof(int));
    printf("T1\n");
    co_clientReadData(pipes, &(response->lengthStoC), sizeof(int));
    printf("T2\n");
    response->CtoS = malloc((response->lengthCtoS) * sizeof(char));
    response->StoC = malloc((response->lengthStoC) * sizeof(char));
    co_clientReadData(pipes, response->CtoS, (response->lengthCtoS) * sizeof(char));
    co_clientReadData(pipes, response->StoC, (response->lengthStoC) * sizeof(char));
}
//-------------------------------------------------------------
static void sendEOF(co_Pair *pipes)
{
    int resquest = REQUEST_EOF;
    co_clientWriteData(pipes, &resquest, sizeof(int));
}
//-------------------------------------------------------------
static void getNumbers(int argc ,char *argv[])
{
	const int LENGTH = 10;
    char n[LENGTH];
    printf("Entrez un nombre : \n");
    fgets(n,LENGTH-1,stdin);
    n[strlen(n)] = '\0';
    argv[argc] = (char *) malloc ((strlen(n) +1)* sizeof(char)); 
    strcpy(argv[argc],n);
}
//-------------------------------------------------------------
static void getPath(int argc ,char *argv[])
{
	const int LENGTH_PATH= 100;
    char path[LENGTH_PATH];
    printf("Entrez le chemin du fichier : \n");
    fgets(path, LENGTH_PATH-1, stdin);
    path[strlen(path)-1] = '\0';
    argv[argc] = (char *) malloc ((strlen(path) + 1) * sizeof(char)); 
    strcpy(argv[argc],path);
}
//-------------------------------------------------------------
static void getThread(int argc ,char *argv[])
{
	const int LENGTH_THREAD=5;
    char thread[LENGTH_THREAD];
    printf("Entrez le nombre de thread : \n");
    fgets(thread, LENGTH_THREAD-1, stdin);
    thread[strlen(thread)-1] = '\0';
    argv[argc] = (char *) malloc ((strlen(thread)) * sizeof(char)); 
    strcpy(argv[argc],thread);
}
//-------------------------------------------------------------
void cleanResponse(co_Response *response)
{
    free(response->CtoS);
    response->CtoS = NULL;
    response->lengthCtoS = 0;
    free(response->StoC);
    response->StoC = NULL;
    response->lengthStoC = 0;
    response->password=0;
}


int main(int argc, char * argv[])
{
    if (argc < 2)
        usage(argv[0], "nombre paramètres incorrect");

    int numService = strtol(argv[1], NULL, 10);
    printf("%d\n", numService);
    // initialisations diverses
    co_Pair pipes;
    co_Connection connection;
    co_Connection confirmation;
    co_Response response;
    co_clientOpenPipes("../ORCHESTRE/pipeClientToOrchestra","../ORCHESTRE/pipeOrchestraToClient", &pipes);
    Semaphore mutex = createSema(1);
    // entrée en section critique pour communiquer avec l'orchestre
    pSema(mutex);
    // envoi à l'orchestre du numéro du service
    connection.request = numService;
    askConnection(&pipes, &connection);
    // attente code de retour
    int resp = establishedConnection(&pipes, &confirmation); 
    // si code d'erreur
    if (resp == REQUEST_FAIL)
    {
        //     sortie de la section critique
        vSema(mutex);
    }
    // sinon
    else
    {
        //     récupération du mot de passe et des noms des 2 tubes
        printf("récupération du mot de passe et des noms des 2 tubes\n");
        receive(&pipes,&response);
        printf("receive\n"); 
        //     envoi d'une accusé de réception à l'orchestre
        sendEOF(&pipes);
        printf("sendEOF\n");
        //     sortie de la section critique
        vSema(mutex);
        printf("mutex\n");
        //     envoi du mot de passe au service
        Pair s_pipes;
        clientOpenPipes(response.StoC, response.CtoS , &s_pipes);
     	printf("open\n");
        clientWriteData(&s_pipes, &(response.password), sizeof(int));
        printf("write\n");
        //     attente de l'accusé de réception du service  (On peut simuler avec un cat < nomduTube)
        int ack;
        clientReadData(&s_pipes, &ack, sizeof(int));
        //     appel de la fonction d'envoi des données (une fct par service)
        int nbArgs = 0;
        char **argument;

        switch(numService)
        {
        	case 0:
        	nbArgs = 3;
        	argument = malloc(nbArgs * sizeof(char *)); 
        	getNumbers(0,argument);
        	getNumbers(1,argument);
            argument[2] = malloc( (strlen("La somme vaut :") + 1) * sizeof(char));
    	    strcpy(argument[2], "La somme vaut :");
        	client_somme_sendData(&s_pipes, nbArgs, argument);
        	client_somme_receiveResult(&s_pipes, nbArgs, argument);
        	break;

        	case 1:
        	nbArgs = 2;
        	argument = (char **) malloc(nbArgs * sizeof(char*)); 
        	getPath(0,argument);
            argument[1] = "result";
        	client_compression_sendData(&s_pipes, nbArgs, argument);
        	client_compression_receiveResult(&s_pipes, nbArgs, argument);
        	break;

        	case 2:
        	nbArgs = 2;
        	argument = (char **) malloc(nbArgs * sizeof(char*));
        	getThread(0,argument);
        	getPath(1,argument);
        	client_max_sendData(&s_pipes, nbArgs, argument);
        	client_max_receiveResult(&s_pipes, nbArgs, argument);
        	break;
        }


        
        //     appel de la fonction de réception du résultat (une fct par service)
        //     envoi d'un accusé de réception au service
        int service_ack = SERVICE_EOF;
        clientWriteData(&s_pipes, &service_ack, sizeof(int));


    // libération éventuelle de ressources
        for (int i=0; i< nbArgs; i++)
        {
            free(argument[i]);
            argument[i]  = NULL;
        }
        free(argument); 
        clientClosePipes(&s_pipes);
        cleanResponse(&response);
      
    }

    //envoi à l'orchestre un code d'acceptation
    connection.request = CODE_ACCEPT;
    askConnection(&pipes, &connection);

    co_orchestraClosePipes(&pipes);

    destroySema(&mutex);    
    
    return EXIT_SUCCESS;
}
