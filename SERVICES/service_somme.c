#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef SERVICE_SOMME_CODES
#define CODE_ACCEPT 0
#define CODE_ERROR -1
#define CODE_FIN -2
#endif

#include "service_orchestre.h"
#include "client_service.h"


static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <clé_sémaphore> <fd_tube_anonyme> "
            "<tube_service_vers_client> <tube_client_vers_ service>\n",
            exeName);
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}


/*----------------------------------------------*
 * fonctions appelables par le service
 *----------------------------------------------*/

// fonction de réception des données
void somme_service_receiveDataData(Pair *pipes, float *num1, float *num2)
{
  serviceReadData(pipes,num1, sizeof(float));
  serviceReadData(pipes,num2, sizeof(float));
}

// fonction de traitement des données
void somme_service_computeResult(float *a, float *b, float *res)
{
  *res = *a + *b;
}

// fonction d'envoi du résultat
void somme_service_sendResult(Pair *pipes, float *res)
{
  printf("result %f\n", *res);
  serviceWriteData(pipes,res, sizeof(float));
}


/*----------------------------------------------*
 * fonction main
 *----------------------------------------------*/
/*
Un programme service prend en ligne de commande les paramètres suivants :
- la clé du sémaphore pour synchroniser le service avec l'orchestre
- le "file descriptor du tube anonyme" orchestre vers service
- le nom du tube nommé pour la communication service vers client == s_c
- le nom du tube nommé pour la communication client vers service == c_s
*/
int main(int argc, char * argv[])
{
    if (argc != 5)
        usage(argv[0], "nombre paramètres incorrect");

    // initialisations diverses
    AnonymeTube anonymeTube;
    anonymeTube.fd[0] = atoi(argv[2]);

    //obtenir le semaphore
    key_t key = ftok(argv[1], PROJET_ID);
    int semid = semget(key, 1 , 660);
    assert(semid != -1);

    float num1 = 0, num2 = 0, res = 0;
    int mdpClient;
    int mdpOrchestre;


    // ouverture de tube nommés
    // communication services => client
    // communication client => service
    Pair pipes;
    serviceOpenPipes(argv[3], argv[4], &pipes);

    //variable qui va prendre le valeurs de divers codes erreurs, acceptation, etc
    int code = 0;
    while (true)
    {
      // attente d'un code de l'orchestre (via tube anonyme)
      serviceRead(&anonymeTube, &code, sizeof(int));

      if(code == CODE_FIN){
        break;
      }else{
        //    réception du mot de passe de l'orchestre
        serviceRead(&anonymeTube, &mdpOrchestre, sizeof(int));

        // attente du mot de passe du client
        serviceReadData(&pipes, &mdpClient,sizeof(int));

        //si mot de passe incorrect
        if (mdpClient != mdpOrchestre) {
          // envoi au client d'un code d'erreur
          code = CODE_ERROR;
          serviceWriteData(&pipes, &code, sizeof(int) );
        }else{
          // envoi au client d'un code d'acceptation
          code = CODE_ACCEPT;
          serviceWriteData(&pipes, &code, sizeof(int) );

          // réception des données du client (une fct par service)
          somme_service_receiveDataData(&pipes, &num1, &num2);

          // calcul du résultat (une fct par service)
          somme_service_computeResult(&num1, &num2, &res);

          // envoi du résultat au client (une fct par service)
          somme_service_sendResult(&pipes, &res);

          // attente de l'accusé de réception du client
          serviceReadData(&pipes, &code, sizeof(int));
        }
        //    modification du sémaphore pour prévenir l'orchestre de la fin
        serviceUnlock(semid);
      }
    }

    // libération éventuelle de ressources
    close(anonymeTube.fd[0]);

    serviceClosePipes(&pipes);

    return EXIT_SUCCESS;
}
