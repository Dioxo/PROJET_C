/* NOMS ET PRENOMS : HAYRAULT JULIEN - MOLINARES VALENCIA DIOGENES */
#define  _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>


#include "config.h"
#include "client_orchestre.h"
#include "service_orchestre.h"

#ifndef ORCHESTRE_CODES
#define CODE_ACCEPT 0
#define CODE_FIN -2
#endif

//---------------------------------------------------------------------------------
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

/* ===================================================================================
                          Instruction envoi vers Service
   ===================================================================================   */ 
//-------------------------------------------------------
void execFils(Service *service, const char *nomExecutable)
{
  if (fork() == 0) 
  {
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
  }
  else
  {
    //fermer le tube en lecture pour l'orchestre
    close(service->anonymeTube.fd[0]);
  }
}

//---------------------------------------------------------------------------------
void createPipes(Service *service, int i)
{
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

//---------------------------------------------------------------------------------
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

/* ===================================================================================
                          Instruction envoi vers client
   ===================================================================================   */ 
//-------------------------------------------------------
static void sendTubePassword(co_Pair *pipes, co_Response *response)
{
    co_orchestraWriteData(pipes, &(response->password), sizeof(int));
    co_orchestraWriteData(pipes, &(response->lengthCtoS), sizeof(int));
    co_orchestraWriteData(pipes, &(response->lengthStoC), sizeof(int));
    co_orchestraWriteData(pipes, response->CtoS, (response->lengthStoC) * sizeof(char));
    co_orchestraWriteData(pipes, response->CtoS, (response->lengthCtoS) * sizeof(char));
}
//-------------------------------------------------------

/* ===================================================================================
                                    ORCHESTRE 
   ===================================================================================   */ 

int main(int argc, char * argv[])
{
    if (argc != 2)
        usage(argv[0], "nombre paramètres incorrect");

    // lecture du fichier de configuration
    config_init(argv[1]);


    // Pour la communication avec les clients
    // - création de 2 tubes nommés pour converser avec les clients
    co_Pair pipes;
    co_orchestraCreatePipes(&pipes);
    co_orchestraOpenPipes("pipeClientToOrchestra","pipeOrchestraToClient", &pipes);
    // - création d'un sémaphore pour que deux clients ne
    //   ne communiquent pas en même temps avec l'orchestre
    Semaphore mutex;
    mutex = createSema(1);

    co_Connection connection;
    co_Response data;


    // lancement des services, avec pour chaque service :
    int nbServices = config_getNbServices();
    Service services[nbServices];
    for (int i = 0; i < nbServices; i++)
    {
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
    for (int i = 0; i < nbServices; i++)
    {
      execFils(&services[i], config_getExeName(i));
    }
    //execFils(&services[0],  "../SERVICES/service_compression" );
    //execFils(&services[1],  "../SERVICES/service_somme" );
    //execFils(&services[2],  "../SERVICES/service_max" );

    bool enUse[3];
    while (true)
    {
        // attente d'une demande de service du client
      co_orchestraReadData(&pipes, &(connection.request), sizeof(int));
      int tmp = connection.request;
      printf("nbService = %d\n", tmp);

        // détecter la fin des traitements lancés précédemment via
        // les sémaphores dédiés (attention on n'attend pas la
        // fin des traitement, on note juste ceux qui sont finis)
      for (int i = 0; i < nbServices; i++) 
      {
        int res = semctl(services[i].semid, 0, GETVAL);
        //si 0 est en cours d'execution
        if (res == 0) 
        {
          enUse[i] = true;
        }
        else
        {
          enUse[i] = false;
        }
      }

        //analyse de la demande du client
        // si ordre de fin
      printf("REQUEST STOP %d\n", REQUEST_STOP);
      if (connection.request == REQUEST_STOP)
      {
        co_Connection response = {REQUEST_FAIL};
        //    retour d'un code d'acceptation
        co_orchestraWriteData(&pipes, &response, sizeof(int));
        //     sortie de la boucle
        break;
      }
      else if(!config_isServiceOpen(tmp))
      {
      // sinon si service non ouvert
        co_Connection response = {REQUEST_FAIL};
        //     retour d'un code d'erreur
        co_orchestraWriteData(&pipes, &response, sizeof(int));
      }

      else if (enUse[tmp]) 
      {
        // sinon si service déjà en cours de traitement
        co_Connection response = {REQUEST_FAIL};
        //     retour d'un code d'erreur
        co_orchestraWriteData(&pipes, &response, sizeof(int));
        break;
      }
      else
      {
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

        //     envoi au client d'un code d'acceptation (via le tube nommé)
        co_Connection response = {REQUEST_ACCEPT};
        co_orchestraWriteData(&pipes, &response, sizeof(int));

        //     envoi du mot de passe au client (via le tube nommé)
        data.password  = mdp;

        //     envoi des noms des tubes nommés au client (via le tube nommé)
        //data.CtoS = "Totototo";  // ?
        //data.StoC = "otototoT";  // ?
        data.CtoS = services[tmp].c_s;
        data.StoC = services[tmp].s_c;

        data.lengthCtoS = strlen(data.CtoS) + 1;
        data.lengthStoC = strlen(data.StoC) + 1;
        sendTubePassword(&pipes, &data);
      }
      // attente d'un accusé de réception du client
      int ack;
      co_orchestraReadData(&pipes, &(ack), sizeof(int)); 
    }

    // attente de la fin des traitements en cours (via les sémaphores)
    int cmpt = 0;
    printf("out bouche\n");
    while(cmpt < nbServices)
    {
      //si un service finit, regarder si le prochain a fini aussi

      if (semctl(services[cmpt].semid, 0, GETVAL) == 1) 
      {
        cmpt++;
      }
      else
      {
        //sleep pour ne pas chercher trop de fois si le service à fini
        sleep(3);
      }
    }

    // envoi à chaque service d'un code de fin
    int code = CODE_FIN;
    printf("finir services\n");
    for (int i = 0; i < nbServices; i++)
    {
      orchestreWrite(&services[i].anonymeTube, &code, sizeof(int));
    }

    // attente de la terminaison des processus services
    printf("wait\n");
    for (int i = 0; i < nbServices; i++)
    {
      wait(NULL);
    }

    // destruction des tubes
    printf("destroy\n");      
    co_orchestraDestroyPipes(&pipes);
    //destruction de semaphores
        printf("mutex\n");      
    destroySema(&mutex);

        printf("for\n");      

    for (int i = 0; i < nbServices; i++) 
    {
      close(services[i].anonymeTube.fd[0]);
      int res = semctl(services[i].semid, 0, IPC_RMID);
      assert(res != -1);
      destroyPipe(&services[i]);
    }

    //finir le API de config
    config_exit();
    return EXIT_SUCCESS;
}
