#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

//FICHIER DE TEST======================================================
int main(int argc, char *argv[]) {
  if (argc != 4)
    return -1;

  int fd = open("c_s", O_WRONLY);
  int taille, mdp;
  char *chaine = malloc(sizeof(argv[3]) + 1);
  strcpy(chaine , argv[3]);
  mdp = atoi(argv[1]);
  taille = atoi(argv[2]);
  write(fd, &mdp, sizeof(int));
  write(fd, &taille, sizeof(int));
  write(fd, argv[3], strlen(argv[3]) + 1);
  return 0;
}
