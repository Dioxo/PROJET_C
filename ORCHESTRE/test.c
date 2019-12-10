#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#include "service_orchestre.h"


void unlock(int semid){
  struct sembuf buf;
  buf.sem_num = 0;
  buf.sem_op = 1;
  buf.sem_flg = SEM_UNDO;
  int res = semop (semid, &buf, 1);
  if (res == -1) {
    perror("");
  }else{
    printf("unlocked\n");
  }
  assert(res != -1);
}

int main(int argc, char const *argv[]) {
  printf("%d\n", getpid());

  printf("argv[3] = %s\n", argv[3] );
  key_t key = ftok(argv[3], 123456);
  
  int semid = semget(key, 1, 0641);
  printf("%d\n", semid);
  assert(semid == atoi(argv[1]));
  AnonymeTube anonymeTube;
  anonymeTube.fd[0] = atoi(argv[2]);

  int tmp;
  serviceRead(&anonymeTube, &tmp, sizeof(int) );

  serviceRead(&anonymeTube, &tmp, sizeof(int) );
  printf("mdp %d\n", tmp);
  sleep(20);

  unlock(semid);
  printf("dead %d\n" , getpid());
  return 0;
}
