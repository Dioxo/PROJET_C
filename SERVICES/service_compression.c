#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

//#include "service_orchestre.h"
#include "client_service.h"

#ifndef SERVICE_COMPRESSION_CODES
#define CODE_ACCEPT 0
#define CODE_ERROR -1
#define CODE_FIN -2
#endif



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
char * compression_service_receiveDataData(Pair *pipes /* autre chose ? */)
{
  int taille = 0;
  serviceReadData(pipes, &taille, sizeof(int));
  assert(taille != 0);

  char *chaine = malloc(taille);
  serviceReadData(pipes, chaine, taille);

  return chaine;
}

// fonction de traitement des données
char * compression_service_computeResult(char* chaine)
{
  // taille de la chaine resultant = au pire de cas, 2 fois la taille de la chaine entrante
  int taille = (strlen(chaine) * 2) + 1;
  char *res = malloc(taille);
  //initialicer la chaine
  res[0] = '\0';
  int cmpt = 0;
  unsigned int i = 0;
  for ( ; i < strlen(chaine); i++) {
    cmpt ++;
    if (chaine[i] != chaine[i+1]) {
      //pour le moment, je vais sur dimmensioner le tableau
      char tmp[3];
      sprintf(tmp, "%d%c", cmpt, chaine[i]);
      if (strlen(res) == 0) {
        strcpy(res, tmp);
      }else{
        strcat(res, tmp);
      }
      cmpt = 0;
    }
  }
  //liberer chaine
  free(chaine);

  // finir la chaine par null-byte
  res[strlen(res)] = '\0';
  // reallouer la taille de res à la taille final
  res = (char *) realloc(res, strlen(res) +1);
  return res;
}

// fonction d'envoi du résultat
void compression_service_sendResult(Pair *pipes, const char *chaine)
{
  //envoi la taille de la chaine
  int taille = strlen(chaine) + 1;
  serviceWriteData(pipes, &taille, sizeof(int));

  //envoi la chaine mnt
  serviceWriteData(pipes, chaine, taille);
}


/*----------------------------------------------*
 * fonction main
 *----------------------------------------------*/
int main(int argc, char * argv[])
{
    if (argc != 5)
        usage(argv[0], "nombre paramètres incorrect");

    // initialisations diverses
    int fd_orchestre= atoi(argv[2]);
    char *chaine = "";

    // ouverture de tube nommés
    // communication services => client
    // communication client => service
    Pair pipes;
    serviceOpenPipes(argv[3], argv[4], &pipes);


    int mdpClient;
    int mdpOrchestre;

    //variable qui va prendre le valeurs de divers codes erreurs, acceptation, etc
    int code = 0;
    while (true)
    {
      // attente d'un code de l'orchestre (via tube anonyme)
      read(fd_orchestre, &code, sizeof(int));

      if(code == CODE_FIN){
        break;
      }else{
        //    réception du mot de passe de l'orchestre
        read(fd_orchestre, &mdpOrchestre, sizeof(int));

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
          chaine = compression_service_receiveDataData(&pipes);
          // calcul du résultat (une fct par service)
          chaine = compression_service_computeResult(chaine);
          // envoi du résultat au client (une fct par service)
          compression_service_sendResult(&pipes, chaine);

          // attente de l'accusé de réception du client
          serviceReadData(&pipes, &code, sizeof(int));

          //liberer la memoire de la chaine utilisée
          free(chaine);
        }
        //    modification du sémaphore pour prévenir l'orchestre de la fin
        //pas encore implementé

      }
    }

    // libération éventuelle de ressources
    close(fd_orchestre);
    serviceClosePipes(&pipes);
    return EXIT_SUCCESS;
}
