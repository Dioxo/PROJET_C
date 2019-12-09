#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "client_service.h"

#ifndef SERVICE_MAX_CODES
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
 // STRUCTURE CONTENTANT LE Tableau A TRAITER
 typedef struct{
   float *tab;
   int taille;
 } Tableau;


//struct contenant le tab de float ,le pointeur sur mutex, le resultat et les bornes à evaluer
typedef struct {
  /*float *tab;
  int taille;*/
  Tableau *tableau;
  float *res;
  pthread_mutex_t *mutex;

  int bInf; // borne inf à evaluer du thread
  int bSup; // borne sup à evaluer du thread
} ThreadData;


void * codeThread(void * arg)
{
  ThreadData *data = (ThreadData *) arg;
  float max = data->tableau->tab[data->bInf];

  for (int i = data->bInf + 1; i <= data->bSup; i++) {
    if (max < data->tableau->tab[i]) {
      max = data->tableau->tab[i];
    }
  }

  pthread_mutex_lock(data->mutex);
  printf("MAX LOCAL %f\n", max);
  if ( *(data->res) < max ) {
    *(data->res) = max;
  }

  pthread_mutex_unlock(data->mutex);

  return NULL;
}

// fonction de réception des données
void max_service_receiveDataData(Pair *pipes, Tableau *tableau, int *nbThreads)
{
  int taille;
  serviceReadData(pipes, &taille, sizeof(int));

  //donner la taille du tableau à la structure
  tableau->taille = taille;
  tableau->tab = malloc(taille * sizeof(float));
  for (int i = 0; i < taille; i++) {
    //remplir le tableau
    serviceReadData(pipes, tableau->tab, sizeof(float));
    tableau->tab++;
  }
  tableau->tab -= taille;

  serviceReadData(pipes, nbThreads, sizeof(int));
}

// fonction de traitement des données
void max_service_computeResult(Tableau *tableau, int nbThreads, float *res)
{
  //definition de threads
  //initialiser les pthreads et mutex
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_t threads[nbThreads];
  ThreadData datas[nbThreads];
  *res = 0;
  // distance que chaque thread va traiter
  int distance = tableau->taille / nbThreads;

  for (int i = 0; i < nbThreads; i++)
  {
    // il faut initialiser datas[i] avec :
    //   . un pointeur sur le tableau à evaluer
    //   . la taille du tableau
    //   . un pointeur sur la variable resultat
    //   . le mutex partagé
    datas[i].tableau = tableau;
    datas[i].res = res;
    datas[i].mutex = &mutex;

    // initialiser bornes de chaque thread
    datas[i].bInf = i * distance;
    datas[i].bSup = ((i+1) * distance )- 1;
  }

  //changer la valeur de la derniere case pour la fin du tableau,
  // pour eviter la situation d'avoir un tableau de taille paire et le nb de threads impair...
  // car le dernier thread va arriver jusqu'a la avant derniere case
  datas[nbThreads - 1].bSup = tableau->taille - 1;

  // lancement des threads
   for (int i = 0; i < nbThreads; i++)
   {
       // et donc on passe un pointeur sur une struct différente chaque fois
       int ret = pthread_create(&(threads[i]), NULL, codeThread, &(datas[i]));
       assert(ret == 0);
   }

   // attente de la fin des threads
   for (int i = 0; i < nbThreads; i++)
   {
       int ret = pthread_join(threads[i], NULL);
       assert(ret == 0);
   }

   //detruction mutex
  pthread_mutex_destroy(&mutex);

}

// fonction d'envoi du résultat
void max_service_sendResult(Pair *pipes, float res)
{
  serviceWriteData(pipes, &res, sizeof(float));
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
    Tableau tableau;
    int nbThreads = 0;
    float res;

    Pair pipes;
    serviceOpenPipes(argv[3], argv[4], &pipes);

    int mdpClient;
    int mdpOrchestre;
    //variable qui va prendre le valeurs de divers codes erreurs, acceptation, etc
    int code = 0;
    while (true)
    {
      // attente d'un code de l'orchestre (via tube anonyme)
      //read(fd_orchestre, &code, sizeof(int));

      if(code == CODE_ERROR){
        break;
      }else{
        //    réception du mot de passe de l'orchestre
        //read(fd_orchestre, &mdpOrchestre, sizeof(int));

        // attente du mot de passe du client
        serviceReadData(&pipes, &mdpClient,sizeof(int));

        //si mot de passe incorrect
        if (mdpClient != 0) {
          // envoi au client d'un code d'erreur
          code = CODE_ERROR;
          serviceWriteData(&pipes, &code, sizeof(int) );
        }else{
          // envoi au client d'un code d'acceptation
          code = CODE_ACCEPT;
          serviceWriteData(&pipes, &code, sizeof(int) );

          // réception des données du client (une fct par service)
          max_service_receiveDataData(&pipes, &tableau, &nbThreads);
          // calcul du résultat (une fct par service)
          max_service_computeResult(&tableau, nbThreads, &res);
          // envoi du résultat au client (une fct par service)
          max_service_sendResult(&pipes, res);

          // attente de l'accusé de réception du client
          serviceReadData(&pipes, &code, sizeof(int));

          //liberer le tableau
          free(tableau.tab);
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
