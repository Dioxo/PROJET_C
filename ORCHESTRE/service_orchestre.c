/* NOMS ET PRENOMS : HAYRAULT JULIEN - MOLINARES VALENCIA DIOGENES */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <assert.h>

#include "myassert.h"

#include "service_orchestre.h"


//===================================================
//     FONCTIONS À UTILISER AVEC LES SEMAPHORES
//===================================================

//---------------------------------------------------------------------------------
void serviceUnlock(int semid)
{
  struct sembuf buf;
  buf.sem_num = 0;
  buf.sem_op = 1;
  buf.sem_flg = 0;
  int res = semop (semid, &buf, 1);
  assert(res != -1);
}


//---------------------------------------------------------------------------------
void orchestreLock(int semid)
{
  struct sembuf buf;
  buf.sem_num = 0;
  buf.sem_op = -1;
  buf.sem_flg = 0;
  int res = semop(semid, &buf, 1);
  assert(res != -1);
}

//===================================================
//           ENVOI DES DONNÉES ORCHESTRE => CLIENT
//===================================================


//---------------------------------------------------------------------------------
void orchestreWrite(AnonymeTube *anonyme, const void *buf, size_t size)
{
  write(anonyme->fd[1], buf , size);
}


//---------------------------------------------------------------------------------
void serviceRead(AnonymeTube *anonyme, void *buf, size_t size)
{
  read(anonyme->fd[0], buf , size);
}
