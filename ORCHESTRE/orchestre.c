#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>


//#include "config.h"
//#include "client_orchestre.h"
#include "service_orchestre.h"

#ifndef ORCHESTRE_CODES
#define  NB_SERVICES 3
#define CODE_ACCEPT 0
#define CODE_FIN -2
#endif

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <fichier config>\n", exeName);
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

typedef struct {
  AnonymeTube anonymeTube;
  char *c_s;
  char *s_c;
  key_t key;
  int semid;
} Service;

void lock(int semid){
  struct sembuf buf;
  buf.sem_num = semid;
  buf.sem_op = -1;
  buf.sem_flg = SEM_UNDO;
  semop(semid, &buf, 1);
}

void createPipes(Service *service, int i);


void destroyPipe(Service *service);

void execFils(Service *service, const char *nomExecutable);

//-------------------------------------------------------
int main(int argc, char * argv[])
{
    if (argc != 2)
        usage(argv[0], "nombre paramètres incorrect");

    // lecture du fichier de configuration

    // Pour la communication avec les clients
    // - création de 2 tubes nommés pour converser avec les clients
    /*
    Pair pipes;
    orchestraCreatePipes(argv[1], Pair *pipes);
    */
    // - création d'un sémaphore pour que deux clients ne
    //   ne communiquent pas en même temps avec l'orchestre


    // lancement des services, avec pour chaque service :
    Service services[NB_SERVICES];
    char tmpName[18];
    for (int i = 0; i < NB_SERVICES; i++) {
      // - création de deux tubes nommés pour les communications entre
      //   les clients et le service
      createPipes(&(services[i]), i);

      //TODO rappeler to close fd[1] dans le fils
      // - création d'un tube anonyme pour converser (orchestre vers service)
      int res = pipe(services[i].anonymeTube.fd);
      assert(res != -1);

      // - un sémaphore pour que le service préviene l'orchestre de la
      //   fin d'un traitement
      //clé de chaque semaphore basé sur le nom "S_C_X"
      sprintf(tmpName, "../SERVICES/S_C_%d", i);
      services[i].key = ftok(tmpName, 123456);
      assert(services[i].key != -1);

      // id de chaque semaphore
      services[i].semid = semget(services[i].key, 1, 0641 | IPC_CREAT);
      assert(services[i].semid != -1);

      //initialiser le semaphore pour faire d'exclusion mutuelle
      res = semctl(services[i].semid , 0, SETVAL, 1);
      assert(res != -1);
    }

    // lancement de chaque service
    execFils(&services[0],  "./service_compression" );
    execFils(&services[1],  "./service_somme" );
    execFils(&services[2],  "./service_max" );

    bool enUse[3];
    while (true)
    {
      break;
        // attente d'une demande de service du client
        int tmp = 0;

        // détecter la fin des traitements lancés précédemment via
        // les sémaphores dédiés (attention on n'attend pas la
        // fin des traitement, on note juste ceux qui sont finis)
        for (int i = 0; i < NB_SERVICES; i++) {
          int res = semctl(services[i].semid, 0, GETVAL);
          //si 0 est en cours d'execution
          if (res == 0) {
            enUse[i] = true;
          }else{
            enUse[i] = false;
          }
        }

        //TODO analyse de la demande du client

        // si ordre de fin
        if (tmp == CODE_FIN) {
          //TODO

          //    retour d'un code d'acceptation

          //     sortie de la boucle
          break;
        }else if(enUse[tmp]){
          //TODO
          // sinon si service non ouvert
          //     retour d'un code d'erreur

        }else if (enUse[tmp]) {
          //TODO
          // sinon si service déjà en cours de traitement
          //     retour d'un code d'erreur

        }else{
          // sinon
          //     génération d'un mot de passe
          // mdp est entre [0 et 10000]
          int mdp = (int)(rand() / (double)RAND_MAX * (10000 - 1));

          //     envoi d'un code de travail au service (via le tube anonyme)
          int code = CODE_ACCEPT;
          orchestreWrite(&services[tmp].anonymeTube, &code, sizeof(int));

          //     envoi du mot de passe au service (via le tube anonyme)
          orchestreWrite(&services[tmp].anonymeTube, &mdp, sizeof(int));

          //    Changer la valeur du semaphore
          lock(services[tmp].semid);
            // TODO
          //     envoi au client d'un code d'acceptation (via le tube nommé)
          //     envoi du mot de passe au client (via le tube nommé)
          //     envoi des noms des tubes nommés au client (via le tube nommé)
        }
        // attente d'un accusé de réception du client
    }

    // attente de la fin des traitements en cours (via les sémaphores)
    while (true) {
      //c'est 1 si le service est libre
      if (semctl(services[0].semid, 0, GETVAL) == 1 &&
          semctl(services[1].semid, 0, GETVAL) == 1 &&
          semctl(services[2].semid, 0, GETVAL) == 1) {
        break;
      }
    }

    // envoi à chaque service d'un code de fin
    int code = CODE_FIN;
    for (int i = 0; i < NB_SERVICES; i++) {
      orchestreWrite(&services[i].anonymeTube, &code, sizeof(int));
    }

    // attente de la terminaison des processus services
    for (int i = 0; i < NB_SERVICES; i++) {
      wait(NULL);
    }

    // destruction des tubes
    //destruction de semaphores
    for (int i = 0; i < NB_SERVICES; i++) {
      int res = semctl(services[i].semid, 0, IPC_RMID);
      assert(res != -1);
      destroyPipe(&services[i]);
    }

    return EXIT_SUCCESS;
}


void execFils(Service *service, const char *nomExecutable){
  if (fork() == 0) {
    //fermer le tube en ecriture
    close(service->anonymeTube.fd[1]);

    char semid[12];
    sprintf(semid,"%d", service->semid);

    char fd[2];
    sprintf(fd,"%d", service->anonymeTube.fd[0]);

    execl(nomExecutable, nomExecutable,
                                  semid,
                                  fd,
                                  service->s_c,
                                  service->c_s,
                                  NULL);
  }
}


void createPipes(Service *service, int i){
  int nameLength;
  nameLength = snprintf(NULL, 0, "S_C_%d", i) + 1;

  service->c_s = malloc(sizeof(int) * nameLength);
  sprintf(service->c_s, "C_S_%d" , i);

  service->s_c = malloc(sizeof(int) * nameLength);
  sprintf(service->s_c, "S_C_%d" , i);

  //variable suplementaire pour créer les tubes dans le dossier SERVICES
  char *chaine = malloc(20 * sizeof(char));
  sprintf(chaine, "../SERVICES/S_C_%d", i);
  int res = mkfifo(chaine, 0644);
  assert(res != -1);

  sprintf(chaine, "../SERVICES/C_S_%d", i);
  res = mkfifo(chaine, 0644);
  assert(res != -1);

  free(chaine);
}

void destroyPipe(Service *service)
{
  //variable suplementaire pour aller vers le dossier SERVICES
  char * chaine = malloc(20 * sizeof(char));
  sprintf(chaine, "../SERVICES/%s", service->c_s);
	int ret;
  ret = unlink(chaine);
	assert(ret == 0);
	free(service->c_s);
	service->c_s = NULL;

  sprintf(chaine, "../SERVICES/%s", service->s_c);
  ret = unlink(chaine);
  assert(ret == 0);
  free(service->s_c);
  service->s_c = NULL;

  free(chaine);
}
