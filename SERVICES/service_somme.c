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

//#include "service_orchestre.h"
//#include "client_service.h"


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
void somme_service_receiveDataData(int r, float *num1, float *num2)
{
  read(r, num1, sizeof(float));
  read(r, num2, sizeof(float));
}

// fonction de traitement des données
void somme_service_computeResult(float *a, float *b, float *res)
{
  *res = *a + *b;
}

// fonction d'envoi du résultat
void somme_service_sendResult(int w, float *res)
{
  write(w, res, sizeof(float));
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
    int fd_orchestre= atoi(argv[2]);
    int fd_s_c,  fd_c_s;
    float num1 = 0, num2 = 0, res = 0;

    //ouverture tube nommé communication services => client
    //printf("Opening %s\n", argv[3]);
    fd_s_c = open(argv[3], O_WRONLY);   // cat < s_c
    assert(fd_s_c != -1);

    //ouverture tube nommé communication client => service
    //printf("Opening %s\n", argv[4]);
    fd_c_s = open(argv[4], O_RDONLY);   // cat > c_s
    assert(fd_c_s != -1);


    int mdpClient;
    int mdpOrchestre;

    //variable qui va prendre le valeurs de divers codes erreurs, acceptation, etc
    int code = 0;
    while (true)
    {
      // attente d'un code de l'orchestre (via tube anonyme)
      read(fd_orchestre, &code, sizeof(int));

      if(code == CODE_ERROR){
        break;
      }else{
        //    réception du mot de passe de l'orchestre
        read(fd_orchestre, &mdpOrchestre, sizeof(int));

        // attente du mot de passe du client
        read(fd_c_s, &mdpClient, sizeof(int));

        //si mot de passe incorrect
        if (mdpClient != 0) {
          // envoi au client d'un code d'erreur
          code = CODE_ERROR;
          write(fd_s_c, &code, sizeof(int) );
        }else{
          // envoi au client d'un code d'acceptation
          code = CODE_ACCEPT;
          write(fd_s_c, &code, sizeof(int));

          // réception des données du client (une fct par service)
          somme_service_receiveDataData(fd_c_s, &num1, &num2);

          // calcul du résultat (une fct par service)
          somme_service_computeResult(&num1, &num2, &res);

          // envoi du résultat au client (une fct par service)
          somme_service_sendResult(fd_s_c, &res);

          // attente de l'accusé de réception du client
          read(fd_c_s, &code, sizeof(int));
        }
        //    modification du sémaphore pour prévenir l'orchestre de la fin
        //pas encore implementé
      }
    }

    // libération éventuelle de ressources
    close(fd_orchestre);
    close(fd_c_s);
    close(fd_s_c);

    return EXIT_SUCCESS;
}
