#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

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
//struct contenant le tab de float ,le pointeur mutex et le resultat
typedef struct {
  float *tab;
  int taille;
  float *res;
  pthread_mutex_t *mutex;

  int bInf; // borne inf à evaluer du thread
  int bSup; // borne sup à evaluer du thread
} ThreadData;


void * codeThread(void * arg)
{
  ThreadData *data = (ThreadData *) arg;
  float max = data->tab[data->bInf];

  for (int i = data->bInf + 1; i <= data->bSup; i++) {
    if (max < data->tab[i]) {
      max = data->tab[i];
    }
  }

  pthread_mutex_lock(data->mutex);

  if ( *(data->res) < max ) {
    *(data->res) = max;
  }

  pthread_mutex_unlock(data->mutex);

  return NULL;
}

// fonction de réception des données
void max_service_receiveDataData(/* tubes,*/ /* autre chose ? */)
{
}

// fonction de traitement des données
void max_service_computeResult(/* autre chose ? */)
{
}

// fonction d'envoi du résultat
void max_service_sendResult(/* tubes,*/ /* autre chose ? */)
{
}


/*----------------------------------------------*
 * fonction main
 *----------------------------------------------*/
int main(int argc, char * argv[])
{
    if (argc != 5)
        usage(argv[0], "nombre paramètres incorrect");

    // initialisations diverses
    int taille = 10;
    float tab[taille];
    for (int i = 0; i < taille; i++) {
      tab[i] = i;
    }

    int nbThreads = 3;
    //initialiser les pthreads et mutex
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_t threads[nbThreads];
    ThreadData datas[nbThreads];
    float res = 0;
    // pré-initialisation des données
    int distance = taille / nbThreads;
    printf("distance %d\n", distance);
    for (int i = 0; i < nbThreads; i++)
    {
      // il faut initialiser datas[i] avec :
      //   . un pointeur sur le tableau à evaluer
      //   . la taille du tableau
      //   . un pointeur sur la variable resultat
      //   . le mutex partagé
      datas[i].tab = tab;
      datas[i].taille = taille;
      datas[i].res = &res;
      datas[i].mutex = &mutex;

      // initialiser bornes de chaque thread
      datas[i].bInf = i * distance;
      datas[i].bSup = ((i+1) * distance )- 1;
    }

    // si le tableau est de taille impair, le dernier thread doit s'occuper jusqu'a la derniere case
    // mais il arrive jusqu'a la avant derniere, alors changer sa valeur pour la derniere.
    if (nbThreads % 2 == 1) {
      datas[nbThreads - 1].bSup = taille - 1;
    }

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

    printf("valeur max = %f\n", res);















    while (true)
    {
        // attente d'un code de l'orchestre (via tube anonyme)
        // si code de fin
        //    sortie de la boucle
        // sinon
        //    réception du mot de passe de l'orchestre
        //    attente du mot de passe du client
        //    si mot de passe incorrect
        //        envoi au client d'un code d'erreur
        //    sinon
        //        envoi au client d'un code d'acceptation
        //        réception des données du client (une fct par service)
        //        calcul du résultat (une fct par service)
        //        envoi du résultat au client (une fct par service)
        //        attente de l'accusé de réception du client
        //    modification du sémaphore pour prévenir l'orchestre de la fin
        break;
    }

    // libération éventuelle de ressources

    return EXIT_SUCCESS;
}
