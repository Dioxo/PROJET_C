#define  _XOPEN_SOURCE

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
#define  NB_SERVICES 1
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



void execFils(Service *service, const char *nomExecutable){
  if (fork() == 0) {
    //fermer le tube en ecriture pour le service
    close(service->anonymeTube.fd[1]);

    char fd[2];
    sprintf(fd,"%d", service->anonymeTube.fd[0]);

    int res = execl(nomExecutable, nomExecutable,
                                                service->s_c,  //clé du semaphore
                                                fd,            //descripteur tube
                                                service->s_c,  //nom tube nommée
                                                service->c_s,  //nom tube nommée
                                                NULL);

    assert(res != -1);
  }else{
    //fermer le tube en lecture pour l'orchestre
    close(service->anonymeTube.fd[0]);
  }
}


void createPipes(Service *service, int i){
  int nameLength;
  nameLength = snprintf(NULL, 0, "../SERVICES/X_X_%d", i) + 1;

  service->c_s = malloc(sizeof(char) * nameLength);
  sprintf(service->c_s, "../SERVICES/C_S_%d" , i);

  service->s_c = malloc(sizeof(char) * nameLength);
  sprintf(service->s_c, "../SERVICES/S_C_%d" , i);

  //creation tube c <=> s
  int res = mkfifo(service->s_c, 0644);
  assert(res != -1);

  res = mkfifo(service->c_s, 0644);
  assert(res != -1);
}

void destroyPipe(Service *service)
{
	int ret;
  ret = unlink(service->c_s);
	assert(ret == 0);
	free(service->c_s);
	service->c_s = NULL;

  ret = unlink(service->s_c);
  assert(ret == 0);
  free(service->s_c);
  service->s_c = NULL;
}


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
    for (int i = 0; i < NB_SERVICES; i++) {
      // - création de deux tubes nommés pour les communications entre
      //   les clients et le service
      createPipes(&(services[i]), i);

      // - création d'un tube anonyme pour converser (orchestre vers service)
      int res = pipe(services[i].anonymeTube.fd);
      assert(res != -1);

      // - un sémaphore pour que le service préviene l'orchestre de la
      //   fin d'un traitement
      services[i].key = ftok(services[i].s_c , PROJET_ID);
      assert(services[i].key != -1);

      // id de chaque semaphore
      services[i].semid = semget(services[i].key, 1, 0660 | IPC_CREAT);
      assert(services[i].semid != -1);

      //initialiser le semaphore pour faire d'exclusion mutuelle
      res = semctl(services[i].semid , 0, SETVAL, 1);
      assert(res != -1);
    }

    // lancement de chaque service
    //execFils(&services[0],  "../SERVICES/service_compression" );
    execFils(&services[0],  "../SERVICES/service_somme" );
    //execFils(&services[2],  "../SERVICES/service_max" );

    bool enUse[3];
    while (true)
    {
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
          break;
          //TODO
          // sinon si service non ouvert
          //     retour d'un code d'erreur

        }else if (enUse[tmp]) {
          //TODO
          // sinon si service déjà en cours de traitement
          //     retour d'un code d'erreur

        }else{
          // sinon
          //    Changer la valeur du semaphore
          orchestreLock(services[tmp].semid);
          //     génération d'un mot de passe
          // mdp est entre [0 et 10000]
          int mdp = (int)(rand() / (double)RAND_MAX * (10000 - 1));

          //     envoi d'un code de travail au service (via le tube anonyme)
          int code = CODE_ACCEPT;
          orchestreWrite(&services[tmp].anonymeTube, &code, sizeof(int));

          //     envoi du mot de passe au service (via le tube anonyme)
          orchestreWrite(&services[tmp].anonymeTube, &mdp, sizeof(int));

            // TODO
          //     envoi au client d'un code d'acceptation (via le tube nommé)
          //     envoi du mot de passe au client (via le tube nommé)
          //     envoi des noms des tubes nommés au client (via le tube nommé)
        }
        // attente d'un accusé de réception du client
    }

    // attente de la fin des traitements en cours (via les sémaphores)
    printf("waiting\n");
    while (true) {
      //c'est 1 si le service est libre
      /*if (semctl(services[0].semid, 0, GETVAL) == 1 &&
          semctl(services[1].semid, 0, GETVAL) == 1 &&
          semctl(services[2].semid, 0, GETVAL) == 1) {
        break;
      }*/
      if (semctl(services[0].semid, 0, GETVAL) == 1) {
        break;
      }
    }

    // envoi à chaque service d'un code de fin
    int code = CODE_FIN;
    for (int i = 0; i < NB_SERVICES; i++) {
      //si cette ligne posse de problemes avec valgrind, commenter alors
      orchestreWrite(&services[i].anonymeTube, &code, sizeof(int));

    }

    // attente de la terminaison des processus services
    for (int i = 0; i < NB_SERVICES; i++) {
      wait(NULL);
    }

    // destruction des tubes
    //destruction de semaphores
    for (int i = 0; i < NB_SERVICES; i++) {
      close(services[i].anonymeTube.fd[0]);
      int res = semctl(services[i].semid, 0, IPC_RMID);
      assert(res != -1);
      destroyPipe(&services[i]);
    }

    return EXIT_SUCCESS;
}
