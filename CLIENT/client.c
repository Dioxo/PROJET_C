#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "myassert.h"


	
#include "client_orchestre.h"
#include "client_service.h"

#include "client_somme.h"
#include "client_compression.h"
#include "client_max.h"


static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <num_service> ...\n", exeName);
    fprintf(stderr, "        ... : les paramètres propres au service\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

static void askConnection(co_Pair *pipes, co_Connection *c)
{
    co_clientWriteData(pipes, &(c->request), sizeof(int));
}

static int establishedConnection(co_Pair *pipes, co_Connection *c)
{
    co_clientReadData(pipes, &(c->request), sizeof(int));
    return c->request;
}

static void receive(co_Pair *pipes, co_Response *response)
{
    co_clientReadData(pipes, &(response->password), sizeof(int));
    co_clientReadData(pipes, &(response->lengthCtoS), sizeof(int));
    co_clientReadData(pipes, &(response->lengthStoC), sizeof(int));
    response->CtoS = malloc((response->lengthCtoS) * sizeof(char));
    response->StoC = malloc((response->lengthStoC) * sizeof(char));
    co_clientReadData(pipes, response->CtoS, (response->lengthCtoS) * sizeof(char));
    co_clientReadData(pipes, response->StoC, (response->lengthStoC) * sizeof(char));
}

static void sendEOF(co_Pair *pipes)
{
    int resquest = REQUEST_EOF;
    co_clientWriteData(pipes, &resquest, sizeof(int));
}


static void getNumbers(int argc ,char *argv[])
{
	const int LENGTH = 10;
    char n[LENGTH];
    printf("Entrez un nombre : \n");
    fgets(n,LENGTH-1,stdin);
    n[strlen(n)-1] = '\0';
    argv[argc] = (char *) malloc ((strlen(n)) * sizeof(char)); 
    strcpy(argv[argc],n);
}

static void getPath(int argc ,char *argv[])
{
	const int LENGTH_PATH= 100;
    char path[LENGTH_PATH];
    printf("Entrez le chemin du fichier : \n");
    fgets(path, LENGTH_PATH-1, stdin);
    path[strlen(path)-1] = '\0';
    argv[argc] = (char *) malloc ((strlen(path)) * sizeof(char)); 
    strcpy(argv[argc],path);

}

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

int main(int argc, char * argv[])
{
    if (argc < 2)
        usage(argv[0], "nombre paramètres incorrect");

    int numService = strtol(argv[1], NULL, 10);
    printf("%d\n", numService);
    // initialisations diverses
    co_Pair pipes;
    co_Connection connection;
    co_Connection confirm;
    co_Response response;
    co_clientOpenPipes("pipeClientToOrchestra","pipeOrchestraToClient", &pipes);
    Semaphore mutex = createSema(1);

    // entrée en section critique pour communiquer avec l'orchestre
    pSema(mutex);
    // envoi à l'orchestre du numéro du service
    connection.request = numService;
    askConnection(&pipes, &connection);
    // attente code de retour
    int resp = establishedConnection(&pipes, &confirm);
    printf("%d\n", resp); 
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
        //     envoi d'une accusé de réception à l'orchestre
        sendEOF(&pipes);
        //     sortie de la section critique
        vSema(mutex);
        //     envoi du mot de passe au service
        Pair s_pipes;
        clientOpenPipes(response.StoC, response.CtoS , &s_pipes);
     	clientWriteData(&s_pipes, &(response.password), sizeof(int));
        //     attente de l'accusé de réception du service
        int ack;
        clientReadData(&s_pipes, &ack, sizeof(int));
        //     appel de la fonction d'envoi des données (une fct par service)
        int argc;
        char **argv;
        switch(numService)
        {
        	case 1:
        	argc = 3;
        	argv = malloc(argc * sizeof(char *)); 
        	getNumbers(0,argv);
        	getNumbers(1,argv);
        	argv[3] = "La somme vaut :";
        	client_somme_sendData(&s_pipes, argc, argv);
        	client_somme_receiveResult(&s_pipes, argc, argv);
        	break;

        	case 2:
        	argc = 1;
        	argv = (char **) malloc(argc * sizeof(char*)); 
        	getPath(0,argv);
        	client_compression_sendData(&s_pipes, argc, argv);
        	client_compression_receiveResult(&s_pipes, argc, argv);
        	break;

        	case 3:
        	argc = 2;
        	argv = (char **) malloc(argc * sizeof(char*));
        	getThread(0,argv);
        	getPath(1,argv);
        	client_max_sendData(&s_pipes, argc, argv);
        	client_max_receiveResult(&s_pipes, argc, argv);
        	break;
        }
        free(*argv);	
        //     appel de la fonction de réception du résultat (une fct par service)
        


        //     envoi d'un accusé de réception au service
        int service_ack = SERVICE_EOF;
        clientWriteData(&s_pipes, &service_ack, sizeof(int));
    //}
    // libération éventuelle de ressources
    }
    
    return EXIT_SUCCESS;
}
