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

void sendData(Pair *pipes, int numService)
{
    clientWriteData(pipes, &numService, sizeof(int));
}

void receiveResults(Pair *pipes)
{
    int n;
    m_readData(pipes, &n, sizeof(int));
}





int main(int argc, char * argv[])
{
    if (argc < 2)
        usage(argv[0], "nombre paramètres incorrect");

    int numService = strtol(argv[1], NULL, 10);

    // initialisations diverses
    getPipeFromOrchestra(); 
    // entrée en section critique pour communiquer avec l'orchestre
    pthread_mutex_lock(mutexcreerparlorchestre);
    // envoi à l'orchestre du numéro du service
    sendtoOrchestra(int firstPipe, int numService);
    // attente code de retour
    O_response  = waitResponseOrchestra();
    // si code d'erreur
    if (response == -1)
    {
        myassert(O_response == 0, O_response)
        //     sortie de la section critique
        pthread_mutex_unlock(response -> mutex);
        

    }
    // sinon
    else
    {
        //     récupération du mot de passe et des noms des 2 tubes
        password = response -> password;
        tubesName = response -> tube1 -> name;
        tubesName = response -> tube2 -> name;
        //     envoi d'une accusé de réception à l'orchestre
        sendtoOrchestra();
        //     sortie de la section critique
        pthread_mutex_unlock(response -> mutex);
        //     envoi du mot de passe au service
        sendtoService(password);
        //     attente de l'accusé de réception du service
        S_response  = waitResponseService();
        //     appel de la fonction d'envoi des données (une fct par service)
        client_somme_sendData(/* tubes,*/ int argc, char * argv[]){}
        client_max_sendData(/* tubes,*/ int argc, char * argv[]){}
        void client_compression_sendData(/* tubes,*/ int argc, char * argv[]){}
        //     appel de la fonction de réception du résultat (une fct par service)
        void client_somme_receiveResult(/* tubes,*/ int argc, char * argv[]);
        void client_max_receiveResult(/* tubes,*/ int argc, char * argv[]);
        void client_compression_receiveResult(/* tubes,*/ int argc, char * argv[]);
        //     envoi d'un accusé de réception au service
        sendtoService(acknowledgment);

    }
    // libération éventuelle de ressources
    
    return EXIT_SUCCESS;
}
