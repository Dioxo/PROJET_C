#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#include "service_orchestre.h"




int main(int argc, char const *argv[]) {

  printf("%d\n", getpid());

  for (size_t i = 0; i < argc; i++) {
    printf("%s\n",argv[i]);
  }

  int fd = open("../SERVICES/S_C_0", O_RDWR);
  assert(fd != -1);

  printf("OPENED!!!!!!!!!!!!!!!!!\n");
  key_t key = ftok("../SERVICES/S_C_0", PROJET_ID);
  if (key == -1) {
    perror("");
    assert(key != -1);
  }
  int semid = semget(key, 1, 0660);
  printf("%d\n", semid);
  if (semid == -1) {
    perror("");
  }
  assert(semid != -1);
  AnonymeTube anonymeTube;
  anonymeTube.fd[0] = atoi(argv[2]);

  int tmp;
  serviceRead(&anonymeTube, &tmp, sizeof(int) );

  serviceRead(&anonymeTube, &tmp, sizeof(int) );
  printf("mdp %d\n", tmp);
  sleep(20);

  serviceUnlock(semid);
  printf("dead %d\n" , getpid());
  return 0;
}
