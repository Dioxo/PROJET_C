#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//FICHIER DE TEST======================================================
int main() {
  int fd = open("c_s", O_WRONLY);
  int taille = 10;
  float tab[taille];
  for (int i = 0; i < taille; i++) {
    tab[i] = i;
  }

  int mdp = 0;
  write(fd, &mdp, sizeof(int));

  write(fd, &taille, sizeof(int));

  for (int i = 0; i < taille; i++)
    write(fd, &tab[i], sizeof(float));

  int nbThreads = 3;
  write(fd, &nbThreads, sizeof(int));

  int codeError = -1;
  write(fd, &codeError, sizeof(int));

  return 0;
}
