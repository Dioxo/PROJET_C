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
//#include "client_service.h"

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

static void askConnection(Pair *pipes, Connection *c)
{
    co_clientWriteData(pipes, &(c->request), sizeof(int));
}

static int establishedConnection(Pair *pipes, Connection *c)
{
    co_clientReadData(pipes, &(c->request), sizeof(int));
    return c->request;
}

static void receive(Pair *pipes, Response *response)
{
    co_clientReadData(pipes, &(response->password), sizeof(int));
    co_clientReadData(pipes, &(response->lengthCtoO), sizeof(int));
    co_clientReadData(pipes, &(response->lengthOtoC), sizeof(int));
    response->CtoO = malloc((response->lengthCtoO) * sizeof(char));
    response->OtoC = malloc((response->lengthOtoC) * sizeof(char));
    co_clientReadData(pipes, response->CtoO, (response->lengthCtoO) * sizeof(char));
    co_clientReadData(pipes, response->OtoC, (response->lengthCtoO) * sizeof(char));
}

static void sendEOF(Pair *pipes)
{
    int resquest = REQUEST_EOF;
    co_clientWriteData(pipes, &resquest, sizeof(int));
}

int main(int argc, char * argv[])
{
    if (argc < 2)
        usage(argv[0], "nombre paramètres incorrect");

    int numService = strtol(argv[1], NULL, 10);
    printf("%d\n", numService);
    // initialisations diverses
    Pair pipes;
    Connection connection;
    Connection confirm;
    Response response;
    co_clientOpenPipes("pipeClientToOrchestra","pipeOrchestraToClient", &pipes);

    // entrée en section critique pour communiquer avec l'orchestre
    //pthread_mutex_lock(getMutex(numService));

    // envoi à l'orchestre du numéro du service
    connection.request = numService;
    askConnection(&pipes, &connection);
    // attente code de retour
    int resp = establishedConnection(&pipes, &confirm);
    printf("%d\n", resp); 
    // si code d'erreur
    if (resp == REQUEST_FAIL)
    {
        myassert(resp == REQUEST_FAIL, "Connection request has failed");
        //     sortie de la section critique
        //pthread_mutex_unlock(getMutex(numService));
    }
    // sinon
    else
    {
        //     récupération du mot de passe et des noms des 2 tubes
        printf("récupération du mot de passe et des noms des 2 tubes\n");
        receive(&pipes,&response); 

        //     envoi d'une accusé de réception à l'orchestre
    //    connection.request = REQUEST_EOF;
    //    sendEOF(&pipes, &connection);
        sendEOF(&pipes);
        //     sortie de la section critique
        //pthread_mutex_unlock(getMutex(numService));
        //     envoi du mot de passe au service
        
        //sendtoService(password);
        //     attente de l'accusé de réception du service
        //S_response  = waitResponseService();
        //     appel de la fonction d'envoi des données (une fct par service)
        //client_somme_sendData(/* tubes,*/ int argc, char * argv[]){}
        //client_max_sendData(/* tubes,*/ int argc, char * argv[]){}
        //void client_compression_sendData(/* tubes,*/ int argc, char * argv[]){}
        //     appel de la fonction de réception du résultat (une fct par service)
        //void client_somme_receiveResult(/* tubes,*/ int argc, char * argv[]);
        //void client_max_receiveResult(/* tubes,*/ int argc, char * argv[]);
        //void client_compression_receiveResult(/* tubes,*/ int argc, char * argv[]);
        //     envoi d'un accusé de réception au service
       //sendtoService(acknowledgment);
    //}
    // libération éventuelle de ressources
    }
    
    return EXIT_SUCCESS;
}
